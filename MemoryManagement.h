/*
 * MemoryManagement.h
 *
 * @author Peter Voigt
 */
#ifndef MEMORY_MANAGEMENT_H
#define MEMORY_MANAGEMENT_H

#include <iostream>
#include <string>
#include <cstdint>

/*
 * MEMORY_SIZE  The memory size in bytes.
 *              eg. 1 Megabyte -> 1000000 Bytes
 *                  1 Mebibyte -> 1048576 Bytes
 *
 * BLOCK_SIZE   The size of an address block in bytes.
 *              1   ->   8-Bit address   ->   eg. 10110000 = 176
 *              2   ->  16-Bit address   ->   eg. 00100011 00000100 = 8964
 *              4   ->  32-Bit address   ->   eg. 00100011 00000100 00100011 00000100 = 587473668
 *              8   ->  64-Bit address   ->   eg. 00100011 00000100 00100011 00000100 00100011 00000100 00100011 00000100 = 2523180191908635396
 */
#define MEMORY_SIZE 200
#define BLOCK_SIZE 8


/*
 * DATA    An array of bytes representing the actual memory.
 * 
 * _FREE_  The address of the first free chunk in the memory.
 *
 * _USED_  The address of the first reserved chunk in the memory.
 */
extern uint8_t DATA[MEMORY_SIZE];
extern uint64_t _FREE_;
extern uint64_t _USED_;

void initMem();
uint8_t * myAlloc(uint64_t size);
bool myFree(uint8_t * ptr);
void dumpMem(uint64_t from, uint64_t to);

uint64_t _combine_2(uint8_t a, uint8_t b);
uint64_t _combine_4(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
uint64_t _combine_8(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f, uint8_t g, uint8_t h);
std::string _int_bin(uint8_t num);
int _num_digits(uint64_t num);
void _write_block(uint64_t value, uint64_t location);
uint64_t _read_block(uint64_t location);

#endif
