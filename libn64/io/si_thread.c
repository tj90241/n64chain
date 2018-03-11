//
// libn64/io/si_thread.c: Parallel interface (SI) I/O engine.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <io.h>
#include <io/si_thread.h>
#include <libn64.h>
#include <mq.h>
#include <stddef.h>
#include <stdint.h>
#include <syscall.h>

libn64func
static void libn64_si_issue(struct libn64_mq *si_backlog,
    uint32_t command, const struct libn64_si_request *si_req);

// Issues a command to the SI subsystem.
void libn64_si_issue(struct libn64_mq *si_backlog,
    uint32_t command, const struct libn64_si_request *si_req) {
  void *opaque = NULL;

  do {
    switch (command) {
      default:
        libn64_send_message1(si_req->mq, LIBN64_IO_RESP_ERROR, opaque);
        command = libn64_recv_message1(si_backlog, LIBN64_NOBLOCK, &opaque);
        si_req = (struct libn64_si_request *) opaque;
        break;
    }
  } while (si_req != NULL);
}

// Thread responsible for processing I/O requests.
void libn64_si_thread(void *opaque) {
  struct libn64_mq *si_backlog = libn64_mq_create();
  struct libn64_si_request *si_req = NULL;

  // This thread uses SI interrupts to watch for command completion.
  libn64_thread_reg_intr(libn64_thread_self(), LIBN64_INTERRUPT_SI);

  while (1) {
    uint32_t command = libn64_recvt_message1(&opaque);

    // RCP signaled an interrupt for the completion of the request.
    // Notify the requestor of the request commpletion.
    if ((int32_t) command == LIBN64_SI_RCP_INTERRUPT) {
      if (si_req != NULL) {
        libn64_send_message1(si_req->mq, LIBN64_IO_RESP_OK, si_req);
        command = libn64_recv_message1(si_backlog, LIBN64_NOBLOCK, &opaque);

        if ((si_req = (struct libn64_si_request *) opaque) != NULL)
          libn64_si_issue(si_backlog, command, si_req);
      }
    }

    // A new command arrived; queue it up or issue it.
    else {
      if (si_req != NULL) {
        libn64_send_message1(si_backlog, command, opaque);
        continue;
      }

      else {
        si_req = (struct libn64_si_request *) opaque;
        libn64_si_issue(si_backlog, command, si_req);
      }
    }
  }
}

