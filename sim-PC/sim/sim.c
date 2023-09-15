#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <ctype.h>

//Constant and gloabal paremeters.
#define Main_Memory_Max 512 // 2^9
#define Max_string_length 500 
#define Max_Label_Len 50
#define CLKS_IN_SECOND (8000000*10/2)


char memin_arr[Main_Memory_Max + 1][9]; // array to save memin lines/information.
int registers_arr[16]; //registers array
static int memin_arr_size; //a constent to save memin actual size
static int pc = 0; //an int variable to save pc position at every clock cycle.
static int instructions_count = 0;  //an int variable to save the number of instructions the progrem as made.
char IOregister[15][9] = { "00000000","00000000","00000000","00000000","00000000","00000000","00000000","00000000","00000000",
"00000000","00000000","00000000","00000000","00000000","00000000" }; //array to represent registers.


int ready_to_irq = 1;

// Command structure for the CPU
typedef struct {
	char inst_line[9]; // Instruction line as a string
	int opcode;
	int rd;
	int rs;
	int rt;
	int imm;
} command;

//start of function decleration
unsigned long long clk64;

void removeExtraWhitespaces(char* str);
void copy_memin_to_array(FILE* memin);
void RegOut(FILE* pregout);
void MemOut(FILE* pmemout);
int HexCharToInt(char h);
void Int_to_Hex(int dec_num, char hex_num[9]);
int Hex_to_2sComp(const char* h);
void WriteTrace(const command* com, FILE* ptrace);
void build_command(char* command_line, command* com);
void clk_counter();
void perform(command* com, FILE* ptrace, FILE* pcycles, FILE* pmemout, FILE* pregout);
//end of function declaration

int main(int argc, char* argv[]) {
	FILE* memin = fopen(argv[1], "r"), * memout = fopen(argv[2], "w"), * regout = fopen(argv[3], "w"),
		* trace = fopen(argv[4], "w"), * cycles = fopen(argv[5], "w");
	// Check if any of the files failed to open
	if (memin == NULL || memout == NULL || regout == NULL || trace == NULL || cycles == NULL)
	{
		printf("One of the files could not be opened \n ");
		exit(1);
	}

	copy_memin_to_array(memin); //copy memin to memin_arr
	// Close the memin file after copying
	fclose(memin);

	int program_checker = 0; //equal 0 if we run fib, equal 1 if we run timer.
	Int_to_Hex(1023, IOregister[13]); //initialize timermax to 50
	Int_to_Hex(0, IOregister[12]); //initislize timercurrent to 0
	int io12val;
	while (1) {
		// Run the simulation until the halt instruction is encountered
		if ((Hex_to_2sComp(IOregister[12]) == Hex_to_2sComp(IOregister[13]))) //irq0 occurred
		{
			Int_to_Hex(1, IOregister[3]); //set irq0status to 1
			Int_to_Hex(0, IOregister[12]); //initialize timercurrent to 0
			if (irq_status_check() == 1 && ready_to_irq) //check if status is 1, enable is 1 and ready to irq
			{
				ready_to_irq = 0; //processor is in irq routine
				Int_to_Hex(pc, IOregister[7]); //saving the cuurent PC in reg 7
				pc = Hex_to_2sComp(IOregister[6]); //jumping to the irq routine
			}
		}
		// Write register values to pregout
		RegOut(regout);
		// Fetch the instruction from memin_arr at the current program counter (pc)
		char command_line[9];
		//removeExtraWhitespaces(memin_arr[pc]); //remove whitespaces if thereany from memin_arr
		strcpy(command_line, memin_arr[pc]);

		// Build the command structure from the command line
		command com;
		build_command(command_line, &com);
		// Write trace information to ptrace
		WriteTrace(&com, trace);

		// Perform the operation specified by the command
		perform(&com, trace, cycles, memout, regout);

		io12val = strtol(IOregister[12], NULL, 16);
		Int_to_Hex(io12val + 1, IOregister[12]);
		// Check if the halt instruction is encountered
		if (com.opcode == 19) {
			break;
		}
	}

	// Write memory values to pmemout
	MemOut(memout);

	// Close all the open files
	fclose(regout);
	fclose(memout);
	fclose(trace);
}

void build_command(char* command_line, command* com) {
	// Copy the instruction line to the command structure
	strcpy(com->inst_line, command_line);

	// Extract opcode, rd, rs, rt, and imm from the command line
	com->opcode = strtol(command_line, NULL, 16) >> 24 & 0xFF;
	com->rd = strtol(command_line, NULL, 16) >> 20 & 0xF;
	com->rs = strtol(command_line, NULL, 16) >> 16 & 0xF;
	com->rt = strtol(command_line, NULL, 16) >> 12 & 0xF;
	com->imm = strtol(command_line, NULL, 16) & 0xFFF;

	// Check if the immediate value is negative and adjust it accordingly
	if (com->imm & 0x800) {
		// The sign bit is set, indicating a negative value
		com->imm |= 0xFFFFF000; // Set the upper bits to convert to a negative value
	}
}

//copy each line in memin.txt into memin_arr array.
void copy_memin_to_array(FILE* memin)
{
	char line[10];  // the size of 10 to accommodate for the null terminator and '\n' at the end of the memin file.
	int i = 0;

	// Read lines from the memin file and copy them to memin_arr
	while (fgets(line, sizeof(line), memin) != NULL && i < Main_Memory_Max)
	{
		line[strcspn(line, "\n")] = '\0';  // Remove newline character
		strcpy(memin_arr[i], line);  // Copy the modified line to memin_arr[i]
		i++;
	}

	memin_arr_size = i;

	// Fill remaining memory slots with zeros
	while (i < Main_Memory_Max)
	{
		strcpy(memin_arr[i], "00000000");
		i++;
	}
}


//write to regout.txt
void RegOut(FILE* pregout)
{
	int i;
	for (i = 2; i < 16; i++)  // Iterate over registers_arr and print its values to pregout
	{
		fprintf(pregout, "%08X\n", registers_arr[i]);
	}
}

//write to memout.txt
void MemOut(FILE* pmemout)
{
	int i = 0;
	for (i = 0; i <= Main_Memory_Max; i++) //go over memin_arr and write it to memout
		fprintf(pmemout, "%s\n", memin_arr[i]);
}

//convert char to int
int HexCharToInt(char h) {
	if (h >= '0' && h <= '9') {
		return h - '0';
	}
	else if (h >= 'A' && h <= 'F') {
		return h - 'A' + 10;
	}
	else if (h >= 'a' && h <= 'f') {
		return h - 'a' + 10;
	}
	return -1;  // Invalid input
}

int HexCharArrToInt(char* h) {
	int len = strlen(h);
	int res = 0;
	// Iterate over each character of the input string from right to left
	for (int i = 0; i < len; i++) {
		// Convert the hexadecimal character to its corresponding integer value
		int value = HexCharToInt(h[len - 1 - i]);

		// Multiply the value by 2 raised to the appropriate power of 4
		res += value * (1 << (4 * i));
	}
}

void Int_to_Hex(int dec_num, char hex_num[9]) {
	if (dec_num < 0)
		dec_num = dec_num + 4294967296; // Add 2^32 to negative dec_num

	sprintf(hex_num, "%08X", dec_num); // Format dec_num as an 8-digit hexadecimal string
}


//checks the innterupts status and enable values, returns 1 if an irq occured (both status and enable is 1)
int irq_status_check()
{
	// Check if any of the irq interrupt conditions are satisfied and ready to handle interrupts
	if ((Hex_to_2sComp(IOregister[0]) && Hex_to_2sComp(IOregister[3])))
		return 1;
	else
		return 0;
}

// Function to convert a hexadecimal string to a signed integer in 2's complement
int Hex_to_2sComp(const char* h) {
	int len = strlen(h);
	int res = 0;

	// Iterate over each character of the input string from right to left
	for (int i = 0; i < len; i++) {
		// Convert the hexadecimal character to its corresponding integer value
		int value = HexCharToInt(h[len - 1 - i]);

		// Multiply the value by 2 raised to the appropriate power of 4
		res += value * (1 << (4 * i));
	}

	// Perform sign extension if the string has less than 8 characters and the most significant bit is set
	if (len < 8 && (res & (1 << (len * 4 - 1)))) {
		// Perform sign extension by OR-ing the value with a negative number formed by shifting 1 to the left
		// by len * 4 bits, effectively setting all the higher bits to 1
		res |= -1 * (1 << (len * 4));
	}

	return res;
}

// Function to write trace information to trace.txt
void WriteTrace(const command* com, FILE* ptrace)
{
	char pch[9] = { 0 }; // Program counter in hexadecimal
	char inst[9] = { 0 }; // Instruction in hexadecimal
	char registers[16][9] = { 0 }; // Register values in hexadecimal

	// Convert program counter to hexadecimal
	Int_to_Hex(pc, pch);
	// Copy instruction to inst
	strcpy(inst, com->inst_line);
	inst[8] = '\0';
	// Iterate over registers 0 to 15
	for (int i = 0; i < 16; i++) {
		// Convert register value to hexadecimal
		Int_to_Hex(registers_arr[i], registers[i]);
	}

	// Write trace information to the file
	fprintf(ptrace, "%s %s", pch, inst);
	for (int i = 0; i < 16; i++) {
		fprintf(ptrace, " %s", registers[i]);
	}
	fprintf(ptrace, "\n");
}

// Function to increment the clock counter every cycle
void clk_counter()
{
	// Get the current clock value from IOregister[8]
	int currentClock = Hex_to_2sComp(IOregister[8]);

	// Increment the clock value by 1
	int newClock = (currentClock + 1) % 4294967296;  // Modulus operator to handle overflow

	// Convert the new clock value back to hexadecimal and store it in IOregister[8]
	Int_to_Hex(newClock, IOregister[8]);
}

// Function to perform the command, write to trace, manage pc and cycles, and halt
void perform(command* com, FILE* ptrace, FILE* pcycles, FILE* pmemout, FILE* pregout)
{
	registers_arr[1] = com->imm;
	char hex_num_temp[9] = "00000000";

	if (com->opcode == 0 || com->opcode == 1 || com->opcode == 2 || com->opcode == 3 || com->opcode == 4) {
		// Arithmetic operations: add, sub, and, or, sll
		if (com->rd != 0 && com->rd != 1) {
			if (com->rs == 1)
				registers_arr[com->rs] = com->imm;
			if (com->rt == 1)
				registers_arr[com->rt] = com->imm;

			switch (com->opcode) {
			case 0: // add
				registers_arr[com->rd] = registers_arr[com->rs] + registers_arr[com->rt];
				break;
			case 1: // sub
				registers_arr[com->rd] = registers_arr[com->rs] - registers_arr[com->rt];
				break;
			case 2: // and
				registers_arr[com->rd] = registers_arr[com->rs] & registers_arr[com->rt];
				break;
			case 3: // or
				registers_arr[com->rd] = registers_arr[com->rs] | registers_arr[com->rt];
				break;
			case 4: // sll
				registers_arr[com->rd] = registers_arr[com->rs] << registers_arr[com->rt];
				break;
			}
		}
		pc++;
	}
	else if (com->opcode == 5 || com->opcode == 6) {
		// Shift operations: sra, srl
		if (com->rd != 0 && com->rd != 1) {
			if (com->rs == 1)
				registers_arr[com->rs] = com->imm;
			if (com->rt == 1)
				registers_arr[com->rt] = com->imm;

			switch (com->opcode) {
			case 5: // sra
				registers_arr[com->rd] = registers_arr[com->rs] >> registers_arr[com->rt];
				break;
			case 6: // srl
				registers_arr[com->rd] = (registers_arr[com->rs] >> registers_arr[com->rt]) & 0x7fffffff;
				break;
			}
		}
		pc++;
	}
	else if (com->opcode >= 7 && com->opcode <= 12) {
		// Branch operations: beq, bne, blt, bgt, ble, bge
		int condition = 0;

		switch (com->opcode) {
		case 7: // beq
			condition = registers_arr[com->rs] == registers_arr[com->rt];
			break;
		case 8: // bne
			condition = registers_arr[com->rs] != registers_arr[com->rt];
			break;
		case 9: // blt
			condition = registers_arr[com->rs] < registers_arr[com->rt];
			break;
		case 10: // bgt
			condition = registers_arr[com->rs] > registers_arr[com->rt];
			break;
		case 11: // ble
			condition = registers_arr[com->rs] <= registers_arr[com->rt];
			break;
		case 12: // bge
			condition = registers_arr[com->rs] >= registers_arr[com->rt];
			break;
		}

		if (condition) {
			if (com->rd == 1)
				registers_arr[com->rd] = com->imm;
			pc = registers_arr[com->rd];
		}
		else {
			pc++;
		}
	}
	else if (com->opcode == 13) {
		// Jump and link: jal
		registers_arr[15] = pc + 1;
		if (com->rd == 1)
			registers_arr[com->rd] = com->imm;
		pc = registers_arr[com->rd];
	}
	else if (com->opcode == 14) {
		// Load word: lw
		if (com->rd != 0 && com->rd != 1) {
			if (com->rs == 1)
				registers_arr[com->rs] = com->imm;
			if (com->rt == 1)
				registers_arr[com->rt] = com->imm;

			registers_arr[com->rd] = Hex_to_2sComp(memin_arr[registers_arr[com->rs] + registers_arr[com->rt]]);
		}
		pc++;
	}
	else if (com->opcode == 15) {
		// Store word: sw
		if (com->rd == 1)
			registers_arr[com->rd] = com->imm;
		if (com->rt == 1)
			registers_arr[com->rt] = com->imm;
		if (com->rs == 1)
			registers_arr[com->rs] = com->imm;

		Int_to_Hex(registers_arr[com->rd], hex_num_temp);

		int memory_address = registers_arr[com->rs] + registers_arr[com->rt];
		strcpy(memin_arr[memory_address], hex_num_temp);

		pc++;
	}
	else if (com->opcode == 16) {
		//reti
		pc = Hex_to_2sComp(IOregister[7]);
		ready_to_irq = 1; //the irq is done
	}

	else if (com->opcode == 17) {
		//in
		if (com->rs == 1)
			registers_arr[com->rs] = com->imm;
		if (com->rt == 1)
			registers_arr[com->rt] = com->imm;
		registers_arr[com->rd] = Hex_to_2sComp(IOregister[registers_arr[com->rs] + registers_arr[com->rt]]);
		pc++;
	}

	else if (com->opcode == 18) {
		//out
		if (com->rd == 1)
			registers_arr[com->rd] = com->imm;
		if (com->rs == 1)
			registers_arr[com->rs] = com->imm;
		if (com->rt == 1)
			registers_arr[com->rt] = com->imm;
		Int_to_Hex(registers_arr[com->rd], IOregister[registers_arr[com->rs] + registers_arr[com->rt]]);
		pc++;
	}
	else if (com->opcode == 19) {
		// Halt: write files and close them
		clk_counter(); // Update the clock after executing the command
		instructions_count++;

		fprintf(pcycles, "%d", instructions_count);
		RegOut(pregout);
		MemOut(pmemout);

		fclose(pmemout);
		fclose(pregout);
		fclose(ptrace);
		fclose(pcycles);

		exit(0); // Terminate the program
	}

	clk_counter(); // Update the clock after executing the command
	instructions_count++; // Increment the number of executed instructions
}

