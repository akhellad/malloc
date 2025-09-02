#include <stdio.h>
#include <string.h>
#include "malloc.h"

void print_separator(const char *title) {
	printf("\n=== %s ===\n", title);
}

void test_basic_malloc() {
	print_separator("1. BASIC MALLOC TESTS");
	
	printf("Before any allocation:\n");
	show_alloc_mem();
	
	printf("\nAllocating TINY (50B), SMALL (500B), LARGE (2000B):\n");
	char *tiny = malloc(50);
	char *small = malloc(500);
	char *large = malloc(2000);
	
	printf("tiny: %p, small: %p, large: %p\n", tiny, small, large);
	
	if (tiny) strcpy(tiny, "TINY");
	if (small) strcpy(small, "SMALL");
	if (large) strcpy(large, "LARGE");
	
	printf("Content: %s, %s, %s\n", tiny, small, large);
	show_alloc_mem();
	
	free(tiny);
	free(small);
	free(large);
}

void test_edge_cases() {
	print_separator("2. EDGE CASES");
	
	printf("malloc(0): ");
	char *ptr0 = malloc(0);
	printf("%p\n", ptr0);
	
	printf("malloc(1): ");
	char *ptr1 = malloc(1);
	printf("%p\n", ptr1);
	if (ptr1) ptr1[0] = 'X';
	
	printf("malloc(TINY_MAX=128): ");
	char *ptr128 = malloc(128);
	printf("%p\n", ptr128);
	
	printf("malloc(TINY_MAX+1=129): ");
	char *ptr129 = malloc(129);
	printf("%p\n", ptr129);
	
	printf("malloc(SMALL_MAX=1024): ");
	char *ptr1024 = malloc(1024);
	printf("%p\n", ptr1024);
	
	printf("malloc(SMALL_MAX+1=1025): ");
	char *ptr1025 = malloc(1025);
	printf("%p\n", ptr1025);
	
	show_alloc_mem();
	
	printf("free(NULL): ");
	free(NULL);
	printf("OK\n");
	
	free(ptr1);
	free(ptr128);
	free(ptr129);
	free(ptr1024);
	free(ptr1025);
}

void test_fragmentation() {
	print_separator("3. FRAGMENTATION TESTS");
	
	printf("Allocating multiple TINY blocks:\n");
	char *tiny_blocks[5];
	for (int i = 0; i < 5; i++) {
		tiny_blocks[i] = malloc(50 + i * 10);
		printf("Block %d: %p\n", i, tiny_blocks[i]);
	}
	
	show_alloc_mem();
	
	printf("\nFreeing blocks 1 and 3 (creating holes):\n");
	free(tiny_blocks[1]);
	free(tiny_blocks[3]);
	
	show_alloc_mem();
	
	printf("\nAllocating new blocks (should reuse holes):\n");
	char *new1 = malloc(55);
	char *new2 = malloc(65);
	printf("new1: %p, new2: %p\n", new1, new2);
	
	show_alloc_mem();
	
	free(tiny_blocks[0]);
	free(tiny_blocks[2]);
	free(tiny_blocks[4]);
	free(new1);
	free(new2);
}

void test_realloc_comprehensive() {
	print_separator("4. COMPREHENSIVE REALLOC TESTS");
	
	printf("realloc(NULL, 100) - should act like malloc:\n");
	char *ptr = realloc(NULL, 100);
	printf("ptr: %p\n", ptr);
	strcpy(ptr, "Hello World");
	show_alloc_mem();
	
	printf("\nExpanding 100 -> 200 (same zone):\n");
	ptr = realloc(ptr, 200);
	printf("ptr: %p, content: %s\n", ptr, ptr);
	strcat(ptr, " - Extended!");
	show_alloc_mem();
	
	printf("\nShrinking 200 -> 50:\n");
	ptr = realloc(ptr, 50);
	printf("ptr: %p, content: %s\n", ptr, ptr);
	show_alloc_mem();
	
	printf("\nExpanding 50 -> 1500 (zone change TINY->SMALL):\n");
	ptr = realloc(ptr, 1500);
	printf("ptr: %p, content: %s\n", ptr, ptr);
	show_alloc_mem();
	
	printf("\nExpanding 1500 -> 3000 (zone change SMALL->LARGE):\n");
	ptr = realloc(ptr, 3000);
	printf("ptr: %p, content: %s\n", ptr, ptr);
	show_alloc_mem();
	
	printf("\nrealloc(ptr, 0) - should act like free:\n");
	ptr = realloc(ptr, 0);
	printf("ptr after realloc(ptr, 0): %p\n", ptr);
	show_alloc_mem();
}

void test_stress() {
	print_separator("5. STRESS TEST");
	
	char *ptrs[20];
	
	printf("Allocating 20 blocks of varying sizes:\n");
	for (int i = 0; i < 20; i++) {
		size_t size = 10 + i * 100;  // 10, 110, 210, ..., 1910
		ptrs[i] = malloc(size);
		if (ptrs[i]) {
			snprintf(ptrs[i], size, "Block_%d", i);
		}
	}
	
	show_alloc_mem();
	
	printf("\nFreeing every other block:\n");
	for (int i = 0; i < 20; i += 2) {
		free(ptrs[i]);
		ptrs[i] = NULL;
	}
	
	show_alloc_mem();
	
	printf("\nReallocating remaining blocks:\n");
	for (int i = 1; i < 20; i += 2) {
		if (ptrs[i]) {
			size_t new_size = 50 + i * 50;
			ptrs[i] = realloc(ptrs[i], new_size);
			printf("Block %d reallocated to %zu bytes: %p\n", i, new_size, ptrs[i]);
		}
	}
	
	show_alloc_mem();
	
	printf("\nFinal cleanup:\n");
	for (int i = 1; i < 20; i += 2) {
		free(ptrs[i]);
	}
	
	show_alloc_mem();
}

void test_memory_integrity() {
	print_separator("6. MEMORY INTEGRITY TEST");
	
	printf("Testing data preservation through operations:\n");
	
	char *ptr = malloc(1000);
	strcpy(ptr, "This is a test string that should be preserved through realloc operations. ");
	strcat(ptr, "It contains some data that we want to verify remains intact.");
	
	printf("Original: %.50s...\n", ptr);
	
	ptr = realloc(ptr, 2000);
	printf("After expand to 2000: %.50s...\n", ptr);
	
	ptr = realloc(ptr, 500);
	printf("After shrink to 500: %.50s...\n", ptr);
	
	ptr = realloc(ptr, 3000);
	printf("After expand to 3000: %.50s...\n", ptr);
	
	show_alloc_mem();
	free(ptr);
}

int main() {
	printf("=== COMPREHENSIVE MALLOC TEST SUITE ===\n");
	printf("Testing custom malloc implementation...\n");
	
	test_basic_malloc();
	test_edge_cases();
	test_fragmentation();
	test_realloc_comprehensive();
	test_stress();
	test_memory_integrity();
	
	print_separator("FINAL STATE");
	printf("All tests completed. Final memory state:\n");
	show_alloc_mem();
	
	printf("\n=== TEST SUITE COMPLETED ===\n");
	return 0;
}
