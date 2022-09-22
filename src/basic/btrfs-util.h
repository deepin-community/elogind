/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include "sd-id128.h"

#include "copy.h"
#include "time-util.h"

typedef struct BtrfsSubvolInfo {
        uint64_t subvol_id;
        usec_t otime;

        sd_id128_t uuid;
        sd_id128_t parent_uuid;

        bool read_only;
} BtrfsSubvolInfo;

typedef struct BtrfsQuotaInfo {
        uint64_t referenced;
        uint64_t exclusive;
        uint64_t referenced_max;
        uint64_t exclusive_max;
} BtrfsQuotaInfo;

typedef enum BtrfsSnapshotFlags {
        BTRFS_SNAPSHOT_FALLBACK_COPY      = 1 << 0, /* If the source isn't a subvolume, reflink everything */
        BTRFS_SNAPSHOT_READ_ONLY          = 1 << 1,
        BTRFS_SNAPSHOT_RECURSIVE          = 1 << 2,
        BTRFS_SNAPSHOT_QUOTA              = 1 << 3,
        BTRFS_SNAPSHOT_FALLBACK_DIRECTORY = 1 << 4, /* If the destination doesn't support subvolumes, reflink/copy instead */
        BTRFS_SNAPSHOT_FALLBACK_IMMUTABLE = 1 << 5, /* When we can't create a subvolume, use the FS_IMMUTABLE attribute for indicating read-only */
        BTRFS_SNAPSHOT_SIGINT             = 1 << 6, /* Check for SIGINT regularly, and return EINTR if seen */
} BtrfsSnapshotFlags;

typedef enum BtrfsRemoveFlags {
        BTRFS_REMOVE_RECURSIVE = 1 << 0,
        BTRFS_REMOVE_QUOTA     = 1 << 1,
} BtrfsRemoveFlags;

int btrfs_is_filesystem(int fd);

int btrfs_is_subvol_fd(int fd);
#if 0 /// UNNEEDED by elogind
int btrfs_is_subvol(const char *path);
#endif // 0

int btrfs_reflink(int infd, int outfd);
int btrfs_clone_range(int infd, uint64_t in_offset, int ofd, uint64_t out_offset, uint64_t sz);

int btrfs_get_block_device_fd(int fd, dev_t *dev);
#if 0 /// UNNEEDED by elogind
int btrfs_get_block_device(const char *path, dev_t *dev);

int btrfs_defrag_fd(int fd);
int btrfs_defrag(const char *p);

int btrfs_quota_enable_fd(int fd, bool b);
int btrfs_quota_enable(const char *path, bool b);
#endif // 0

int btrfs_quota_scan_start(int fd);
int btrfs_quota_scan_wait(int fd);
#if 0 /// UNNEEDED by elogind
int btrfs_quota_scan_ongoing(int fd);

int btrfs_subvol_make(const char *path);
int btrfs_subvol_make_fd(int fd, const char *subvolume);

int btrfs_subvol_make_fallback(const char *path, mode_t);

int btrfs_subvol_snapshot_fd_full(int old_fd, const char *new_path, BtrfsSnapshotFlags flags, copy_progress_path_t progress_path, copy_progress_bytes_t progress_bytes, void *userdata);
static inline int btrfs_subvol_snapshot_fd(int old_fd, const char *new_path, BtrfsSnapshotFlags flags) {
        return btrfs_subvol_snapshot_fd_full(old_fd, new_path, flags, NULL, NULL, NULL);
}

int btrfs_subvol_snapshot_full(const char *old_path, const char *new_path, BtrfsSnapshotFlags flags, copy_progress_path_t progress_path, copy_progress_bytes_t progress_bytes, void *userdata);
static inline int btrfs_subvol_snapshot(const char *old_path, const char *new_path, BtrfsSnapshotFlags flags) {
        return btrfs_subvol_snapshot_full(old_path, new_path, flags, NULL, NULL, NULL);
}
#endif // 0

int btrfs_subvol_remove(const char *path, BtrfsRemoveFlags flags);
int btrfs_subvol_remove_fd(int fd, const char *subvolume, BtrfsRemoveFlags flags);

int btrfs_subvol_set_read_only_fd(int fd, bool b);
#if 0 /// UNNEEDED by elogind
int btrfs_subvol_set_read_only(const char *path, bool b);
int btrfs_subvol_get_read_only_fd(int fd);

int btrfs_subvol_get_id(int fd, const char *subvolume, uint64_t *ret);
#endif // 0
int btrfs_subvol_get_id_fd(int fd, uint64_t *ret);
#if 0 /// UNNEEDED by elogind
int btrfs_subvol_get_parent(int fd, uint64_t subvol_id, uint64_t *ret);

int btrfs_subvol_get_info_fd(int fd, uint64_t subvol_id, BtrfsSubvolInfo *info);

int btrfs_subvol_find_subtree_qgroup(int fd, uint64_t subvol_id, uint64_t *ret);

int btrfs_subvol_get_subtree_quota(const char *path, uint64_t subvol_id, BtrfsQuotaInfo *quota);
int btrfs_subvol_get_subtree_quota_fd(int fd, uint64_t subvol_id, BtrfsQuotaInfo *quota);

int btrfs_subvol_set_subtree_quota_limit(const char *path, uint64_t subvol_id, uint64_t referenced_max);
int btrfs_subvol_set_subtree_quota_limit_fd(int fd, uint64_t subvol_id, uint64_t referenced_max);

int btrfs_subvol_auto_qgroup_fd(int fd, uint64_t subvol_id, bool new_qgroup);
int btrfs_subvol_auto_qgroup(const char *path, uint64_t subvol_id, bool create_intermediary_qgroup);

int btrfs_qgroupid_make(uint64_t level, uint64_t id, uint64_t *ret);
#endif // 0
int btrfs_qgroupid_split(uint64_t qgroupid, uint64_t *level, uint64_t *id);

#if 0 /// UNNEEDED by elogind
int btrfs_qgroup_create(int fd, uint64_t qgroupid);
#endif // 0
int btrfs_qgroup_destroy(int fd, uint64_t qgroupid);
int btrfs_qgroup_destroy_recursive(int fd, uint64_t qgroupid);

#if 0 /// UNNEEDED by elogind
int btrfs_qgroup_set_limit_fd(int fd, uint64_t qgroupid, uint64_t referenced_max);
int btrfs_qgroup_set_limit(const char *path, uint64_t qgroupid, uint64_t referenced_max);

int btrfs_qgroup_copy_limits(int fd, uint64_t old_qgroupid, uint64_t new_qgroupid);

int btrfs_qgroup_assign(int fd, uint64_t child, uint64_t parent);
#endif // 0
int btrfs_qgroup_unassign(int fd, uint64_t child, uint64_t parent);

int btrfs_qgroup_find_parents(int fd, uint64_t qgroupid, uint64_t **ret);

#if 0 /// UNNEEDED by elogind
int btrfs_qgroup_get_quota_fd(int fd, uint64_t qgroupid, BtrfsQuotaInfo *quota);
int btrfs_qgroup_get_quota(const char *path, uint64_t qgroupid, BtrfsQuotaInfo *quota);

static inline int btrfs_log_dev_root(int level, int ret, const char *p) {
        return log_full_errno(level, ret,
                              "File system behind %s is reported by btrfs to be backed by pseudo-device /dev/root, which is not a valid userspace accessible device node. "
                              "Cannot determine correct backing block device.", p);
}
#endif // 0