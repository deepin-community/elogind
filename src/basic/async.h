/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#include <sys/types.h>

#include "macro.h"

int asynchronous_job(void* (*func)(void *p), void *arg);

#if 0 /// UNNEEDED by elogind
int asynchronous_sync(pid_t *ret_pid);
#endif // 0
int asynchronous_close(int fd);

DEFINE_TRIVIAL_CLEANUP_FUNC(int, asynchronous_close);
