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
		INX_LDA_IM = 0xA9,
		INX_LDA_ZP = 0xA5,
		INX_ASL_ACC = 0x0A,
		INX_LDX_IM = 0xA2,
		INX_STA_ZP = 0x85,
		INX_JSR = 0x20;
		





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
		
		pc = 0xFF99;
		sp = 0x0100;


		//initialize general purpose registers
		A = 0;
		X = 0;
		Y = 0;

		memory.initialize();
	}

	WORD fetchWord(uint& cycle, Memory _memory)
	{
		WORD chunkword = _memory[pc];
		pc++;
		chunkword |= (_memory[pc] <<8);
		pc++;
		cycle-=2;
		return chunkword;
	}
	void writeWord(uint& cycle, WORD address, Memory& _memory, WORD chunk)
	{
		_memory[address] = chunk & 0xFF;
		cycle--;
		_memory[address+1] = chunk >> 8;
		cycle--;
		return;
	}
	BYTE fetch(uint& cycle, Memory _memory)
	{
		BYTE chunk = _memory[pc];
		pc++;
		cycle--;
		return chunk;

	}
	BYTE read(uint& cycle,WORD address, Memory _memory)
	{
		BYTE chunk = _memory[address];
		cycle--;
		return chunk;
	}
	void write(uint& cycle, WORD address, Memory& _memory, BYTE chunk)
	{
		_memory[address] = chunk;
		cycle--;

	}
	void execute(uint cycle,Memory &mem)
	{
		while (cycle > 0)
		{
			BYTE inx=fetch(cycle, mem);
			//decoding 
			switch (inx)
			{
			case INX_LDA_IM:
				{
				BYTE operand=fetch(cycle, mem);
				A = operand;
				flgzero = (A == 0);
				flgnegative = (A & 0b10000000) > 0;

					break;
				}
			case INX_LDA_ZP:
			{
				BYTE zeropage_address = fetch(cycle, mem);
				BYTE data = read(cycle,zeropage_address, mem);
				flgzero = (A == 0);
				flgnegative = (A & 0b10000000) > 0;
			
				break;
			}
			case INX_LDX_IM:
			{
				BYTE data = fetch(cycle, mem);
				X = data;
				flgzero = (X == 0);
				flgnegative = (X & 0b10000000) > 0;
				//cycle--;
				break;
				 //cycle--;
			}
			case INX_STA_ZP:
			{
				BYTE zeropage_addressW = fetch(cycle, mem);
				write(cycle, zeropage_addressW, mem, A);
				break;
				//cycle--;

			}

			case INX_ASL_ACC:
			{

				A = A << 1;

				flgzero = (A == 0);
				flgnegative = (A & 0b10000000) > 0;

				break;
			}
			case INX_JSR:
			{
				WORD jumpAddress = fetchWord(cycle, mem);
				writeWord(cycle, sp, mem, pc - 1);
				pc = jumpAddress;
				cycle--;

				break;

			}
			default:
				{
					printf("\ndecoder switched to default, no instruction decoded");
					break;
				}
			}

		


		}

		return;
	}


}_cpu;




int main()
{
	printf("\n6052 cpu, %d byte(bytesize) %d byte (wordsize)\nMemory:%d KB \n\n", sizeof(BYTE), sizeof(WORD),Memory::MEM_MAX_CAP/1024);
	//printf("%d", sizeof(0x0 | _cpu.intrrupt_disable));

	//printf("%d", _cpu.intrrupt_disable);

	 Memory memory;
	_cpu.hard_reset(memory);

	//hardwire programm into memory.
	WORD address = 0x6161;
	memory[address++] = CPU::INX_LDA_IM;  //load
	memory[address++] = (BYTE)10;			
	memory[address++] = CPU::INX_ASL_ACC; //left shift		flgintrrupt_disable	1 '\x1'	unsigned char

	memory[address++] = CPU::INX_STA_ZP; //store
	memory[address++] = 0x002F;
	//segment 1
	address = 0xFF99;
	memory[address++] = CPU::INX_JSR; // function call to 0x6161
	memory[address++] = 0x61;
	memory[address++] = 0x61;
	_cpu.execute(12, memory);

	printf("\n %d", memory[0x002F]);

	return 0;

}