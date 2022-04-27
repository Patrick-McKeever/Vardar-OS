#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>

struct __fs_node;
struct __dirent;
typedef uint32_t 			(*read_func_t)		(struct __fs_node*, uint32_t, uint32_t, uint8_t*);
typedef uint32_t 			(*write_func_t)		(struct __fs_node*, uint32_t, uint32_t, uint8_t*);
typedef void 				(*open_func_t)		(struct __fs_node*);
typedef void 				(*close_func_t)		(struct __fs_node*);
typedef struct __dirent*	(*readdir_func_t)	(struct __fs_node*, uint32_t);
typedef struct __dirent*	(*finddir_func_t)	(struct __fs_node*, char*);

typedef struct __fs_node {
	char 				name[128];
	uint32_t 			mask;
	uint32_t 			uid;
	uint32_t 			gid;
	uint32_t 			flags;
	uint32_t 			inode;
	uint32_t 			length;
	uint32_t 			impl;
	read_func_t 		read;
	write_func_t 		write;
	open_func_t 		open;
	close_func_t 		close;
	readdir_func_t 		readdir;
	finddir_func_t 		finddir;
	struct __fs_node 	*ptr;
} fs_node_t;


typedef struct __dirent {
	char name[128];
	uint32_t ino;
} dirent_t;

typedef enum {
	FS_REGULAR			=		0x0,
	FS_HARDLINK			=		0x1,
	FS_SYMLINK			=		0x2,
	FS_CHAR_DEV			=		0x3,
	FS_BLOCK_DEV		=		0x4,
	FS_DIRECTORY		=		0x5,
	FS_PIPE				=		0x6
} fs_filetype_t;

uint32_t read_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t write_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
void open_fs(fs_node_t *node, uint8_t read, uint8_t write);
void close_fs(fs_node_t *node);
dirent_t *readdir_fs(fs_node_t *node, uint32_t index);
dirent_t *finddir_fs(fs_node_t *node, char *name);


#endif
