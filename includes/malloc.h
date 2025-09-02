#ifndef MALLOC_H
# define MALLOC_H

# include <sys/mman.h>
# include <unistd.h>
# include <stdlib.h>

# define TINY_MAX_SIZE		128
# define SMALL_MAX_SIZE		1024

# define TINY_ZONE_SIZE		(getpagesize() * 4)
# define SMALL_ZONE_SIZE	(getpagesize() * 32)

# define ZONE_TINY			0
# define ZONE_SMALL			1
# define ZONE_LARGE			2

typedef struct s_block {
	size_t			size;
	int				is_free;
	struct s_block	*next;
	struct s_block	*prev;
}	t_block;

typedef struct s_zone {
	void			*start;
	size_t			size;
	size_t			available;
	int				type;
	struct s_zone	*next;
	t_block			*blocks;
}	t_zone;

typedef struct s_malloc_data {
	t_zone	*tiny_zones;
	t_zone	*small_zones;
	t_zone	*large_zones;
}	t_malloc_data;

extern t_malloc_data	g_malloc_data;

void	*malloc(size_t size);
void	free(void *ptr);
void	*realloc(void *ptr, size_t size);
void	show_alloc_mem(void);

t_block	*find_free_block(t_zone *zones, size_t size);
t_block	*split_block(t_block *block, size_t size);
void	merge_free_blocks(t_zone *zone);

#endif