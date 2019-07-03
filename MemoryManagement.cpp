/*
 * MemoryManagement.cpp
 *
 * @author Peter Voigt
 */
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <string>
#include <math.h>
#include "MemoryManagement.h"

extern uint8_t DATA[MEMORY_SIZE] = {};
extern uint64_t _FREE_ = 0;
extern uint64_t _USED_ = 0;

static bool initialized = false;

void initMem() {
	// initializing the memory
	if(initialized == false) {
		// resetting the pointers
		_FREE_ = 0;
		_USED_ = MEMORY_SIZE;
		// clearing the memory
		for(int i = 0; i < BLOCK_SIZE; i++) {
			DATA[i] = 0;
		}

		// calculating available memory size
		uint64_t available = MEMORY_SIZE - (2 * BLOCK_SIZE);

		// writing 1st chunk description
		#if BLOCK_SIZE == 8
			DATA[8]  = available >> 56;
			DATA[9]  = available >> 48;
			DATA[10] = available >> 40;
			DATA[11] = available >> 32;
			DATA[12] = available >> 24;
			DATA[13] = available >> 16;
			DATA[14] = available >> 8;
			DATA[15] = available;
		#elif BLOCK_SIZE == 2
			DATA[2] = available >> 8;
			DATA[3] = available;
		#elif BLOCK_SIZE == 1
			DATA[1] = available;
		#else
			DATA[4] = available >> 24;
			DATA[5] = available >> 16;
			DATA[6] = available >> 8;
			DATA[7] = available;
		#endif

		initialized = true;
	}
}

uint8_t * myAlloc(uint64_t size) {
	initMem();

	//
	// cycling through all free chunks in the memory until one large enough is found
	//
	
	uint64_t freeAddr = _FREE_;
	uint64_t freeSize;

	bool updateFirstAddr = true;
	bool foundSpace = false;
	uint64_t prevFreeChunk = _FREE_;

	while(true) {
		freeSize = _read_block(freeAddr + BLOCK_SIZE);

		if(freeSize >= size) {
			foundSpace = true;
			break;
		}

		updateFirstAddr = false;
		prevFreeChunk = freeAddr;

		if(freeAddr == 0) {
			break;
		} else {
			freeAddr = _read_block(freeAddr);
		}
	}

	// return nullptr if no free space is available
	if(foundSpace == false) {
		return nullptr;
	}

	
	//
	// creating a free chunk with leftover space from the prev chunk
	//
	
	uint64_t toAllocate = freeSize;
	uint8_t neededPadding = 2 * BLOCK_SIZE;

	
	if(size + neededPadding < freeSize) {
		uint64_t blockAddr = freeAddr + (2 * BLOCK_SIZE) + size;
		uint64_t newSpace = freeSize - (size + neededPadding);

		uint64_t newAddr = _read_block(freeAddr);
		
		_write_block(newAddr, blockAddr);
		_write_block(newSpace, blockAddr + BLOCK_SIZE);

		if(updateFirstAddr) {
			_FREE_ = blockAddr;
		} else {
			_write_block(blockAddr, prevFreeChunk);
		}

		toAllocate = size;
	} else if(updateFirstAddr) {
		uint64_t newFirst = _read_block(freeAddr);
		_FREE_ = newFirst;
	}

	_write_block(toAllocate, freeAddr + BLOCK_SIZE);


	//
	// updating refrences of used chunks
	//
	
	if(freeAddr < _USED_) {
		
		if(_USED_ == MEMORY_SIZE) {
			_write_block(0, freeAddr);
		} else {
			_write_block(_USED_, freeAddr);
		}
		
		_USED_ = freeAddr;
	} else {
		
		//
		// cycling through the used chunks to find the ones surrounding the prev allocated one
		//
		uint64_t prevUsed = _USED_;
		uint64_t nextUsed;
		bool lastBlock = false;

		while(true) {
			nextUsed = _read_block(prevUsed);

			if(nextUsed == 0) {
				lastBlock = true;
				break;
			}

			if(freeAddr > prevUsed && freeAddr < nextUsed) {
				break;
			}

			prevUsed = nextUsed;
		}

		//
		// chaining the prev allocated chunk to the surrounding ones
		//
		if(lastBlock) {
			_write_block(freeAddr, prevUsed);
			_write_block(0, freeAddr);
		} else {
			_write_block(nextUsed, freeAddr);
			_write_block(freeAddr, prevUsed);
		}

	}

	// returning the pointer to the allocated chunk
	return &DATA[freeAddr + (2 * BLOCK_SIZE)];
}

bool myFree(uint8_t * ptr) {
	initMem();

	
	//
	// looking up the chunk (ptr) is pointing to
	//
	
	uint64_t ptrAddr;

	#if BLOCK_SIZE == 8
		ptrAddr = _combine_8(*(ptr - (2 * BLOCK_SIZE)), *(ptr - (2 * BLOCK_SIZE) + 1), *(ptr - (2 * BLOCK_SIZE) + 2), *(ptr - (2 * BLOCK_SIZE) + 3), *(ptr - (2 * BLOCK_SIZE) + 4), *(ptr - (2 * BLOCK_SIZE) + 5), *(ptr - (2 * BLOCK_SIZE) + 6), *(ptr - (2 * BLOCK_SIZE) + 7));
	#elif BLOCK_SIZE == 2
		ptrAddr = _combine_2(*(ptr - (2 * BLOCK_SIZE)), *(ptr - (2 * BLOCK_SIZE) + 1));
	#elif BLOCK_SIZE == 1
		ptrAddr = *(ptr - (2 * BLOCK_SIZE));
	#else
		ptrAddr = _combine_4(*(ptr - (2 * BLOCK_SIZE)), *(ptr - (2 * BLOCK_SIZE) + 1), *(ptr - (2 * BLOCK_SIZE) + 2), *(ptr - (2 * BLOCK_SIZE) + 3));
	#endif

	bool foundChunk = false;
	uint64_t ptrLocation = 0;
	uint64_t prevUsedChunk = 0;
	uint64_t lastUsed = _USED_;

	while(true) {
		prevUsedChunk = ptrLocation;
		ptrLocation = lastUsed;
		lastUsed = _read_block(lastUsed);

		if(lastUsed == ptrAddr) {
			foundChunk = true;
			break;
		}

		if(lastUsed == 0) {
			break;
		}
	}

	// returning false if the chunk the given pointer is pointing to, doesn't exist
	if(foundChunk == false) {
		return false;
	}

	uint64_t _value_0_ptrLocation = _read_block(ptrLocation);
	
	//
	// looking up the sourounding free chunks
	//
	if(_FREE_ < ptrLocation) {
		uint64_t prevFree = _FREE_;
		uint64_t nextFree;
		bool lastBlock = false;

		while(true) {
			nextFree = _read_block(prevFree);

			if(nextFree == 0) {
				lastBlock = true;
				break;
			}

			if(ptrLocation > prevFree && ptrLocation < nextFree) {
				break;
			}

			prevFree = nextFree;
		}

		//
		// determining wether the chunk needs to be merged with neighbouring free chunks or not
		//
		bool mergePrev = false;
		bool mergeNext = false;

		if(prevFree + (2 * BLOCK_SIZE) + _read_block(prevFree + BLOCK_SIZE) == ptrLocation) {
			mergePrev = true;
		}

		if(ptrLocation + (2 * BLOCK_SIZE) + _read_block(ptrLocation + BLOCK_SIZE) == nextFree) {
			mergeNext = true;
		}


		uint64_t newChunkAddr = ptrLocation;
		uint64_t newChunkSize = _read_block(ptrLocation + BLOCK_SIZE);
		uint64_t nextChunkAddr = nextFree;

		//
		// merging the chunk with its neighbours & initializing it
		//
		if(mergePrev && mergeNext) {
			newChunkSize = _read_block(prevFree + BLOCK_SIZE) + _read_block(nextFree + BLOCK_SIZE) + (4 * BLOCK_SIZE) + _read_block(ptrLocation + BLOCK_SIZE);
			newChunkAddr = prevFree;
			nextChunkAddr = _read_block(nextFree);
		} else if(mergePrev) {
			newChunkSize = _read_block(prevFree + BLOCK_SIZE) + (2 * BLOCK_SIZE) + _read_block(ptrLocation + BLOCK_SIZE);
			newChunkAddr = prevFree;
		} else if(mergeNext) {
			newChunkSize = _read_block(nextFree + BLOCK_SIZE) + (2 * BLOCK_SIZE) + _read_block(ptrLocation + BLOCK_SIZE);
			nextChunkAddr = _read_block(nextFree);

			_write_block(newChunkAddr, prevFree);
		} else {
			_write_block(newChunkAddr, prevFree);
		}

		_write_block(newChunkSize, newChunkAddr + BLOCK_SIZE);
		_write_block(nextChunkAddr, newChunkAddr);

	
	//
	// handling edge cases
	//
	
	} else {
		if(_FREE_ == ptrLocation + (2 * BLOCK_SIZE) + _read_block(ptrLocation + BLOCK_SIZE)) {
			uint64_t newChunkSize = (2 * BLOCK_SIZE) + _read_block(ptrLocation + BLOCK_SIZE) + _read_block(_FREE_ + BLOCK_SIZE);
			uint64_t nextChunkAddr = _read_block(_FREE_);

			_write_block(newChunkSize, ptrLocation + BLOCK_SIZE);
			_write_block(nextChunkAddr, ptrLocation);
		} else {
			_write_block(_FREE_, ptrLocation);
		}

		_FREE_ = ptrLocation;
	}

	if(ptrLocation == _USED_) {
		if(_value_0_ptrLocation == 0) {
			_USED_ = MEMORY_SIZE;
		} else {
			_USED_ = _value_0_ptrLocation;
		}
	} else {
		_write_block(_value_0_ptrLocation, prevUsedChunk);
	}

	// returning true after the chunk has been freed
	return true;
}

void dumpMem(uint64_t from, uint64_t to) {
	initMem();
	
	if(from < 0 || to >= MEMORY_SIZE || to < from) {
		return;
	}
	
	int startPadding = from % BLOCK_SIZE;
	int len = to - from;
	int addrLen = _num_digits(to) + 1;

	uint64_t freeLabel = _FREE_;
	uint64_t usedLabel = _USED_;
	uint64_t freeSize = _read_block(freeLabel + BLOCK_SIZE);
	uint64_t usedSize = _read_block(usedLabel + BLOCK_SIZE);
	bool printFreeLabel = false;
	bool printUsedLabel = false;
    
		int c_addr = (from / BLOCK_SIZE) * BLOCK_SIZE;
		for(uint64_t i = from; i <= to; i++) {
			if(i % BLOCK_SIZE == 0 || i == from) {

				if(i != from) {
					std::cout << "  ";
					for(int h = 0; h < BLOCK_SIZE; h++) {
						uint8_t c = DATA[i - (BLOCK_SIZE - (h))];
						if(c >= 32 && c <= 126) {
							std::cout << (char)c;
						} else {
							std::cout << ".";
						}
					}
				}

				if(printFreeLabel) {
					std::cout << "    [FREE -> " << std::to_string(freeSize) << " Bytes]";
				}
				if(printUsedLabel) {
					std::cout << "    [USED -> " << std::to_string(usedSize) << " Bytes]";
				}
				printFreeLabel = false;
				printUsedLabel = false;

				std::cout << std::endl << std::setw(addrLen) << c_addr;
				std::cout << " | ";
				c_addr += BLOCK_SIZE;
			}
			if(i == from && startPadding != 0) {
				for(int h = 0; h < startPadding; h++) {
					std::cout << "         ";
				}
			}
			std::cout << _int_bin(DATA[i]) << " ";
			
			if(i == freeLabel) {
				freeSize = _read_block(freeLabel + BLOCK_SIZE);
				freeLabel = _read_block(freeLabel);
				printFreeLabel = true;
			} else if(i == usedLabel) {
				usedSize = _read_block(usedLabel + BLOCK_SIZE);
				usedLabel = _read_block(usedLabel);
				printUsedLabel = true;
			}
		}
		std::cout << std::endl;
}

uint64_t _combine_2(uint8_t a, uint8_t b) {
	uint64_t combined = ((uint64_t)a << 8) | ((uint64_t)b);
	return combined;
}

uint64_t _combine_4(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
	uint64_t combined = ((uint64_t)a << 24) | ((uint64_t)b << 16) | ((uint64_t)c << 8) | ((uint64_t)d);
	return combined;
}

uint64_t _combine_8(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f, uint8_t g, uint8_t h) {
	uint64_t combined = ((uint64_t)a << 56) | ((uint64_t)b << 48) | ((uint64_t)c << 40) | ((uint64_t)d << 32) | ((uint64_t)e << 24) | ((uint64_t)f << 16) | ((uint64_t)g << 8) | ((uint64_t)h);
	return combined;
}

std::string _int_bin(uint8_t num) {
	std::string binaryString = "";
	uint8_t div = std::pow(2, 7);
	for(uint8_t i = 0; i < 8; i++) {
		binaryString += std::to_string(num / div);
		num = num % div;
		div = div / 2;
	}

	return binaryString;
}

int _num_digits(uint64_t num) {
	uint64_t div = 10;
	int counter = 1;
	while(num >= div) {
		counter++;
		div *= 10;
	}

	return counter;
}
void _write_block(uint64_t value, uint64_t location) {
	#if BLOCK_SIZE == 8
		DATA[location]     = value >> 56;
		DATA[location + 1] = value >> 48;
		DATA[location + 2] = value >> 40;
		DATA[location + 3] = value >> 32;
		DATA[location + 4] = value >> 24;
		DATA[location + 5] = value >> 16;
		DATA[location + 6] = value >> 8;
		DATA[location + 7] = value;
	#elif BLOCK_SIZE == 2
		DATA[location]     = value >> 8;
		DATA[location + 1] = value;
	#elif BLOCK_SIZE == 1
		DATA[location]     = value;
	#else
		DATA[location]     = value >> 24;
		DATA[location + 1] = value >> 16;
		DATA[location + 2] = value >> 8;
		DATA[location + 3] = value;
	#endif
}

uint64_t _read_block(uint64_t location) {
	#if BLOCK_SIZE == 8
		return _combine_8(DATA[location], DATA[location + 1], DATA[location + 2], DATA[location + 3], DATA[location + 4], DATA[location + 5], DATA[location + 6], DATA[location + 7]);
	#elif BLOCK_SIZE == 2
		return _combine_2(DATA[location], DATA[location + 1]);
	#elif BLOCK_SIZE == 1
		return DATA[location];
	#else
		return _combine_4(DATA[location], DATA[location + 1], DATA[location + 2], DATA[location + 3]);
	#endif
}