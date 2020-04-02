//
// tools/checksum.c: ROM checksum/padding tool.
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CHECKSUM_START    0x00001000
#define CHECKSUM_LENGTH   0x00100000
#define CIC_NUS6102_SEED  0xF8CA4DDC
#define CRC_OFFSET        0x00000010
#define HEADER_SIZE       0x00000040
#define BUFFER_SIZE       0x00008000

static inline uint32_t btol(const uint8_t *b) {
  return
    (b[0] << 24) |
    (b[1] << 16) |
    (b[2] <<  8) |
    (b[3] <<  0);
}

static inline void ltob(uint32_t l, uint8_t *b) {
  b[0] = l >> 24;
  b[1] = l >> 16;
  b[2] = l >>  8;
  b[3] = l >>  0;
}

static inline uint32_t rol(uint32_t i, unsigned b) {
  return (i << b) | (i >> (32 - b));
}

static int calculate_crc(const uint8_t *buffer, uint32_t crc[2]);

static long get_file_length_and_rewind(FILE *f);
static int safe_fread(uint8_t *buffer, size_t len, FILE *f);
static int safe_fwrite(const uint8_t *buffer, size_t len, FILE *f);

static int calculate_crc(const uint8_t *buffer, uint32_t crc[2]) {
  uint8_t *crc_buffer;
  unsigned l, offs;

  uint32_t t1, t2, t3, t4, t5, t6;
  uint32_t c1, k1, k2;

  if ((crc_buffer = malloc(BUFFER_SIZE)) == NULL) {
    perror("malloc");

    return 1;
  }

  t1 = t2 = t3 = CIC_NUS6102_SEED;
  t4 = t5 = t6 = CIC_NUS6102_SEED;

  l = CHECKSUM_LENGTH;
  offs = CHECKSUM_START;

  while (1) {
    unsigned i, n;

    n = (BUFFER_SIZE < l) ? BUFFER_SIZE : l;
    memcpy(crc_buffer, buffer + offs, n);
    offs += n;

    for (i = 0; i < n; i += 4) {
      c1 = btol(crc_buffer + i);
      k1 = t6 + c1;

      if (k1 < t6)
        t4++;

      t6 = k1;
      t3 ^= c1;
      k2 = c1 & 0x1F;
      k1 = rol(c1, k2);
      t5 += k1;

      if (c1 < t2)
        t2 ^= k1;
      else
        t2 ^= t6 ^ c1;

      t1 += c1 ^ t5;
    }

    l -= n;

    if (!l)
      break;
  }

  crc[0] = t6 ^ t4 ^ t3;
  crc[1] = t5 ^ t2 ^ t1;

  free(crc_buffer);
  return 0;
}

int main(int argc, const char *argv[]) {
  size_t min_sz = 2 * 1024 * 1024;

  uint8_t *buffer, out[8];
  uint32_t crc[2], c_crc[2];

  long fsz;
  int status;
  FILE *f;

  // Process arguments.
  if (argc != 3) {
    printf("Usage: %s <header> <filename>\n", argv[0]);
    return EXIT_SUCCESS;
  }

  // Open header, read it into memory, close.
  if ((buffer = calloc(min_sz, 1)) == NULL) {
    perror("calloc");

    return EXIT_FAILURE;
  }

  if ((f = fopen(argv[1], "rb")) == NULL) {
    perror("fopen");
    free(buffer);

    return EXIT_FAILURE;
  }

  if (safe_fread(buffer, CHECKSUM_START, f)) {
    fprintf(stderr, "Unable to read contents of: %s.\n", argv[1]);
    free(buffer);
    fclose(f);

    return EXIT_FAILURE;
  }

  fclose(f);

  // Open ROM contents, read it into memory, compute CRC, close.
  if ((f = fopen(argv[2], "rb")) == NULL) {
    perror("fopen");
    free(buffer);

    return EXIT_FAILURE;
  }

  if ((fsz = get_file_length_and_rewind(f)) < 0) {
    fprintf(stderr, "Unable to determine file size of: %s.\n", argv[2]);
    free(buffer);
    fclose(f);

    return EXIT_FAILURE;
  }

  if (safe_fread(buffer + CHECKSUM_START,
    ((CHECKSUM_LENGTH - CHECKSUM_START) > fsz)
      ? fsz : CHECKSUM_LENGTH - CHECKSUM_START, f)) {
    fprintf(stderr, "Unable to read contents of: %s.\n", argv[2]);
    free(buffer);
    fclose(f);

    return EXIT_FAILURE;
  }

  // Read current CRC, compute correct one.
  c_crc[0] = btol(buffer + CRC_OFFSET);
  c_crc[1] = btol(buffer + CRC_OFFSET + 4);
  status = calculate_crc(buffer, crc);

  fclose(f);

  if (status) {
    free(buffer);

    return EXIT_FAILURE;
  }

  if ((c_crc[0] == crc[0] && c_crc[1] == crc[1]) && ((size_t) fsz >= min_sz))
    return EXIT_SUCCESS;

  // CRCs don't match; update the ROM.
  ltob(crc[0], out);
  ltob(crc[1], out + 4);

  if ((f = fopen(argv[2], "rb+")) == NULL) {
    perror("fopen");
    free(buffer);

    return EXIT_FAILURE;
  }

  // Ensure the file is at least min_sz in length, write the CRC.
  size_t wr_len = min_sz > CHECKSUM_LENGTH ? CHECKSUM_LENGTH : min_sz;

  if (safe_fwrite(buffer, wr_len, f)) {
     fprintf(stderr, "Unable to write contents to: %s.\n", argv[2]);
     free(buffer);
     fclose(f);

    return EXIT_FAILURE;
  }

  free(buffer);

  if (fseek(f, CRC_OFFSET, SEEK_SET)) {
    perror("fseek");
    fclose(f);

    return EXIT_FAILURE;
  }

  if (safe_fwrite(out, sizeof(out), f)) {
    fprintf(stderr, "Unable to write CRC to: %s.\n", argv[2]);
    fclose(f);

    return EXIT_FAILURE;
  }

  fclose(f);
  return 0;
}

long get_file_length_and_rewind(FILE *f) {
  long sz;

  if (fseek(f, 0, SEEK_END))
    return -1;

  if ((sz = ftell(f)) < 0)
    return -1;

  rewind(f);
  return sz;
}

int safe_fread(uint8_t *buffer, size_t len, FILE *f) {
  size_t i, amt;

  for (i = 0; i < len; i += amt) {
    amt = fread(buffer + i, 1, len - i, f);

    if (ferror(f) || feof(f))
      return -1;
  }

  return 0;
}

int safe_fwrite(const uint8_t *buffer, size_t len, FILE *f) {
  size_t i, amt;

  for (i = 0; i < len; i += amt) {
    amt = fwrite(buffer + i, 1, len - i, f);

    if (ferror(f))
      return -1;
  }

  return 0;
}

