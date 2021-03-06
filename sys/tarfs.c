#include <sys/tarfs.h>
#include <sys/utils.h>
#include <sys/defs.h>
#include <sys/string.h>
#include <sys/kprintf.h>
#include <sys/dirent.h>
#include <sys/memory.h>

/* Get TARFS header for the filename */
posix_header_ustar *get_file(char *filename) {
    char *p = &_binary_tarfs_start, relative_path[100];
    int size, i;
    posix_header_ustar *phu;

    /* Remove root directory from file path */
    if (filename[0] == '/') {
        i = 1;
        while (filename[i++] != '/');
        strcpy(relative_path, filename + i);
    }

    while (p < &_binary_tarfs_end) {
        phu = (posix_header_ustar*)p;
        if (strcmp(phu->name, relative_path) == 0) {
            return phu;
        }

        size = oct_to_dec(atoi(phu->size));
        p += sizeof(posix_header_ustar);
        if (size == 0) {
            continue;
        }

        // 512 byte aligned address
        p += ROUND_UP(size, BLOCK_SIZE);
    }

    return NULL;
}

void create_new_node(file_node *current_node, file_node *parent_node, char *name, uint64_t first, uint64_t last, int type) {
    strcpy(current_node->name, name);
    current_node->first = first;
    current_node->last = last;
    current_node->cursor = first;
    current_node->type  = type;

    current_node->child[0] = current_node;
    current_node->child[1] = parent_node;
}

// http://www.grymoire.com/Unix/Inodes.html
void parse(char *dir_path, int type, uint64_t first, uint64_t last) {
    file_node *temp_node, *aux_node, *currnode = root_node->child[2];
    char *temp;
    int i;

    char *path = (char *)kmalloc(sizeof(char) * strlen(dir_path));
    strcpy(path, dir_path);

    temp = strtok(path, "/");
    while (temp != NULL) {
        aux_node = currnode;

        // iterate through all childrens of currnode
        for (i = 2; i < currnode->last; i++){
            if (strcmp(temp, currnode->child[i]->name) == 0) {
                currnode = (file_node *)currnode->child[i];
                break;
            }
        }

        // if no child has been found add this as child of current
        if (i == aux_node->last) {

            temp_node = (file_node *)kmalloc(sizeof(file_node));
            create_new_node(temp_node, currnode, temp, first, last, type);

            currnode->child[currnode->last] = temp_node;
            currnode->last += 1;
        }

        temp = strtok(NULL, "/");
    }
}

/* Create a hierarchical directory structure */
void* init_tarfs() {
    char *p = &_binary_tarfs_start;
    posix_header_ustar *phu;
    int size;
    file_node *temp_node;

    root_node = (file_node *)kmalloc(sizeof(file_node));
    create_new_node(root_node, root_node, "/", 0, 2, DIRECTORY);

    temp_node = (file_node *)kmalloc(sizeof(file_node));
    create_new_node(temp_node, root_node, "rootfs", 0, 2, DIRECTORY);
    root_node->child[2] = temp_node;
    root_node->last += 1;

    while (p < &_binary_tarfs_end) {
        phu = (posix_header_ustar*)p;
        size = oct_to_dec(atoi(phu->size));

        if (strcmp(phu->typeflag, "5") == 0) {
            parse(phu->name, DIRECTORY, 0, 2);
        } else {
            parse(phu->name, FILE, (uint64_t)(phu + 1), (uint64_t)((void *)phu + sizeof(posix_header_ustar) + size));
        }

        p += sizeof(posix_header_ustar);
        if (size == 0) {
            continue;
        }

        // 512 byte aligned address
        p += ROUND_UP(size, BLOCK_SIZE);
    }

    return (void *)&root_node;
}
