//
// libn64/io/thread.c: Parallel and serial I/O engine.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <io.h>
#include <io/thread.h>
#include <libn64.h>
#include <mq.h>
#include <stdint.h>
#include <syscall.h>

libn64func
static void libn64_io_issue(const struct libn64_io_request *req);

// Issues a request to the PI subsystem.
void libn64_io_issue(const struct libn64_io_request *req) {
  uint32_t pi_addr;

  extern char _binary_filesystem_bin_start;
  char *fs_ptr = &_binary_filesystem_bin_start;
  uint32_t fs_offs = (uint32_t) fs_ptr - 0x70000400;

  __asm__ __volatile__(
    ".set noreorder\n\t"

    "lui %0, 0xA460\n\t"
    "sw %1, 0x0(%0)\n\t"
    "sw %2, 0x4(%0)\n\t"
    "sw %3, 0xC(%0)\n\t"

    ".set reorder\n\t"

    : "=&r" (pi_addr)
    : "r" (req->dest_address),
      "r" (req->src_address + fs_offs),
      "r" (req->size)
  );
}

// Thread responsible for processing I/O requests.
void libn64_io_thread(void *opaque) {
  libn64_thread_reg_intr(libn64_thread_self(), LIBN64_INTERRUPT_PI);

  struct libn64_mq *backlog = libn64_mq_create();
  struct libn64_io_request *req = NULL;
  int pending = 0;

  while (1) {
    uint32_t command = libn64_recvt_message1(&opaque);

    switch (command) {
      case LIBN64_IO_RCP_INTERRUPT:

        // RCP signaled an interrupt for the completion of the request.
        // Notify the requestor of the commpletion. If the backlog is
        // not empty, issue the next request to the PI subsystem.
        if (req != NULL) {
          libn64_send_message1(req->mq, LIBN64_IO_RESP_OK, req);

          if (pending) {
            pending--;

            libn64_recv_message1(backlog, LIBN64_BLOCK, &opaque);
            req = (struct libn64_io_request *) opaque;
            libn64_io_issue(req);
          } else {
            req = NULL;
          }
        }

        break;

      case LIBN64_IO_CMD_CART2RAM:

        // If a request is already in progress, add it to the backlog.
        // Otherwise, begin processing the request immediately.
        if (pending) {
          libn64_send_message1(backlog, command, req);
          pending++;
        } else {
          req = (struct libn64_io_request *) opaque;
          libn64_io_issue(req);
        }

        break;

      default:
        continue;
    }
  }
}

