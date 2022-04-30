#include "vfs/ustar.h"
#include "memory_management/kheap.h"
#include "utils/string.h"
#include <stdbool.h>

static inline int
oct2bin(char *str, int size) 
{
	int n = 0;
	char *c = str;
	while (size-- > 0) {
		n *= 8;
		n += *c - '0';
		c++;
	}
	return n;
}

char*
ustar_read(void *ustar, const char *const filename)
{
	ustar_entry_t *ustar_ptr = (ustar_entry_t*) ustar;
	while(! strncmp(ustar_ptr->signature, "ustar", 5)) {
		size_t len = oct2bin(ustar_ptr->size, 11);
		if(! strncmp(ustar_ptr->name, filename, strlen(ustar_ptr->name) + 1)) {
			char *res = kalloc(len) + 1;
			memmove(res, (void*) (ustar_ptr + 1), len);
			return res;
		}

		// Calculate the number of 512-byte blocks taken up by the file by
		// rounding file size up to nearest multiple of 512.
		size_t num_blocks = len % BLOCK_SIZE == 0 ? 
			(len / BLOCK_SIZE) : (len / BLOCK_SIZE) + 1;
		ustar_ptr += (num_blocks > 0 ? num_blocks : 1);
	}
	return NULL;
}


ustar_entry_t**
ustar_readdir(void *ustar, const char *const dirname)
{
	bool found_dir = false;
	ustar_entry_t *ustar_ptr = (ustar_entry_t*) ustar;
	ustar_entry_t **results = kalloc(sizeof(ustar_entry_t*));
	size_t num_results = 0, arr_size = 1;

	while(!strncmp(ustar_ptr->signature, "ustar", 5)) {
		size_t len = oct2bin(ustar_ptr->size, 11);

		if(!strncmp(ustar_ptr->name, dirname, strlen(dirname) + 1)) {
			if(ustar_ptr->filetype == USTAR_DIR) {
				found_dir = true;
			} else {
				if(num_results++ > arr_size) {
					arr_size *= 2;
					results = krealloc(results, arr_size * sizeof(ustar_entry_t*));
				}

				results[num_results] = (ustar_entry_t*) kalloc(sizeof(ustar_entry_t));
				memmove(results[num_results], ustar_ptr, sizeof(ustar_entry_t));
			}
		}

		// Calculate the number of 512-byte blocks taken up by the file by
		// rounding file size up to nearest multiple of 512.
		size_t num_blocks = len % BLOCK_SIZE == 0 ? 
			(len / BLOCK_SIZE) : (len / BLOCK_SIZE) + 1;
		ustar_ptr += (num_blocks > 0 ? num_blocks : 1);
	}

	if(found_dir) {
		return results;
	}

	return NULL;
}

