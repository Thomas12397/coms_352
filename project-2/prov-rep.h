#ifndef PROV_REP_H
#define PROV_REP_H

int open_file();
int get_file_size(int fd);
char* init_mem_map_file(int fd, int size);
void new_alloc_mem_map_file(char *map,int size, int filedesc, int rt, int rv);
void sync_mem_map_file(char *map, int size);
void unmap_mem_map_file(char *map, int size);


#endif
