/* Generated by mkromfs. Edit with caution. */
#include <rtthread.h>
#include <dfs_romfs.h>
#ifdef RT_USING_DFS_ROMFS
static const struct romfs_dirent _romfs_root[] = {
    {ROMFS_DIRENT_DIR, "FLASH", RT_NULL, 0},
    {ROMFS_DIRENT_DIR, "SD", RT_NULL, 0}
};

const struct romfs_dirent romfs_root = {
    ROMFS_DIRENT_DIR, "/", (rt_uint8_t *)_romfs_root, sizeof(_romfs_root)/sizeof(_romfs_root[0])
};
#endif
