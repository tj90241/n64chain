//
// libn64/io/pi_thread.c: Parallel interface (PI) I/O engine.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <io.h>
#include <io/filesystem.h>
#include <io/pi_thread.h>
#include <libn64.h>
#include <mq.h>
#include <stddef.h>
#include <stdint.h>
#include <syscall.h>

libn64func
static void libn64_pi_issue(struct libn64_mq *pi_backlog,
    uint32_t command, const struct libn64_pi_request *pi_req);

// Issues a command to the PI subsystem.
void libn64_pi_issue(struct libn64_mq *pi_backlog,
    uint32_t command, const struct libn64_pi_request *pi_req) {
  void *opaque = NULL;

  do {
    switch (command) {
      case LIBN64_PI_CMD_FILESYSTEM_LOAD:
        filesystem_load(pi_req);
        return;

      default:
        libn64_send_message1(pi_req->mq, LIBN64_IO_RESP_ERROR, opaque);
        command = libn64_recv_message1(pi_backlog, LIBN64_NOBLOCK, &opaque);
        pi_req = (struct libn64_pi_request *) opaque;
        break;
    }
  } while (pi_req != NULL);
}

// Thread responsible for processing I/O requests.
void libn64_pi_thread(void *opaque) {
  struct libn64_mq *pi_backlog = libn64_mq_create();
  struct libn64_pi_request *pi_req = NULL;

  // This thread uses PI interrupts to watch for command completion.
  libn64_thread_reg_intr(libn64_thread_self(), LIBN64_INTERRUPT_PI);

  while (1) {
    uint32_t command = libn64_recvt_message1(&opaque);

    // RCP signaled an interrupt for the completion of the request.
    // Notify the requestor of the request commpletion.
    if ((int32_t) command == LIBN64_PI_RCP_INTERRUPT) {
      if (pi_req != NULL) {
        libn64_send_message1(pi_req->mq, LIBN64_IO_RESP_OK, pi_req);
        command = libn64_recv_message1(pi_backlog, LIBN64_NOBLOCK, &opaque);

        if ((pi_req = (struct libn64_pi_request *) opaque) != NULL)
          libn64_pi_issue(pi_backlog, command, pi_req);
      }
    }

    // A new command arrived; queue it up or issue it.
    else {
      if (pi_req != NULL) {
        libn64_send_message1(pi_backlog, command, opaque);
        continue;
      }

      else {
        pi_req = (struct libn64_pi_request *) opaque;
        libn64_pi_issue(pi_backlog, command, pi_req);
      }
    }
  }
}

