#include "fs.h"
#include "errno.h"
#include "string.h"
#include "timer.h"
#include "kprintf.h"
#include "version.h"

static fs_node_t fs_nodes[FS_MAX_FILES];
static fs_fd_t fs_fd_table[FS_MAX_FDS];
static int current_dir = 0;

void fs_init(void) {
    memset(fs_nodes, 0, sizeof(fs_nodes));
    memset(fs_fd_table, 0, sizeof(fs_fd_table));
    for (int i = 0; i < FS_MAX_FDS; i++) fs_fd_table[i].node_idx = -1;

    strcpy(fs_nodes[0].name, "/");
    fs_nodes[0].type = FS_TYPE_DIR;
    fs_nodes[0].parent = -1;
    fs_nodes[0].in_use = 1;
    fs_nodes[0].created_tick = 0;
    fs_mkdir("home");
    fs_mkdir("bin");
    fs_mkdir("etc");
    fs_mkdir("tmp");
    fs_create_file("readme.txt");
    const char* welcome = "Welcome to MyMisu OS!\nThis is a simple ramdisk filesystem.\nType 'help' for commands.\n";
    fs_write_file("readme.txt", welcome, strlen(welcome));
    fs_create_file("hostname");
    fs_write_file("hostname", "misu-pc", 7);
    fs_create_file("version");
    fs_write_file("version", "MyMisu OS " MYMISU_VERSION, strlen("MyMisu OS " MYMISU_VERSION));
}

static int find_free_node(void) {
    for (int i = 1; i < FS_MAX_FILES; i++)
        if (!fs_nodes[i].in_use) return i;
    return -1;
}

static int find_node(const char* name) {
    for (int i = 0; i < FS_MAX_FILES; i++)
        if (fs_nodes[i].in_use && strcmp(fs_nodes[i].name, name) == 0 && fs_nodes[i].parent == current_dir)
            return i;
    return -1;
}

static int find_free_fd(void) {
    for (int i = 0; i < FS_MAX_FDS; i++)
        if (!fs_fd_table[i].in_use) return i;
    return -1;
}

int fs_mkdir(const char* name) {
    if (!name || !*name) return -EINVAL;
    if (strlen(name) >= FS_MAX_NAME) return -ENAMETOOLONG;
    if (find_node(name) >= 0) return -EEXIST;
    int idx = find_free_node();
    if (idx < 0) return -ENOSPC;
    strncpy(fs_nodes[idx].name, name, FS_MAX_NAME - 1);
    fs_nodes[idx].name[FS_MAX_NAME - 1] = '\0';
    fs_nodes[idx].type = FS_TYPE_DIR;
    fs_nodes[idx].size = 0;
    fs_nodes[idx].parent = current_dir;
    fs_nodes[idx].in_use = 1;
    fs_nodes[idx].created_tick = timer_get_ticks();
    return 0;
}

int fs_create_file(const char* name) {
    if (!name || !*name) return -EINVAL;
    if (strlen(name) >= FS_MAX_NAME) return -ENAMETOOLONG;
    if (find_node(name) >= 0) return -EEXIST;
    int idx = find_free_node();
    if (idx < 0) return -ENOSPC;
    strncpy(fs_nodes[idx].name, name, FS_MAX_NAME - 1);
    fs_nodes[idx].name[FS_MAX_NAME - 1] = '\0';
    fs_nodes[idx].type = FS_TYPE_FILE;
    fs_nodes[idx].size = 0;
    fs_nodes[idx].data[0] = '\0';
    fs_nodes[idx].parent = current_dir;
    fs_nodes[idx].in_use = 1;
    fs_nodes[idx].created_tick = timer_get_ticks();
    return 0;
}

int fs_write_file(const char* name, const char* data, size_t len) {
    int idx = find_node(name);
    if (idx < 0) return -ENOENT;
    if (fs_nodes[idx].type != FS_TYPE_FILE) return -EISDIR;
    size_t copy_len = len;
    if (copy_len >= FS_MAX_DATA) copy_len = FS_MAX_DATA - 1;
    memcpy(fs_nodes[idx].data, data, copy_len);
    fs_nodes[idx].data[copy_len] = '\0';
    fs_nodes[idx].size = copy_len;
    return (int)copy_len;
}

int fs_read_file(const char* name, char* buf, size_t max_len) {
    int idx = find_node(name);
    if (idx < 0) return -ENOENT;
    if (fs_nodes[idx].type != FS_TYPE_FILE) return -EISDIR;
    size_t copy_len = fs_nodes[idx].size;
    if (copy_len >= max_len) copy_len = max_len - 1;
    memcpy(buf, fs_nodes[idx].data, copy_len);
    buf[copy_len] = '\0';
    return (int)copy_len;
}

int fs_list(const char* dir_name, char* buf, size_t max_len) {
    (void)dir_name;
    int count = 0;
    size_t offset = 0;
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (fs_nodes[i].in_use && fs_nodes[i].parent == current_dir) {
            const char* type_str = (fs_nodes[i].type == FS_TYPE_DIR) ? "DIR " : "FILE";
            size_t name_len = strlen(fs_nodes[i].name);
            if (offset + 6 + name_len + 1 < max_len) {
                memcpy(buf + offset, type_str, 4);
                offset += 4;
                buf[offset++] = ' ';
                buf[offset++] = ' ';
                memcpy(buf + offset, fs_nodes[i].name, name_len);
                offset += name_len;
                buf[offset++] = '\n';
                count++;
            }
        }
    }
    buf[offset] = '\0';
    return count;
}

int fs_delete(const char* name) {
    int idx = find_node(name);
    if (idx < 0 || idx == 0) return -ENOENT;
    if (fs_nodes[idx].type == FS_TYPE_DIR) {
        for (int i = 0; i < FS_MAX_FILES; i++)
            if (fs_nodes[i].in_use && fs_nodes[i].parent == idx) return -ENOTEMPTY;
    }
    fs_nodes[idx].in_use = 0;
    return 0;
}

int fs_cd(const char* name) {
    if (strcmp(name, "..") == 0) {
        if (fs_nodes[current_dir].parent >= 0)
            current_dir = fs_nodes[current_dir].parent;
        return 0;
    }
    if (strcmp(name, "/") == 0) { current_dir = 0; return 0; }
    int idx = find_node(name);
    if (idx < 0) return -ENOENT;
    if (fs_nodes[idx].type != FS_TYPE_DIR) return -ENOTDIR;
    current_dir = idx;
    return 0;
}

const char* fs_pwd(void) { return fs_nodes[current_dir].name; }

int fs_get_file_count(void) {
    int c = 0;
    for (int i = 0; i < FS_MAX_FILES; i++)
        if (fs_nodes[i].in_use && fs_nodes[i].type == FS_TYPE_FILE) c++;
    return c;
}

int fs_get_dir_count(void) {
    int c = 0;
    for (int i = 0; i < FS_MAX_FILES; i++)
        if (fs_nodes[i].in_use && fs_nodes[i].type == FS_TYPE_DIR) c++;
    return c;
}

fs_node_t* fs_get_node(const char* name) {
    int idx = find_node(name);
    if (idx < 0) return 0;
    return &fs_nodes[idx];
}

/* ---------- New POSIX-style fd-based API ---------- */

int fs_open(const char* name, uint32_t flags) {
    if (!name || !*name) return -EINVAL;
    int idx = find_node(name);
    if (idx < 0) {
        if (flags & O_CREAT) {
            int rc = fs_create_file(name);
            if (rc < 0) return rc;
            idx = find_node(name);
            if (idx < 0) return -EIO;
        } else {
            return -ENOENT;
        }
    }
    if (fs_nodes[idx].type != FS_TYPE_FILE) return -EISDIR;
    if (flags & O_TRUNC) {
        fs_nodes[idx].size = 0;
        fs_nodes[idx].data[0] = '\0';
    }
    int fd = find_free_fd();
    if (fd < 0) return -EMFILE;
    fs_fd_table[fd].node_idx = idx;
    fs_fd_table[fd].offset   = (flags & O_APPEND) ? fs_nodes[idx].size : 0;
    fs_fd_table[fd].flags    = flags;
    fs_fd_table[fd].in_use   = 1;
    return fd;
}

int fs_close(int fd) {
    if (fd < 0 || fd >= FS_MAX_FDS) return -EBADF;
    if (!fs_fd_table[fd].in_use) return -EBADF;
    fs_fd_table[fd].in_use = 0;
    fs_fd_table[fd].node_idx = -1;
    return 0;
}

int fs_read_fd(int fd, char* buf, size_t n) {
    if (fd < 0 || fd >= FS_MAX_FDS || !fs_fd_table[fd].in_use) return -EBADF;
    if (!buf) return -EFAULT;
    fs_fd_t* f = &fs_fd_table[fd];
    if ((f->flags & 0x3) == O_WRONLY) return -EACCES;
    fs_node_t* node = &fs_nodes[f->node_idx];
    if (f->offset >= node->size) return 0;
    size_t avail = node->size - f->offset;
    size_t to_copy = (n < avail) ? n : avail;
    memcpy(buf, node->data + f->offset, to_copy);
    f->offset += to_copy;
    return (int)to_copy;
}

int fs_write_fd(int fd, const char* buf, size_t n) {
    if (fd < 0 || fd >= FS_MAX_FDS || !fs_fd_table[fd].in_use) return -EBADF;
    if (!buf) return -EFAULT;
    fs_fd_t* f = &fs_fd_table[fd];
    if ((f->flags & 0x3) == O_RDONLY) return -EACCES;
    fs_node_t* node = &fs_nodes[f->node_idx];
    if (f->flags & O_APPEND) f->offset = node->size;
    if (f->offset >= FS_MAX_DATA - 1) return -ENOSPC;
    size_t space = (FS_MAX_DATA - 1) - f->offset;
    size_t to_copy = (n < space) ? n : space;
    memcpy(node->data + f->offset, buf, to_copy);
    f->offset += to_copy;
    if (f->offset > node->size) node->size = f->offset;
    node->data[node->size] = '\0';
    return (int)to_copy;
}

int fs_lseek(int fd, int32_t offset, int whence) {
    if (fd < 0 || fd >= FS_MAX_FDS || !fs_fd_table[fd].in_use) return -EBADF;
    fs_fd_t* f = &fs_fd_table[fd];
    fs_node_t* node = &fs_nodes[f->node_idx];
    int32_t new_off;
    if (whence == SEEK_SET)      new_off = offset;
    else if (whence == SEEK_CUR) new_off = (int32_t)f->offset + offset;
    else if (whence == SEEK_END) new_off = (int32_t)node->size + offset;
    else return -EINVAL;
    if (new_off < 0) return -EINVAL;
    if (new_off > (int32_t)FS_MAX_DATA - 1) return -EINVAL;
    f->offset = (uint32_t)new_off;
    return new_off;
}

int fs_stat(const char* name, fs_stat_t* out) {
    if (!name || !out) return -EFAULT;
    int idx = find_node(name);
    if (idx < 0) return -ENOENT;
    out->type         = fs_nodes[idx].type;
    out->size         = fs_nodes[idx].size;
    out->created_tick = fs_nodes[idx].created_tick;
    out->parent       = fs_nodes[idx].parent;
    return 0;
}

int fs_unlink(const char* name) {
    int idx = find_node(name);
    if (idx < 0 || idx == 0) return -ENOENT;
    if (fs_nodes[idx].type != FS_TYPE_FILE) return -EISDIR;
    fs_nodes[idx].in_use = 0;
    return 0;
}

int fs_rmdir(const char* name) {
    int idx = find_node(name);
    if (idx < 0 || idx == 0) return -ENOENT;
    if (fs_nodes[idx].type != FS_TYPE_DIR) return -ENOTDIR;
    for (int i = 0; i < FS_MAX_FILES; i++)
        if (fs_nodes[i].in_use && fs_nodes[i].parent == idx) return -ENOTEMPTY;
    fs_nodes[idx].in_use =
 0;
    return 0;
}

int fs_getcwd(char* buf, size_t n) {
    if (!buf || n == 0) return -EFAULT;
    const char* name = fs_nodes[current_dir].name;
    size_t len = strlen(name);
    if (len + 1 > n) return -EINVAL;
    memcpy(buf, name, len + 1);
    return (int)len;
}

fs_node_t* fs_get_table_ptr(void) { return fs_nodes; }

