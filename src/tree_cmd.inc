/* Recursive tree printer. dir_idx is the inode to print children of. */
static void tree_print_recurse(int dir_idx, int depth, fs_node_t* nodes){
    for(int i=0;i<FS_MAX_FILES;i++){
        if(!nodes[i].in_use) continue;
        if(nodes[i].parent != dir_idx) continue;
        if(i == 0) continue;  /* skip root */

        /* Indent */
        for(int d=0;d<depth;d++) kprintf("    ");

        /* Tree connector */
        terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
        kprintf("|-- ");

        /* Name colored by type */
        if(nodes[i].type == FS_TYPE_DIR){
            terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
            kprintf("%s/\n", nodes[i].name);
            tree_print_recurse(i, depth+1, nodes);
        } else {
            terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
            kprintf("%s", nodes[i].name);
            terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
            kprintf("  (%d bytes)\n", nodes[i].size);
        }
    }
}

static void cmd_tree(void){
    /* Find root via fs_get_node trick - use fs internal table */
    /* We use the fact that fs_nodes is indexed and root is at 0 */
    /* Need access to the static array - so we walk by checking parent==0 from fs_get_node */
    /* Simpler: get nodes via a helper. But we don't have one. So use fs_get_node for each name. */

    /* Actually we have fs_node_t* fs_get_node which only returns by name. We need the table. */
    /* For tonight, walk all 64 slots through a public accessor. */

    extern fs_node_t* fs_get_table_ptr(void);  /* forward decl - we add this to fs */
    fs_node_t* nodes = fs_get_table_ptr();

    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
    kprintf("\n  /\n");
    tree_print_recurse(0, 1, nodes);
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
    kprintf("\n");
}
