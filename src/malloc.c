#include "malloc.h"

t_malloc_data g_malloc_data = {NULL, NULL, NULL};

static int get_zone_type(size_t size)
{
	if (size <= TINY_MAX_SIZE)
		return ZONE_TINY;
	if (size <= SMALL_MAX_SIZE)
		return ZONE_SMALL;
	return ZONE_LARGE;
}

static size_t get_zone_size(int type)
{
	if (type == ZONE_TINY)
		return TINY_ZONE_SIZE;
	if (type == ZONE_SMALL)
		return SMALL_ZONE_SIZE;
	return 0;
}

static t_zone *create_zone(int type, size_t block_size)
{
	size_t zone_size;
	
	if (type == ZONE_LARGE)
		zone_size = block_size + sizeof(t_zone) + sizeof(t_block);
	else
		zone_size = get_zone_size(type);
	
	void *ptr = mmap(NULL, zone_size, PROT_READ | PROT_WRITE, 
					 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (ptr == MAP_FAILED)
		return NULL;
	
	t_zone *zone = (t_zone *)ptr;
	zone->start = ptr;
	zone->size = zone_size;
	zone->available = zone_size - sizeof(t_zone);
	zone->type = type;
	zone->next = NULL;
	zone->blocks = NULL;
	
	return zone;
}

static t_block *create_block_in_zone(t_zone *zone, size_t size)
{
	if (zone->available < sizeof(t_block) + size)
		return NULL;
	
	t_block *block;
	
	if (!zone->blocks)
	{
		block = (t_block *)((char *)zone->start + sizeof(t_zone));
		zone->blocks = block;
		block->prev = NULL;
	}
	else
	{
		t_block *last = zone->blocks;
		while (last->next)
			last = last->next;
		
		block = (t_block *)((char *)last + sizeof(t_block) + last->size);
		last->next = block;
		block->prev = last;
	}
	
	block->size = size;
	block->is_free = 0;
	block->next = NULL;
	zone->available -= sizeof(t_block) + size;
	
	return block;
}

t_block *find_free_block(t_zone *zones, size_t size)
{
	t_zone *current_zone = zones;
	
	while (current_zone)
	{
		t_block *current_block = current_zone->blocks;
		while (current_block)
		{
			if (current_block->is_free && current_block->size >= size)
				return current_block;
			current_block = current_block->next;
		}
		current_zone = current_zone->next;
	}
	return NULL;
}

static void add_zone_to_list(t_zone **head, t_zone *new_zone)
{
	if (!*head)
	{
		*head = new_zone;
		return;
	}
	
	t_zone *current = *head;
	while (current->next)
		current = current->next;
	current->next = new_zone;
}

void *malloc(size_t size)
{
	write(1, "[CUSTOM MALLOC] Called\n", 23);
	
	if (size == 0)
		return NULL;
	
	int type = get_zone_type(size);
	t_zone **zone_list;
	
	if (type == ZONE_TINY)
		zone_list = &g_malloc_data.tiny_zones;
	else if (type == ZONE_SMALL)
		zone_list = &g_malloc_data.small_zones;
	else
		zone_list = &g_malloc_data.large_zones;
	
	t_block *block = find_free_block(*zone_list, size);
	
	if (block)
	{
		block->is_free = 0;
		return (char *)block + sizeof(t_block);
	}
	
	t_zone *existing_zone = *zone_list;
	while (existing_zone)
	{
		block = create_block_in_zone(existing_zone, size);
		if (block)
			return (char *)block + sizeof(t_block);
		existing_zone = existing_zone->next;
	}
	
	t_zone *new_zone = create_zone(type, size);
	if (!new_zone)
		return NULL;
	
	add_zone_to_list(zone_list, new_zone);
	
	block = create_block_in_zone(new_zone, size);
	if (!block)
		return NULL;
	
	return (char *)block + sizeof(t_block);
}