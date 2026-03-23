#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define FS_MAX_FILES     64
#define FS_MAX_NAME      32
#define FS_MAX_DATA      4096
#define FS_TYPE_FILE     1
#define FS_TYPE_DIR      2

typedef struct {
    char name[FS_MAX_NAME];
    uint8_t type;
    uint32_t size;
    char data[FS_MAX_DATA];
    int parent;
    bool in_use;
    uint32_t created_tick;
} fs_node_t;

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

#endif
