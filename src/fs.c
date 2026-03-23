#include "fs.h"
#include "string.h"
#include "timer.h"
#include "kprintf.h"

static fs_node_t fs_nodes[FS_MAX_FILES];
static int current_dir = 0;

void fs_init(void) {
    memset(fs_nodes, 0, sizeof(fs_nodes));

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
    const char* hostname = "misu-pc";
    fs_write_file("hostname", hostname, strlen(hostname));

    fs_create_file("version");
    const char* version = "MyMisu OS v0.3.0";
    fs_write_file("version", version, strlen(version));
}

static int find_free_node(void) {
    for (int i = 1; i < FS_MAX_FILES; i++)
        if (!fs_nodes[i].in_use) return i;
    return -1;
}

static int find_node(const char* name) {
    for (int i = 0; i < FS_MAX_FILES; i++)
        if (fs_nodes[i].in_use && strcmp(fs_nodes[i].name, name) == 0 &&
            fs_nodes[i].parent == current_dir)
            return i;
    return -1;
}

int fs_mkdir(const char* name) {
    if (find_node(name) >= 0) return -1;
    int idx = find_free_node();
    if (idx < 0) return -1;

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
    if (find_node(name) >= 0) return -1;
    int idx = find_free_node();
    if (idx < 0) return -1;

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
    if (idx < 0) return -1;
    if (fs_nodes[idx].type != FS_TYPE_FILE) return -1;

    size_t copy_len = len;
    if (copy_len >= FS_MAX_DATA) copy_len = FS_MAX_DATA - 1;

    memcpy(fs_nodes[idx].data, data, copy_len);
    fs_nodes[idx].data[copy_len] = '\0';
    fs_nodes[idx].size = copy_len;
    return (int)copy_len;
}

int fs_read_file(const char* name, char* buf, size_t max_len) {
    int idx = find_node(name);
    if (idx < 0) return -1;
    if (fs_nodes[idx].type != FS_TYPE_FILE) return -1;

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
            size_t type_len = 4;

            if (offset + type_len + 2 + name_len + 1 < max_len) {
                memcpy(buf + offset, type_str, type_len);
                offset += type_len;
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
    if (idx < 0) return -1;
    if (idx == 0) return -1;

    if (fs_nodes[idx].type == FS_TYPE_DIR) {
        for (int i = 0; i < FS_MAX_FILES; i++)
            if (fs_nodes[i].in_use && fs_nodes[i].parent == idx)
                return -1;
    }

    fs_nodes[idx].in_use = 0;
    return 0;
}

int fs_get_file_count(void) {
    int count = 0;
    for (int i = 0; i < FS_MAX_FILES; i++)
        if (fs_nodes[i].in_use && fs_nodes[i].type == FS_TYPE_FILE) count++;
    return count;
}

int fs_get_dir_count(void) {
    int count = 0;
    for (int i = 0; i < FS_MAX_FILES; i++)
        if (fs_nodes[i].in_use && fs_nodes[i].type == FS_TYPE_DIR) count++;
    return count;
}

fs_node_t* fs_get_node(const char* name) {
    int idx = find_node(name);
    if (idx < 0) return 0;
    return &fs_nodes[idx];
}
