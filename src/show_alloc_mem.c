#include "malloc.h"

static void write_str(const char *str)
{
	int len = 0;
	while (str[len])
		len++;
	write(1, str, len);
}

static void write_hex(void *ptr)
{
	unsigned long addr = (unsigned long)ptr;
	char hex[19] = "0x";
	char digits[] = "0123456789ABCDEF";
	int i = 2;
	
	if (addr == 0)
	{
		write(1, "0x0", 3);
		return;
	}
	
	unsigned long temp = addr;
	while (temp > 0)
	{
		hex[i++] = digits[temp % 16];
		temp /= 16;
	}
	
	write(1, hex, 2);
	for (int j = i - 1; j >= 2; j--)
		write(1, &hex[j], 1);
}

static void write_number(size_t num)
{
	if (num == 0)
	{
		write(1, "0", 1);
		return;
	}
	
	char buffer[32];
	int i = 0;
	
	while (num > 0)
	{
		buffer[i++] = '0' + (num % 10);
		num /= 10;
	}
	
	for (int j = i - 1; j >= 0; j--)
		write(1, &buffer[j], 1);
}

static void show_zone_blocks(t_zone *zone, const char *zone_name)
{
	if (!zone)
		return;
	
	t_zone *current_zone = zone;
	
	while (current_zone)
	{
		int has_allocated_blocks = 0;
		t_block *current_block = current_zone->blocks;
		
		while (current_block)
		{
			if (!current_block->is_free)
			{
				has_allocated_blocks = 1;
				break;
			}
			current_block = current_block->next;
		}
		
		if (has_allocated_blocks)
		{
			write_str(zone_name);
			write_str(" : ");
			write_hex(current_zone->start);
			write_str("\n");
			
			current_block = current_zone->blocks;
			while (current_block)
			{
				if (!current_block->is_free)
				{
					void *block_start = (char *)current_block + sizeof(t_block);
					void *block_end = (char *)block_start + current_block->size - 1;
					
					write_hex(block_start);
					write_str(" - ");
					write_hex(block_end);
					write_str(" : ");
					write_number(current_block->size);
					write_str(" bytes\n");
				}
				current_block = current_block->next;
			}
		}
		current_zone = current_zone->next;
	}
}

static size_t calculate_total_allocated(void)
{
	size_t total = 0;
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
				if (!current_block->is_free)
					total += current_block->size;
				current_block = current_block->next;
			}
			current_zone = current_zone->next;
		}
	}
	return total;
}

void show_alloc_mem(void)
{
	show_zone_blocks(g_malloc_data.tiny_zones, "TINY");
	show_zone_blocks(g_malloc_data.small_zones, "SMALL");
	show_zone_blocks(g_malloc_data.large_zones, "LARGE");
	
	write_str("Total : ");
	write_number(calculate_total_allocated());
	write_str(" bytes\n");
}