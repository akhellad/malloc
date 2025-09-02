#include "malloc.h"

static int get_zone_type(size_t size)
{
	if (size <= TINY_MAX_SIZE)
		return ZONE_TINY;
	if (size <= SMALL_MAX_SIZE)
		return ZONE_SMALL;
	return ZONE_LARGE;
}

static int get_block_zone_type(t_block *block)
{
	t_zone *zones[3] = {g_malloc_data.tiny_zones, 
						g_malloc_data.small_zones, 
						g_malloc_data.large_zones};
	int zone_types[3] = {ZONE_TINY, ZONE_SMALL, ZONE_LARGE};
	
	for (int i = 0; i < 3; i++)
	{
		t_zone *current_zone = zones[i];
		while (current_zone)
		{
			if ((char *)block >= (char *)current_zone->start && 
				(char *)block < (char *)current_zone->start + current_zone->size)
			{
				return zone_types[i];
			}
			current_zone = current_zone->next;
		}
	}
	return -1;
}

static int should_reallocate(t_block *block, size_t new_size)
{
	int current_zone = get_block_zone_type(block);
	int required_zone = get_zone_type(new_size);
	
	return (current_zone != required_zone);
}

static t_block *find_block_from_ptr_realloc(void *ptr)
{
	if (!ptr)
		return NULL;
	
	t_block *block = (t_block *)((char *)ptr - sizeof(t_block));
	
	t_zone *zones[3] = {g_malloc_data.tiny_zones, 
						g_malloc_data.small_zones, 
						g_malloc_data.large_zones};
	
	for (int i = 0; i < 3; i++)
	{
		t_zone *current_zone = zones[i];
		while (current_zone)
		{
			t_block *current_block = current_zone->blocks;
			while (current_block)
			{
				if (current_block == block && !current_block->is_free)
					return current_block;
				current_block = current_block->next;
			}
			current_zone = current_zone->next;
		}
	}
	return NULL;
}

static void *memcpy_custom(void *dest, const void *src, size_t n)
{
	char *d = (char *)dest;
	const char *s = (const char *)src;
	
	while (n--)
		*d++ = *s++;
	return dest;
}

static int can_expand_block(t_block *block, size_t new_size)
{
	if (new_size <= block->size)
		return 1;
	
	size_t needed = new_size - block->size;
	t_block *next = block->next;
	
	if (!next)
	{
		t_zone *zones[3] = {g_malloc_data.tiny_zones, 
							g_malloc_data.small_zones, 
							g_malloc_data.large_zones};
		
		for (int i = 0; i < 3; i++)
		{
			t_zone *current_zone = zones[i];
			while (current_zone)
			{
				if ((char *)block >= (char *)current_zone->start && 
					(char *)block < (char *)current_zone->start + current_zone->size)
				{
					return (current_zone->available >= needed);
				}
				current_zone = current_zone->next;
			}
		}
		return 0;
	}
	
	if (next->is_free && next->size + sizeof(t_block) >= needed)
		return 1;
	
	return 0;
}

static void expand_block(t_block *block, size_t new_size)
{
	if (new_size <= block->size)
	{
		return;
	}
	
	size_t needed = new_size - block->size;
	t_block *next = block->next;
	
	if (next && next->is_free && next->size + sizeof(t_block) >= needed)
	{
		if (next->size + sizeof(t_block) == needed)
		{
			block->size = new_size;
			block->next = next->next;
			if (next->next)
				next->next->prev = block;
		}
		else
		{
			block->size = new_size;
			char *new_next_pos = (char *)block + sizeof(t_block) + new_size;
			t_block *new_next = (t_block *)new_next_pos;
			
			new_next->size = next->size + sizeof(t_block) - needed;
			new_next->is_free = 1;
			new_next->next = next->next;
			new_next->prev = block;
			block->next = new_next;
			
			if (next->next)
				next->next->prev = new_next;
		}
	}
	else
	{
		t_zone *zones[3] = {g_malloc_data.tiny_zones, 
							g_malloc_data.small_zones, 
							g_malloc_data.large_zones};
		
		for (int i = 0; i < 3; i++)
		{
			t_zone *current_zone = zones[i];
			while (current_zone)
			{
				if ((char *)block >= (char *)current_zone->start && 
					(char *)block < (char *)current_zone->start + current_zone->size)
				{
					if (current_zone->available >= needed)
					{
						block->size = new_size;
						current_zone->available -= needed;
						return;
					}
				}
				current_zone = current_zone->next;
			}
		}
	}
}

void *realloc(void *ptr, size_t size)
{
	write(1, "[CUSTOM REALLOC] Called\n", 24);
	
	if (!ptr)
		return malloc(size);
	
	if (size == 0)
	{
		free(ptr);
		return NULL;
	}
	
	t_block *block = find_block_from_ptr_realloc(ptr);
	if (!block)
		return NULL;
	
	if (should_reallocate(block, size))
	{
		void *new_ptr = malloc(size);
		if (!new_ptr)
			return NULL;
		
		size_t copy_size = (size < block->size) ? size : block->size;
		memcpy_custom(new_ptr, ptr, copy_size);
		
		free(ptr);
		return new_ptr;
	}
	
	if (size <= block->size)
	{
		if (size < block->size)
		{
			block->size = size;
		}
		return ptr;
	}
	
	if (can_expand_block(block, size))
	{
		expand_block(block, size);
		return ptr;
	}
	
	void *new_ptr = malloc(size);
	if (!new_ptr)
		return NULL;
	
	size_t copy_size = (size < block->size) ? size : block->size;
	memcpy_custom(new_ptr, ptr, copy_size);
	
	free(ptr);
	return new_ptr;
}