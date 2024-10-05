#include <stdio.h>

#include <iostream>


//http://retro.hansotten.nl/uploads/andrewjacobs/index.html
//https://www.c64-wiki.com/wiki/Reset_(Process)

typedef unsigned char BYTE;
typedef unsigned short WORD;

WORD memory[1024] = { 0 };


struct CPU
{


	WORD pc; //program counter.
	BYTE sp; //stack pointer

	//general purpose 8bit-register.
	BYTE A, Y, X;

	//status flag.

	BYTE
		flgcarry : 1,
		flgzero:1,
		flgintrrupt_disable:1,
		flgdecimal_mode:1,
		flgbreak_cmd:1,
		flgoverflow:1,
		flgnegative:1;



	void hard_reset()
	{
		pc = 0xFFFC;
		sp = 0x0000;

	}


}_cpu;


int main()
{

	//printf("%d", sizeof(0x0 | _cpu.intrrupt_disable));

	//printf("%d", _cpu.intrrupt_disable);

	_cpu.hard_reset();


	return 0;

}