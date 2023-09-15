#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Defenitions:
#define MAX_LINE_LENGTH 500
#define MAX_LABEL_LENGTH 50
#define MEMORY_SIZE 512

//a structure for a label: has name and location fields.
typedef struct {
	char label_name[MAX_LABEL_LENGTH];
	int location;
} label_struct;

//a structure for a line 
typedef char line[MAX_LINE_LENGTH];

//Data structure of instruction, seperated by fields.
typedef struct {
	char* label;
	char* opcode;
	char* rd;
	char* rs;
	char* rt;
	char* imm;
} line_fields;

//Global variables:
label_struct labels[MEMORY_SIZE]; //global array to store the labels and their location.
int label_counter = 0; //counter of the labels
int memory[MEMORY_SIZE]; //global array to store the memory
int program_length = 0; //a counter in order to get the program length

//A "dictionary", gets an opcode and returns the number that represent it.
int opcode_converter(char* opcode) {
	char temp[MAX_LINE_LENGTH];
	strcpy(temp, opcode); //copies the contents of opcode string into temp array.
	_strlwr(temp); //temp is now in lower case letters
	if (!strcmp(temp, "add"))
		return 0;
	else if (!strcmp(temp, "sub"))
		return 1;
	else if (!strcmp(temp, "and"))
		return 2;
	else if (!strcmp(temp, "or"))
		return 3;
	else if (!strcmp(temp, "sll"))
		return 4;
	else if (!strcmp(temp, "sra"))
		return 5;
	else if (!strcmp(temp, "srl"))
		return 6;
	else if (!strcmp(temp, "beq"))
		return 7;
	else if (!strcmp(temp, "bne"))
		return 8;
	else if (!strcmp(temp, "blt"))
		return 9;
	else if (!strcmp(temp, "bgt"))
		return 10;
	else if (!strcmp(temp, "ble"))
		return 11;
	else if (!strcmp(temp, "bge"))
		return 12;
	else if (!strcmp(temp, "jal"))
		return 13;
	else if (!strcmp(temp, "lw"))
		return 14;
	else if (!strcmp(temp, "sw"))
		return 15;
	else if (!strcmp(temp, "reti"))
		return 16;
	else if (!strcmp(temp, "in"))
		return 17;
	else if (!strcmp(temp, "out"))
		return 18;
	else if (!strcmp(temp, "halt"))
		return 19;
	return -1;
}

//A "dictionary", gets a register and returns the number that represent it.
int register_convertor(char* register_name) {
	char temp[MAX_LINE_LENGTH];
	strcpy(temp, register_name); //copies the contents of the register string into temp.
	_strlwr(temp); //temp is now in lower case letters
	if (register_name = NULL)
		return 0;
	else if (!strcmp(temp, "$zero"))
		return 0;
	else if (!strcmp(temp, "$imm"))
		return 1;
	else if (!strcmp(temp, "$v0"))
		return 2;
	else if (!strcmp(temp, "$a0"))
		return 3;
	else if (!strcmp(temp, "$a1"))
		return 4;
	else if (!strcmp(temp, "$t0"))
		return 5;
	else if (!strcmp(temp, "$t1"))
		return 6;
	else if (!strcmp(temp, "$t2"))
		return 7;
	else if (!strcmp(temp, "$t3"))
		return 8;
	else if (!strcmp(temp, "$s0"))
		return 9;
	else if (!strcmp(temp, "$s1"))
		return 10;
	else if (!strcmp(temp, "$s2"))
		return 11;
	else if (!strcmp(temp, "$gp"))
		return 12;
	else if (!strcmp(temp, "$sp"))
		return 13;
	else if (!strcmp(temp, "$fp"))
		return 14;
	else if (!strcmp(temp, "$ra"))
		return 15;
	else
		return atoi(temp);
}

//In order to find the line type. there are 5 types:
//1=empty line/just a note   2=just a label   3=full line with label   4=word   5=regular line- no label
int get_line_type(line_fields line)
{
	if ((line.label == NULL && line.opcode == NULL)) //empy line/a note only
		return 1;
	if (line.label != NULL && line.opcode == NULL) //a label only
		return 2;
	if (line.label != NULL && line.opcode != NULL) //a full line, with label, no word
		return 3;
	if (strcmp(line.opcode, ".word") == 0) //word 
		return 4;
	if (line.label == NULL && line.opcode != NULL && (strcmp(line.opcode, ".word") != 0)) //a regular line- no label
		return 5;
}

//according to the line type, converting the line into line_fields type.
line_fields convert_line(char* line) {
	line_fields result = { NULL };
	if (line[0] == '#')
		return result;
	else
		line = strtok(line, "#"); //remove comments

	// Check if the line is empty
	if (line == NULL || line[0] == '\0')
		return result;

	// Remove leading and trailing whitespace
	char* trimmed_line = strtok(line, " \t\n");
	if (trimmed_line == NULL)
		return result;


	// Check if the line starts with a label
	char* colon = strchr(trimmed_line, ':');
	if (colon != NULL) {
		// Extract the label
		*colon = '\0'; // Replace the colon with null terminator
		result.label = trimmed_line;

		// Move to the next part of the line
		trimmed_line = strtok(NULL, " \t\n");
		if (trimmed_line == NULL)
			return result;
	}

	// Check if the line is a ".word" type
	if (strcmp(trimmed_line, ".word") == 0) {
		// Store the first value as opcode and the rest as rd
		result.opcode = trimmed_line;
		result.rd = strtok(NULL, " \t\n");
		result.rt = strtok(NULL, " \t\n");
		return result;
	}

	// Extract opcode, rd, rs, rt, imm (if present)
	result.opcode = trimmed_line;
	result.rd = strtok(NULL, " ,\t\n");
	result.rs = strtok(NULL, " ,\t\n");
	result.rt = strtok(NULL, " ,\t\n");
	result.imm = strtok(NULL, " ,\t\n");
	return result;
}

//In the first pass we find the labels and add their name and memory location into labels[] array.
void first_pass(FILE* input_file)
{
	int curr_memory = 0; //current place in memory, initialized to 0.
	line temp_line = { 0 };
	while (fgets(temp_line, MAX_LINE_LENGTH, input_file) != NULL) //reading the lines from the input_file one by one
	{
		line_fields input_line_fields = convert_line(temp_line); //convert each line to line_fields type
		int line_type = get_line_type(input_line_fields);
		if (line_type == 2) //a label only
		{
			strcpy(labels[label_counter].label_name, input_line_fields.label); //storing the label's name
			labels[label_counter].location = curr_memory; //storing the label's location
			label_counter++;
		}
		else if (line_type == 3) //regular line with a label
		{
			strcpy(labels[label_counter].label_name, input_line_fields.label); //storing the label's name
			labels[label_counter].location = curr_memory; //storing the label's location
			label_counter++;
			curr_memory++;
		}
		else if (line_type != 1 && line_type != 4) //in any case that the line is not an empty line and not a .word
			curr_memory++;
	}
}

//In the second pass we encode evey command to the SIMP format and store in memory[] array.
//In addition- in this function we count the program's length.
void second_pass(FILE* input_file)
{
	fseek(input_file, 0, SEEK_SET); //moves the file position indicator to the start of the file, allowing subsequent read or write operations to begin from the beginning of the file.
	int curr_memory = 0; //current place in memory, initialized to 0.
	line temp_line = { 0 };
	int max_address = 0; //maximum adress in memory, initialized to 0.
	while (fgets(temp_line, MAX_LINE_LENGTH, input_file) != NULL) //running the lines in the code
	{
		//printf("%s\n", temp_line);
		line_fields input_line_fields = convert_line(temp_line); //convert each line to line_fields type
		int line_type = get_line_type(input_line_fields);
		if (line_type == 4) //if the line is a .word line
		{
			int address;
			int data;
			//cheack if the address is an hexadecimal number
			if (input_line_fields.rd[0] == '0' && (input_line_fields.rd[1] == 'x' || input_line_fields.rd[1] == 'X'))
				address = HexToInt2sComp(input_line_fields.rd);
			else
				address = atoi(input_line_fields.rd);

			//cheack if the data is an hexadecimal number
			if (input_line_fields.rt[0] == '0' && (input_line_fields.rt[1] == 'x' || input_line_fields.rt[1] == 'X'))
				data = HexToInt2sComp(input_line_fields.rt);
			else
				data = atoi(input_line_fields.rt);

			memory[address] = data; //storing the data in the right place in the memory 
			if (address >= max_address)
				max_address = address;
		}
		else if (line_type == 3 || line_type == 5) //if the line is a regular line with or without a label
		{
			int opcode = opcode_converter(input_line_fields.opcode);
			int rd = register_convertor(input_line_fields.rd);
			int rs = register_convertor(input_line_fields.rs);
			int rt = register_convertor(input_line_fields.rt);
			int imm = immediate_converter(input_line_fields.imm);
			int mem_line;

			//enconding the line's fields (opcode, rd, rs, rt, imm) to a memory line in the instruction format (as we got in the project instructions)
			if (imm >= 0) {
				mem_line = (opcode << 24) + (rd << 20) + (rs << 16) + (rt << 12) + (imm);
			}
			else
				mem_line = (opcode << 24) + (rd << 20) + (rs << 16) + (rt << 12) + (imm + 512);

			memory[curr_memory] = mem_line; //storing the encoded line in the right place in memory
			curr_memory++;
		}
		if (curr_memory > max_address) 
			max_address = curr_memory;
	}
	program_length = max_address;
}

//Converts the immediate fields to location (if it is a label), to a decimal num (if it is an hexadecimal number) or just to an int.
int immediate_converter(char* imm)
{
	//if the imm is for a word (a label)- we want to replace it with the label's location
	if (isalpha(imm[0])) // if the first character in a letter- imm is a label
	{
		for (int i = 0; i < label_counter; i++) //finding the right label in the labels array
		{
			if (!strcmp(imm, labels[i].label_name))
			{
				return labels[i].location; //getting the right label's location
			}
		}
	}
	else if ((imm[0] == '0') && ((imm[1] == 'x') || (imm[1] == 'X'))) //the imm is a hexadecimal number
	{
		return HexToInt2sComp(imm);
	}
	else
	{
		return atoi(imm);
	}
}

//Convert Hex to int in 2's complement
int HexToInt2sComp(char* h) {
	int len = strlen(h);
	int res = 0;

	// Iterate over each character of the input string from right to left
	for (int i = 0; i < len - 2; i++) {
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

//Convert hexadecimal char to int
int HexCharToInt(char h) {
	if (isdigit(h)) {
		return h - '0'; // Convert digit character to its corresponding integer value
	}
	else {
		h = tolower(h); // Convert uppercase character to lowercase
		if (h >= 'a' && h <= 'f') {
			return h - 'a' + 10; // Convert lowercase hexadecimal character to its corresponding integer value
		}
	}
	return -1; // Invalid hexadecimal character
}

//Gets the input file (the test file in assembly), and the output file (memin.txt), and writes to the ouput file the encoded lines
void write_to_file(FILE* input_file, FILE* output_file) {
	first_pass(input_file);
	second_pass(input_file);
	for (int i = 0; i <= program_length; i++)
	{
		fprintf(output_file, "%08X", memory[i]);//print to outpot_file the hexadecimal value of the current memory, 8 digits.
		fwrite("\n", 1, 1, output_file);// writes \n into the file
	}
}

int main(int argc, char* argv[]) {
	FILE* input_file = fopen(argv[1], "r"); //open the input file- one of the Assemnly programs
	FILE* output_file = fopen(argv[2], "w"); //open the output file- memin.txt
	if (input_file == NULL || output_file == NULL) { 
		printf("Failed open the file");
		return 0;
	}
	write_to_file(input_file, output_file);
	fclose(input_file);
	fclose(output_file);
	return 0;

}
