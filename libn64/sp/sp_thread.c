//
// libn64/sp/sp_thread.c: Signal processor (SP) engine.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <libn64.h>
#include <mq.h>
#include <sp.h>
#include <sp/sp_thread.h>
#include <stddef.h>
#include <stdint.h>
#include <syscall.h>

libn64func
static void libn64_sp_issue(struct libn64_mq *sp_backlog,
    uint32_t command, const struct libn64_sp_request *sp_req);

// Issues a command to the SP subsystem.
void libn64_sp_issue(struct libn64_mq *sp_backlog,
    uint32_t command, const struct libn64_sp_request *sp_req) {
  void *opaque = NULL;

  do {
    switch (command) {
      default:
        libn64_send_message1(sp_req->mq, LIBN64_SP_RESP_ERROR, opaque);
        command = libn64_recv_message1(sp_backlog, LIBN64_NOBLOCK, &opaque);
        sp_req = (struct libn64_sp_request *) opaque;
        break;
    }
  } while (sp_req != NULL);
}

// Thread responsible for processing SP requests/events.
void libn64_sp_thread(void *opaque) {
  struct libn64_mq *sp_backlog = libn64_mq_create();
  struct libn64_sp_request *sp_req = NULL;

  // This thread uses SP interrupts to watch for command completion.
  libn64_thread_reg_intr(libn64_thread_self(), LIBN64_INTERRUPT_SP);

  while (1) {
    uint32_t command = libn64_recvt_message1(&opaque);

    // RCP signaled an interrupt for the completion of the request.
    // Notify the requestor of the request commpletion.
    if ((int32_t) command == LIBN64_SP_RCP_INTERRUPT) {
      if (sp_req != NULL) {
        libn64_send_message1(sp_req->mq, LIBN64_SP_RESP_OK, sp_req);
        command = libn64_recv_message1(sp_backlog, LIBN64_NOBLOCK, &opaque);

        if ((sp_req = (struct libn64_sp_request *) opaque) != NULL)
          libn64_sp_issue(sp_backlog, command, sp_req);
      }
    }

    // A new command arrived; queue it up or issue it.
    else {
      if (sp_req != NULL) {
        libn64_send_message1(sp_backlog, command, opaque);
        continue;
      }

      else {
        sp_req = (struct libn64_sp_request *) opaque;
        libn64_sp_issue(sp_backlog, command, sp_req);
      }
    }
  }
}

