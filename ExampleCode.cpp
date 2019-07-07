/*
 * 
 *
 * @author Peter Voigt
 */
#include <iostream>
#include <string>
#include "MemoryManagement.h"

using namespace std;

int main() {
	dumpMem(0, 99);
	cout << endl;

	uint8_t * testPtr_1 = myAlloc(12);
	testPtr_1[0] = 72;
	testPtr_1[1] = 97;
	testPtr_1[2] = 108;
	testPtr_1[3] = 108;
	testPtr_1[4] = 111;
	testPtr_1[5] = 32;
	testPtr_1[6] = 87;
	testPtr_1[7] = 101;
	testPtr_1[8] = 108;
	testPtr_1[9] = 116;
	testPtr_1[10] = 33;
	testPtr_1[11] = 33;

	cout << ">> ALLOCATE 12 BYTES" << endl;

	uint8_t * testPtr_2 = myAlloc(16);

	cout << ">> ALLOCATE 16 BYTES" << endl;

	uint8_t * testPtr_3 = myAlloc(12);
	testPtr_3[0] = 72;
	testPtr_3[1] = 97;
	testPtr_3[2] = 108;
	testPtr_3[3] = 108;
	testPtr_3[4] = 111;
	testPtr_3[5] = 32;
	testPtr_3[6] = 87;
	testPtr_3[7] = 101;
	testPtr_3[8] = 108;
	testPtr_3[9] = 116;
	testPtr_3[10] = 33;
	testPtr_3[11] = 33;

	cout << ">> ALLOCATE 12 BYTES" << endl;

	dumpMem(0, 99);
	cout << endl;

	myFree(testPtr_1);
	cout << ">> FREE 12 BYTES" << endl;
	myFree(testPtr_3);
	cout << ">> FREE 12 BYTES" << endl;

	dumpMem(0, 99);
	cout << endl;

	myFree(testPtr_2);
	cout << ">> FREE 16 BYTES" << endl;

	dumpMem(0, 99);
	cout << endl;

	testPtr_2 = myAlloc(28);
	cout << ">> ALLOCATE 28 BYTES" << endl;

	dumpMem(0, 99);
	cout << endl;

	myFree(testPtr_2);
	cout << ">> FREE 28 BYTES" << endl;

	dumpMem(0, 99);

	return 0;
}