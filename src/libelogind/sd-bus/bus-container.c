/* SPDX-License-Identifier: LGPL-2.1+ */

#include <fcntl.h>
#include <unistd.h>

#include "bus-container.h"
#include "bus-internal.h"
#include "bus-socket.h"
#include "fd-util.h"
#include "namespace-util.h"
#include "process-util.h"
#include "string-util.h"
#include "util.h"

int bus_container_connect_socket(sd_bus *b) {
        _cleanup_close_pair_ int pair[2] = { -1, -1 };
        _cleanup_close_ int pidnsfd = -1, mntnsfd = -1, usernsfd = -1, rootfd = -1;
        int r, error_buf = 0;
        pid_t child;
        ssize_t n;

        assert(b);
        assert(b->input_fd < 0);
        assert(b->output_fd < 0);
        assert(b->nspid > 0 || b->machine);

        if (b->nspid <= 0) {
                log_debug("sd-bus: connecting bus%s%s to machine %s...",
                          b->description ? " " : "", strempty(b->description), b->machine);

                r = container_get_leader(b->machine, &b->nspid);
                if (r < 0)
                        return r;
        } else
                log_debug("sd-bus: connecting bus%s%s to namespace of PID "PID_FMT"...",
                          b->description ? " " : "", strempty(b->description), b->nspid);

        r = namespace_open(b->nspid, &pidnsfd, &mntnsfd, NULL, &usernsfd, &rootfd);
        if (r < 0)
                return r;

        b->input_fd = socket(b->sockaddr.sa.sa_family, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
        if (b->input_fd < 0)
                return -errno;

        b->input_fd = fd_move_above_stdio(b->input_fd);

        b->output_fd = b->input_fd;

        bus_socket_setup(b);

        if (socketpair(AF_UNIX, SOCK_SEQPACKET|SOCK_CLOEXEC, 0, pair) < 0)
                return -errno;

        r = namespace_fork("(sd-buscntrns)", "(sd-buscntr)", NULL, 0, FORK_RESET_SIGNALS|FORK_DEATHSIG,
                           pidnsfd, mntnsfd, -1, usernsfd, rootfd, &child);
        if (r < 0)
                return r;
        if (r == 0) {
                pair[0] = safe_close(pair[0]);

                r = connect(b->input_fd, &b->sockaddr.sa, b->sockaddr_size);
                if (r < 0) {
                        /* Try to send error up */
                        error_buf = errno;
                        (void) write(pair[1], &error_buf, sizeof(error_buf));
                        _exit(EXIT_FAILURE);
                }

                _exit(EXIT_SUCCESS);
        }

        pair[1] = safe_close(pair[1]);

        r = wait_for_terminate_and_check("(sd-buscntrns)", child, 0);
        if (r < 0)
                return r;
        if (r != EXIT_SUCCESS)
                return -EPROTO;

        n = read(pair[0], &error_buf, sizeof(error_buf));
        if (n < 0)
                return -errno;

        if (n > 0) {
                if (n != sizeof(error_buf))
                        return -EIO;

                if (error_buf < 0)
                        return -EIO;

                if (error_buf == EINPROGRESS)
                        return 1;

                if (error_buf > 0)
                        return -error_buf;
        }

        return bus_socket_start_auth(b);
}
