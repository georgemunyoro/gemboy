/*
 * Placeholder interface implementation for the testing framework.
 *
 * The four functions starting with mycpu_ should be provided by your CPU to be
 * tested.
 *
 * Additionally, this file contains an example implementation of the mock MMU
 * that maps the tester's instruction memory area read-only, and tracks all
 * write operations.
 */

#include <string.h>

#include "./src/cpu.h"
#include "lib/tester.h"

static size_t instruction_mem_size;
static uint8_t *instruction_mem;

static int num_mem_accesses;
static struct mem_access mem_accesses[16];

Memory g_Memory;
CPU g_CPU(&g_Memory);

struct tester_operations myops = test_ops;

/*
 * Example mock MMU implementation, mapping the tester's instruction memory
 * read-only at address 0, and logging all writes.
 */

// uint8_t mymmu_read(uint16_t address)
// {
//     if (address < instruction_mem_size)
//         return instruction_mem[address];
//     else
//         return 0xaa;
// }

// void mymmu_write(uint16_t address, uint8_t data)
// {
//     struct mem_access *access = &mem_accesses[num_mem_accesses++];
//     access->type = MEM_ACCESS_WRITE;
//     access->addr = address;
//     access->val = data;
// }
