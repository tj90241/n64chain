//
// tools/mkfs.c: libn64 filesystem creation tool.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014 Tyler J. Stachecki <tstache1@binghamton.edu>
//
// This file is more or less a direct rip of chksum64:
// Copyright 1997 Andreas Sterbenz <stan@sbox.tu-graz.ac.at>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
typedef HANDLE dir_handle_t;
typedef WIN32_FIND_DATA file_handle_t;
#else
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
typedef DIR *dir_handle_t;
typedef struct dirent *file_handle_t;
#endif

struct fsinode {
  struct fsinode *next;
  char *local_path;
  char *name;
  size_t size;
};

static struct fsinode *create_fsinode(const char *path, file_handle_t *f);
static void delete_fsinode(struct fsinode *inode);
static int dir_close(dir_handle_t *handle);
static const char *dir_getname(file_handle_t *handle);
static int dir_next(const char *path, dir_handle_t *handle, file_handle_t *f);
static int dir_open(const char *path, dir_handle_t *handle);
static int walk_filesystem(const char *path, struct fsinode **root);
static int write_image(FILE *fs, FILE *fslist, const struct fsinode *root);

struct fsinode *create_fsinode(const char *path, file_handle_t *f) {
  struct fsinode *inode;
  const char *name = dir_getname(f);
  char *local_path, *name_copy;

  if ((inode = (struct fsinode *) malloc(sizeof(*inode))) == NULL) {
    fprintf(stderr, "Failed to allocate memory for an inode.\n");
    return NULL;
  }

  inode->next = NULL;

  if ((local_path = (char *) malloc(strlen(path) + strlen(name) + 2)) == NULL) {
    fprintf(stderr, "Failed to allocate memory for a path.\n");

    free(inode);
    return NULL;
  }

  if ((name_copy = (char *) malloc(strlen(name) + 1)) == NULL) {
    fprintf(stderr, "Failed to allocate memory for a name.\n");

    free(local_path);
    free(inode);
    return NULL;
  }

#ifdef _WIN32
  sprintf(local_path, "%s\\%s", path, name);

  if (!(f->dwFileAttributes & (FILE_ATTRIBUTE_NORMAL |
                               FILE_ATTRIBUTE_ARCHIVE))) {
    fprintf(stderr, "Only regular files are supported.\n");
    fprintf(stderr, "Unable to handle the following: '%s'\n", local_path);

    return NULL;
  }

  inode->size = (((size_t) f->nFileSizeHigh) << 32) | f->nFileSizeLow;
#else
  struct stat buf;
  int status;

  sprintf(local_path, "%s/%s", path, name);
  status = stat(local_path, &buf);

  if (status != 0 || !S_ISREG(buf.st_mode)) {
    if (status != 0) {
      fprintf(stderr, "Failed to read file attributes: '%s'\n", local_path);
    }

    else {
      fprintf(stderr, "Only regular files are supported.\n");
      fprintf(stderr, "Unable to handle the following: '%s'\n", local_path);
    }

    free(name_copy);
    free(local_path);
    free(inode);

    return NULL;
  }

  inode->size = buf.st_size;
#endif

  strcpy(name_copy, name);

  inode->local_path = local_path;
  inode->name = name_copy;
  return inode;
}

void delete_fsinode(struct fsinode *inode) {
  struct fsinode *next;

  while (inode != NULL) {
    next = inode->next;

    free(inode->local_path);
    free(inode->name);
    free(inode);

    inode = next;
  }
}

static int dir_close(dir_handle_t *handle) {
#ifdef _WIN32
  if (*handle != INVALID_HANDLE_VALUE)
    FindClose(*handle);
#else
  if (*handle != NULL)
    closedir(*handle);
#endif

  return 0;
}

const char *dir_getname(file_handle_t *handle) {
#ifdef _WIN32
  return handle->cFileName;
#else
  return (*handle)->d_name;
#endif
}

int dir_next(const char *path, dir_handle_t *handle, file_handle_t *f) {
#ifdef _WIN32
  if (*handle == INVALID_HANDLE_VALUE) {
    char searchpath[MAX_PATH];

    if (strlen(path) > (MAX_PATH - 3))
      return -1;

    sprintf(searchpath, "%s\\*", path);

    *handle = FindFirstFile(searchpath, f);
    if (*handle == INVALID_HANDLE_VALUE)
      return -1;
  }

  else {
    if (FindNextFile(*handle, f) == 0)
      return GetLastError() == ERROR_NO_MORE_FILES ? 1 : -1;
  }
#else
  path = path;
  errno = 0;
  if ((*f = readdir(*handle)) == NULL)
    return errno == 0 ? 1 : -1;
#endif

  return 0;
}

int dir_open(const char *path, dir_handle_t *handle) {
#ifdef _WIN32
  DWORD status;

  path = path;
  *handle = INVALID_HANDLE_VALUE;
  status = GetFileAttributes(path);

  if (status == INVALID_FILE_ATTRIBUTES)
    return 1;

  if (status != FILE_ATTRIBUTE_DIRECTORY)
    return -1;
#else
  errno = 0;
  if ((*handle = opendir(path)) == NULL)
    return errno == ENOENT ? 1 : -1;
#endif

  return 0;
}

int main(int argc, const char *argv[]) {
  struct fsinode *root;
  FILE *fs, *fslist;
  int status;

  if (argc != 4) {
    printf("Usage: %s <fs bin path> <listing output path> <root>\n", argv[0]);
    return EXIT_SUCCESS;
  }

  if ((fs = fopen(argv[1], "w+")) == NULL) {
    fprintf(stderr, "Failed to open %s for writing.\n", argv[1]);
    return EXIT_FAILURE;
  }

  if ((fslist = fopen(argv[2], "w+")) == NULL) {
    fprintf(stderr, "Failed to open %s for writing.\n", argv[2]);
  }

  status = walk_filesystem(argv[3], &root);

  if (status == EXIT_SUCCESS) {
    status = write_image(fs, fslist, root);
  }

  delete_fsinode(root);
  fclose(fslist);
  fclose(fs);
  return status;
}

int walk_filesystem(const char *path, struct fsinode **root) {
  dir_handle_t root_handle;
  file_handle_t file_handle;
  int walk_status;

  *root = NULL;
  walk_status = dir_open(path, &root_handle);

  if (walk_status < 0) {
    fprintf(stderr, "Unable to list the contents of '%s'.\n", path);
    return EXIT_FAILURE;
  }

  else if (walk_status > 0) {
    printf("WARNING: '%s' does not exist; creating empty filesystem.\n", path);
    return EXIT_SUCCESS;
  }

  if ((walk_status = dir_next(path, &root_handle, &file_handle)) >= 0) {
    do {
      const char *name = dir_getname(&file_handle);

      if (strcmp(name, ".") && strcmp(name, "..")) {
        struct fsinode *inode;

        if ((inode = create_fsinode(path, &file_handle)) == NULL) {
          delete_fsinode(*root);
          dir_close(&root_handle);
          *root = NULL;

          return EXIT_FAILURE;
        }

        if (root != NULL) {
          inode->next = *root;
          *root = inode;
        }

        else {
          *root = inode;
        }
      }

      walk_status = dir_next(path, &root_handle, &file_handle);
    } while(walk_status == 0);
  }

  if (walk_status < 0) {
    fprintf(stderr, "An error occurred while listing files in '%s'\n", path);
  }

  dir_close(&root_handle);
  return walk_status > 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int write_image(FILE *fs, FILE *fslist, const struct fsinode *root) {
  const struct fsinode *inode;
  size_t file_offset;
  unsigned i;

  fprintf(fslist, "// This file is auto-generated by mkfs; do not alter\n\n");
  fprintf(fslist, "#ifndef FILESYSTEM_H\n#define FILESYSTEM_H\n\n");

  if (root == NULL) {
    fprintf(fs, "EMPTYFS!");
    fprintf(fslist, "#endif\n");
    return EXIT_SUCCESS;
  }

  fprintf(fslist, "#define CART_FILES");

  for (inode = root; inode != NULL; inode = inode->next) {

    // Convert the filenames into macro-y looking formats.
    for (i = 0; i < strlen(inode->name); i++) {
      if (isalpha(inode->name[i]))
        inode->name[i] = toupper(inode->name[i]);
      else if (!isdigit(inode->name[i]))
        inode->name[i] = '_';
    }

    fprintf(fslist, " \\\n  X(%s)", inode->name);
  }

  fprintf(fslist, "\n\n");

  for (inode = root, file_offset = 0; inode != NULL; inode = inode->next) {
    FILE *f;

    fprintf(fslist, "#define CART_OFFS_%s 0x%.8X\n", inode->name,
        (unsigned int) (file_offset + 0x1000));

    fprintf(fslist, "#define CART_SIZE_%s 0x%.8X\n", inode->name,
        (unsigned int) inode->size);

    if ((f = fopen(inode->local_path, "rb")) == NULL) {
      fprintf(stderr, "Failed to open for reading: '%s'\n", inode->local_path);
      return EXIT_FAILURE;
    }

    while (!feof(f)) {
      unsigned char byte;

      if (ferror(f)) {
        fprintf(stderr, "Failed while reading: '%s'\n", inode->local_path);
        fclose(f);

        return EXIT_FAILURE;
      }

      byte = fgetc(f);

      if (!feof(f) && fputc(byte, fs) == EOF) {
        fprintf(stderr, "Failed while writing: '%s'\n", inode->local_path);
        fclose(f);

        return EXIT_FAILURE;
      }
    }

    file_offset += inode->size;

    if (file_offset & 0x1) {
      fputc('\0', fs);
      file_offset++;
    }

    fclose(f);
  }

  if (!file_offset) {
    fprintf(fs, "EMPTYFS!");
  }

  fprintf(fslist, "\n#endif\n");
  return EXIT_SUCCESS;
}

