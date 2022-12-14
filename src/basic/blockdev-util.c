/* SPDX-License-Identifier: LGPL-2.1+ */

#include <sys/file.h>
#include <unistd.h>

//#include "alloc-util.h"
#include "blockdev-util.h"
#include "btrfs-util.h"
//#include "dirent-util.h"
#include "fd-util.h"
//#include "fileio.h"
//#include "missing_magic.h"
//#include "parse-util.h"
#include "stat-util.h"

#if 0 /// UNNEEDED by elogind
int block_get_whole_disk(dev_t d, dev_t *ret) {
        char p[SYS_BLOCK_PATH_MAX("/partition")];
        _cleanup_free_ char *s = NULL;
        dev_t devt;
        int r;

        assert(ret);

        if (major(d) == 0)
                return -ENODEV;

        /* If it has a queue this is good enough for us */
        xsprintf_sys_block_path(p, "/queue", d);
        if (access(p, F_OK) >= 0) {
                *ret = d;
                return 0;
        }
        if (errno != ENOENT)
                return -errno;

        /* If it is a partition find the originating device */
        xsprintf_sys_block_path(p, "/partition", d);
        if (access(p, F_OK) < 0)
                return -errno;

        /* Get parent dev_t */
        xsprintf_sys_block_path(p, "/../dev", d);
        r = read_one_line_file(p, &s);
        if (r < 0)
                return r;

        r = parse_dev(s, &devt);
        if (r < 0)
                return r;

        /* Only return this if it is really good enough for us. */
        xsprintf_sys_block_path(p, "/queue", devt);
        if (access(p, F_OK) < 0)
                return -errno;

        *ret = devt;
        return 1;
}
#endif // 0

int get_block_device(const char *path, dev_t *ret) {
        _cleanup_close_ int fd = -1;
        struct stat st;
        int r;

        assert(path);
        assert(ret);

        /* Gets the block device directly backing a file system. If the block device is encrypted, returns
         * the device mapper block device. */

        fd = open(path, O_NOFOLLOW|O_CLOEXEC);
        if (fd < 0)
                return -errno;

        if (fstat(fd, &st))
                return -errno;

        if (major(st.st_dev) != 0) {
                *ret = st.st_dev;
                return 1;
        }

        r = btrfs_get_block_device_fd(fd, ret);
        if (r > 0)
                return 1;
        if (r != -ENOTTY) /* not btrfs */
                return r;

        *ret = 0;
        return 0;
}

#if 0 /// UNNEEDED by elogind
int block_get_originating(dev_t dt, dev_t *ret) {
        _cleanup_closedir_ DIR *d = NULL;
        _cleanup_free_ char *t = NULL;
        char p[SYS_BLOCK_PATH_MAX("/slaves")];
        struct dirent *de, *found = NULL;
        const char *q;
        dev_t devt;
        int r;

        /* For the specified block device tries to chase it through the layers, in case LUKS-style DM stacking is used,
         * trying to find the next underlying layer.  */

        xsprintf_sys_block_path(p, "/slaves", dt);
        d = opendir(p);
        if (!d)
                return -errno;

        FOREACH_DIRENT_ALL(de, d, return -errno) {

                if (dot_or_dot_dot(de->d_name))
                        continue;

                if (!IN_SET(de->d_type, DT_LNK, DT_UNKNOWN))
                        continue;

                if (found) {
                        _cleanup_free_ char *u = NULL, *v = NULL, *a = NULL, *b = NULL;

                        /* We found a device backed by multiple other devices. We don't really support automatic
                         * discovery on such setups, with the exception of dm-verity partitions. In this case there are
                         * two backing devices: the data partition and the hash partition. We are fine with such
                         * setups, however, only if both partitions are on the same physical device. Hence, let's
                         * verify this. */

                        u = path_join(p, de->d_name, "../dev");
                        if (!u)
                                return -ENOMEM;

                        v = path_join(p, found->d_name, "../dev");
                        if (!v)
                                return -ENOMEM;

                        r = read_one_line_file(u, &a);
                        if (r < 0)
                                return log_debug_errno(r, "Failed to read %s: %m", u);

                        r = read_one_line_file(v, &b);
                        if (r < 0)
                                return log_debug_errno(r, "Failed to read %s: %m", v);

                        /* Check if the parent device is the same. If not, then the two backing devices are on
                         * different physical devices, and we don't support that. */
                        if (!streq(a, b))
                                return -ENOTUNIQ;
                }

                found = de;
        }

        if (!found)
                return -ENOENT;

        q = strjoina(p, "/", found->d_name, "/dev");

        r = read_one_line_file(q, &t);
        if (r < 0)
                return r;

        r = parse_dev(t, &devt);
        if (r < 0)
                return -EINVAL;

        if (major(devt) == 0)
                return -ENOENT;

        *ret = devt;
        return 1;
}

int get_block_device_harder(const char *path, dev_t *ret) {
        int r;

        assert(path);
        assert(ret);

        /* Gets the backing block device for a file system, and handles LUKS encrypted file systems, looking for its
         * immediate parent, if there is one. */

        r = get_block_device(path, ret);
        if (r <= 0)
                return r;

        r = block_get_originating(*ret, ret);
        if (r < 0)
                log_debug_errno(r, "Failed to chase block device '%s', ignoring: %m", path);

        return 1;
}

int lock_whole_block_device(dev_t devt, int operation) {
        _cleanup_free_ char *whole_node = NULL;
        _cleanup_close_ int lock_fd = -1;
        dev_t whole_devt;
        int r;

        /* Let's get a BSD file lock on the whole block device, as per: https://elogind.io/BLOCK_DEVICE_LOCKING */
        /* Let's get a BSD file lock on the whole block device, as per: https://systemd.io/BLOCK_DEVICE_LOCKING */

        r = block_get_whole_disk(devt, &whole_devt);
        if (r < 0)
                return r;

        r = device_path_make_major_minor(S_IFBLK, whole_devt, &whole_node);
        if (r < 0)
                return r;

        lock_fd = open(whole_node, O_RDONLY|O_CLOEXEC|O_NONBLOCK);
        if (lock_fd < 0)
                return -errno;

        if (flock(lock_fd, operation) < 0)
                return -errno;

        return TAKE_FD(lock_fd);
}
#endif // 0
