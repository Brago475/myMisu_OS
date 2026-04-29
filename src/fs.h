#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define FS_MAX_FILES     64
#define FS_MAX_NAME      32
#define FS_MAX_DATA      4096
#define FS_MAX_FDS       16

#define FS_TYPE_FILE     1
#define FS_TYPE_DIR      2

/* open() flags */
#define O_RDONLY   0x0000
#define O_WRONLY   0x0001
#define O_RDWR     0x0002
#define O_CREAT    0x0040
#define O_TRUNC    0x0200
#define O_APPEND   0x0400

/* lseek() whence values */
#define SEEK_SET   0
#define SEEK_CUR   1
#define SEEK_END   2

typedef struct {
    char name[FS_MAX_NAME];
    uint8_t type;
    uint32_t size;
    char data[FS_MAX_DATA];
    int parent;
    bool in_use;
    uint32_t created_tick;
} fs_node_t;

/* File descriptor table entry */
typedef struct {
    int node_idx;       /* index into fs_nodes, -1 = free */
    uint32_t offset;    /* current read/write offset      */
    uint32_t flags;     /* open flags (O_RDONLY etc)      */
    bool in_use;
} fs_fd_t;

/* stat() result */
typedef struct {
    uint8_t type;
    uint32_t size;
    uint32_t created_tick;
    int parent;
} fs_stat_t;

/* ---------- Original name-based API (kept for compatibility) ---------- */
void fs_init(void);
int fs_mkdir(const char* name);
int fs_create_file(const char* name);
int fs_write_file(const char* name, const char* data, size_t len);
int fs_read_file(const char* name, char* buf, size_t max_len);
int fs_list(const char* dir_name, char* buf, size_t max_len);
int fs_delete(const char* name);
int fs_cd(const char* name);
const char* fs_pwd(void);
int fs_get_file_count(void);
int fs_get_dir_count(void);
fs_node_t* fs_get_node(const char* name);

/* ---------- New POSIX-style fd-based API ----------
 * All return >= 0 on success or a negative errno on failure.
 */
int fs_open(const char* name, uint32_t flags);
int fs_close(int fd);
int fs_read_fd(int fd, char* buf, size_t n);
int fs_write_fd(int fd, const char* buf, size_t n);
int fs_lseek(int fd, int32_t offset, int whence);
int fs_stat(const char* name, fs_stat_t* out);
int fs_unlink(const char* name);
int fs_rmdir(const char* name);
int fs_getcwd(char* buf, size_t n);

#endif
