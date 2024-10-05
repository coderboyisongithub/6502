#include <stdio.h>

#include <iostream>


//http://retro.hansotten.nl/uploads/andrewjacobs/index.html
//https://www.c64-wiki.com/wiki/Reset_(Process)
typedef unsigned int uint;
typedef unsigned char BYTE;
typedef unsigned short WORD;
const unsigned int  clear = 0,set=1;


struct Memory
{

	static constexpr unsigned int MEM_MAX_CAP = 1024 * 64;
	BYTE mem[MEM_MAX_CAP];

	void initialize()
	{


		for (WORD block = 0; block < 0xFFFF; block++)
		{
			mem[block] = 0;
		}
	}

	BYTE operator[](WORD address) const
	{
		return mem[address];
	}
	BYTE& operator[](WORD address) 
	{
		return mem[address];
	}
	

};

struct CPU
{


	WORD pc; //program counter.
	BYTE sp; //stack pointer

	//general purpose 8bit-register.
	BYTE A, Y, X;

	//7 status flag.

	BYTE
		flgcarry : 1,
		flgzero:1,
		flgintrrupt_disable:1,
		flgdecimal_mode:1,
		flgbreak_cmd:1,
		flgoverflow:1,
		flgnegative:1;


	//instruction
	static constexpr BYTE 
		INX_LDA = 0xA9;



	/*
	
FCE2   A2 FF      LDX #$FF        ; 
FCE4   78         SEI             ; set interrupt disable 
FCE5   9A         TXS             ; transfer .X to stack
FCE6   D8         CLD             ; clear decimal flag  
FCE7   20 02 FD   JSR $FD02       ; check for cart  
FCEA   D0 03      BNE $FCEF       ; .Z=0? then no cart detected
FCEC   6C 00 80   JMP ($8000)     ; direct to cartridge cold start via vector
FCEF   8E 16 D0   STX $D016       ; sets bit 5 (MCM) off, bit 3 (38 cols) off
FCF2   20 A3 FD   JSR $FDA3       ; initialise I/O
FCF5   20 50 FD   JSR $FD50       ; initialise memory
FCF8   20 15 FD   JSR $FD15       ; set I/O vectors ($0314..$0333) to kernal defaults
FCFB   20 5B FF   JSR $FF5B       ; more initialising... mostly set system IRQ to correct value and start
FCFE   58         CLI             ; clear interrupt  
FCFF   6C 00 A0   JMP ($A000)     ; direct to BASIC cold start via vector
	
	*/
	void hard_reset(Memory &memory)
	{
		//reset flag
		flgcarry = clear;
		flgzero = clear;
		flgnegative = clear;
		flgoverflow = clear;
		flgbreak_cmd = clear;
		flgdecimal_mode = clear;
		flgintrrupt_disable = set;
		
		pc = 0xFFFC;
		sp = 0x0100;

		//initialize general purpose registers
		A = 0;
		X = 0;
		Y = 0;

		memory.initialize();
	}
	BYTE fetch(uint& cycle, Memory _memory)
	{
		BYTE chunk = _memory[pc];
		pc++;
		cycle--;
		return chunk;

	}
	void execute(uint cycle,Memory &mem)
	{
		while (cycle > 0)
		{
			BYTE inx=fetch(cycle, mem);
			//decoding 
			switch (inx)
			{
			case INX_LDA:
				{
				BYTE operand=fetch(cycle, mem);
				flgzero = (A == 0);
				flgnegative = (A & 0b10000000) > 0;

					break;
				}
			default:
				{
					printf("\ndecoder switch to dafault, no instruction decoded");
					break;
				}
			}

		


		}

		return;
	}


}_cpu;




int main()
{

	//printf("%d", sizeof(0x0 | _cpu.intrrupt_disable));

	//printf("%d", _cpu.intrrupt_disable);

	 Memory memory;
	_cpu.hard_reset(memory);

	//hardwire programm into memory.
	memory[0xFFFC] = CPU::INX_LDA;
	memory[0xFFFD] = (BYTE)10;


	printf("\n%d", memory.MEM_MAX_CAP);
	_cpu.execute(2, memory);

	return 0;

}