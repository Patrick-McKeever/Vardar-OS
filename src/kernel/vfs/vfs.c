#include "vfs/vfs.h"

uint32_t read_fs(fs_node_t *node, uint32_t offset, uint32_t size, 
				 uint8_t *buffer)
{
	if(node->read) {
		return node->read(node, offset, size, buffer);
	}
	return 0;
}

uint32_t write_fs(fs_node_t *node, uint32_t offset, uint32_t size, 
				  uint8_t *buffer)
{
	if(node->write) {
		return node->write(node, offset, size, buffer);
	}
	return 0;
}

void open_fs(fs_node_t *node, uint8_t read, uint8_t write)
{
	if(node->open) {
		node->open(node);
	}
}

void close_fs(fs_node_t *node)
{
	if(node->close) {
		node->close(node);
	}
}

dirent_t *readdir_fs(fs_node_t *node, uint32_t index)
{
	if((node->flags & 0x07) == FS_DIRECTORY && node->readdir) {
		return node->readdir(node, index);
	}
	return NULL;
}

dirent_t *finddir_fs(fs_node_t *node, char *name)
{
	if((node->flags & 0x07) == FS_DIRECTORY && node->finddir) {
		return node->finddir(node, name);
	}
	return NULL;
}
