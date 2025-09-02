#include "malloc.h"

static t_block *find_block_from_ptr(void *ptr)
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

static void merge_adjacent_blocks(t_zone *zone, t_block *block)
{
	t_block *next = block->next;
	t_block *prev = block->prev;
	
	if (next && next->is_free)
	{
		char *block_end = (char *)block + sizeof(t_block) + block->size;
		if (block_end == (char *)next)
		{
			block->size += sizeof(t_block) + next->size;
			block->next = next->next;
			if (next->next)
				next->next->prev = block;
			zone->available += sizeof(t_block);
		}
	}
	
	if (prev && prev->is_free)
	{
		char *prev_end = (char *)prev + sizeof(t_block) + prev->size;
		if (prev_end == (char *)block)
		{
			prev->size += sizeof(t_block) + block->size;
			prev->next = block->next;
			if (block->next)
				block->next->prev = prev;
			zone->available += sizeof(t_block);
		}
	}
}

static int zone_is_empty(t_zone *zone)
{
	t_block *current = zone->blocks;
	
	while (current)
	{
		if (!current->is_free)
			return 0;
		current = current->next;
	}
	return 1;
}

static void remove_zone_from_list(t_zone **head, t_zone *zone_to_remove)
{
	if (!*head || !zone_to_remove)
		return;
	
	if (*head == zone_to_remove)
	{
		*head = zone_to_remove->next;
		return;
	}
	
	t_zone *current = *head;
	while (current->next && current->next != zone_to_remove)
		current = current->next;
	
	if (current->next == zone_to_remove)
		current->next = zone_to_remove->next;
}

static t_zone *find_zone_containing_block(t_block *block)
{
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
				if (current_block == block)
					return current_zone;
				current_block = current_block->next;
			}
			current_zone = current_zone->next;
		}
	}
	return NULL;
}

void free(void *ptr)
{
	write(1, "[CUSTOM FREE] Called\n", 21);
	
	if (!ptr)
		return;
	
	t_block *block = find_block_from_ptr(ptr);
	if (!block)
		return;
	
	t_zone *zone = find_zone_containing_block(block);
	if (!zone)
		return;
	
	block->is_free = 1;
	zone->available += block->size;
	
	merge_adjacent_blocks(zone, block);
	
	if (zone->type == ZONE_LARGE || zone_is_empty(zone))
	{
		t_zone **zone_list;
		if (zone->type == ZONE_TINY)
			zone_list = &g_malloc_data.tiny_zones;
		else if (zone->type == ZONE_SMALL)
			zone_list = &g_malloc_data.small_zones;
		else
			zone_list = &g_malloc_data.large_zones;
		
		remove_zone_from_list(zone_list, zone);
		munmap(zone->start, zone->size);
	}
}