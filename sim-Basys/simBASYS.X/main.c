
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <xc.h>
#include <sys/attribs.h>

 //from previous sim
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <ctype.h>
//

#include "config.h"
#include "lcd.h"
#include "ssd.h"
#include "led.h"
#include "btn.h"
#include "swt.h"


#pragma config JTAGEN = OFF     
#pragma config FWDTEN = OFF      


/* ------------------------------------------------------------ */
/*						Configuration Bits		                */
/* ------------------------------------------------------------ */


// Device Config Bits in  DEVCFG1:	
#pragma config FNOSC =	PRIPLL
#pragma config FSOSCEN =	OFF
#pragma config POSCMOD =	XT
#pragma config OSCIOFNC =	ON
#pragma config FPBDIV =     DIV_2

// Device Config Bits in  DEVCFG2:	
#pragma config FPLLIDIV =	DIV_2
#pragma config FPLLMUL =	MUL_20
#pragma config FPLLODIV =	DIV_1


/*
 * Timing definitions
 */
 //#define CLKS_IN_SECOND (40000000)
 //#define CLKS_IN_SECOND (PB_FRQ)
#define CLKS_IN_SECOND (8000000*10/2)
#define CLKS_IN_MILLISECOND (CLKS_IN_SECOND / 1000)
#define SECONDS_IN_DAY (24*3600)

//Constant and gloabal paremeters.
#define Main_Memory_Max 512 // 2^9
#define Max_string_length 500 
#define Max_Label_Len 50

int registers_arr[16]; //registers array
static int memin_arr_size; //a constent to save memin actual size
static int pc = 0; //an int variable to save pc position at every clock cycle.
static int instructions_count = 0;  //an int variable to save the number of instructions the progrem as made.
char IOregister[15][9] = { "00000000","00000000","00000000","00000000","00000000","00000000","00000000","00000000","00000000",
"00000007","00000000","00000000","00000000","00000000","00000000" }; //array to represent registers.
int fib_size = 33;//fib memin array size.
char fib[Main_Memory_Max + 1][9] = {
    "00D01001", "04DD1007", "0E301020", "0D100006", "0F201021", "13000000", "00DD1FFD",
    "0F9D1002", "0FFD1001", "0F3D1000", "00501001", "0A13500E", "00230000", "07100018",
    "01331001", "0D100006", "00921000", "0E3D1000", "01331002", "0D100006", "00229000",
    "0E3D1000", "0EFD1001", "0E9D1002", "00DD1003", "07F00000", "00000000", "00000000",
    "00000000", "00000000", "00000000", "00000000", "00000007"
};//fib memin- initialized with correct values.
int timer_size = 90;//timer memin array size.
char timer[Main_Memory_Max + 1][9] = {
    "00DD11F9", "0F9D1006", "0FAD1005", "0FBD1004", "0F3D1003",
    "0F4D1002", "0F2D1001", "0FFD1000", "00500000", "00600000",
    "007013FF", "00801018", "12801006", "1270100D", "00901001",
    "00A00000", "00800000", "00700000", "12901000", "12901001",
    "12901002", "1290100B", "1220100A", "07100016", "11401003",
    "11B01004", "11301005", "0719B047", "0719401E", "0719304D",
    "12001003", "12001001", "12001002", "00551001", "00A00000",
    "00A0100A", "0715A029", "00221001", "12901001", "12901002",
    "10000000", "00500000", "00A00000", "00A01006", "00661001",
    "0716A032", "00221007", "12901001", "12901002", "10000000",
    "00500000", "00600000", "00A00000", "00A0100A", "00771001",
    "0717A03C", "002210A7", "12901001", "12901002", "10000000",
    "00500000", "00600000", "00700000", "00A00000", "00A01006",
    "00881001", "0718A04D", "002216A7", "12901001", "12901002",
    "10000000", "12001004", "11A0100B", "01A9A000", "12A0100B",
    "00A01000", "10000000", "12001005", "12001003", "00500000",
    "00600000", "00700000", "00800000", "00200000", "1220100A",
    "12901000", "12901001", "12901002", "10000000", "00000000"
};//timer memin- initialized with correct values.



char memin_arr[Main_Memory_Max + 1][9]; //the main memin array- would be initialized to fib memin/timer memin, in order to the switches state.
int pause_btnl = 0; //variable for pause condition (in order to BTNL press).
int single = 0; //variable for "single step" condition (in order to BTNR press).

int reg = 0; //variable for the registers that would be shown on the first row in the LCD screen
int curr_memory = 0; //variable for the memory places that would be shown on the first row in the LCD screen

int ready_to_irq = 1; //if equals to 1- we are currently not in an interrupt, so we are ready to get an interrupt.
int fib_flag = 0; //equals to 1 if fib is running
int reti_count = 0; //a counter for every time we see a reti insrtuction

//variable for every bottom press
int btnl_pressed = 0;
int btnr_pressed = 0;
int btnd_pressed = 0;
int btnu_pressed = 0;
int btnc_pressed = 0;

// Command structure for the CPU
typedef struct {
    char inst_line[9]; // Instruction line as a string
    int opcode;
    int rd;
    int rs;
    int rt;
    int imm;
} command;

//function to build command type variable.
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

//convert an Int into and hexadecimal number
void Int_to_Hex(int dec_num, char hex_num[9]) {
    if (dec_num < 0)
        dec_num = dec_num + 4294967296; // Add 2^32 to negative dec_num

    sprintf(hex_num, "%08X", dec_num); // Format dec_num as an 8-digit hexadecimal string
}

//converts and hexadecimal number using two's compliment, to int
int Hex_to_2sComp(const char* h) {
    int len = strlen(h);
    int res = 0;
    int isNegative = 0;

    // Check if the most significant bit is set (indicating a negative value)
    if (len > 0 && h[0] >= '8' && h[0] <= 'F') {
        isNegative = 1;
    }

    // Iterate over each character of the input string from right to left
    for (int i = 0; i < len; i++) {
        // Convert the hexadecimal character to its corresponding integer value
        int value = HexCharToInt(h[len - 1 - i]);

        // Multiply the value by 2 raised to the appropriate power of 4
        res += value * (1 << (4 * i));
    }

    // Perform sign extension if the value is negative
    if (isNegative) {
        // Perform sign extension by OR-ing the value with a negative number formed by shifting 1 to the left
        // by len * 4 bits, effectively setting all the higher bits to 1
        res |= -1 * (1 << (len * 4));
    }

    return res;
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

//Function to update leds in order to the reti instructions count
void led_updater()
{
    //to check how to jump after every reti instruction 

    //encountred reti instruction
	reti_count = reti_count + 1;
	LED_SetValue((reti_count - 2) % 8, 0);
	LED_ToggleValue((reti_count - 1) % 8);
}

// Function to perform the command, write to trace, manage pc and cycles, and halt
void perform(command* com)
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
        led_updater();
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
        instructions_count = 0;
        pc = 0;
        //		while(1){}
                //exit(0); // Terminate the program
    }

    clk_counter(); // Update the clock after executing the command
    instructions_count++; // Increment the number of executed instructions
}

// Last 32 bit clock counter sample
unsigned int clk32 = 0;

char buffer[80];//Buffer for writing to LCD

/*
 * Last 64 bit clock counter sample
 *
 * We maintain this counter in software by polling the 32 bit hardware
 * counter frequently, detecting wrap-around, and maintaining the high
 * 32 bits in software.
 */
unsigned int clk32_prev = 0;
unsigned int clk64_msb = 0;
unsigned long long clk64;


static void clk_poll(void)
{
    clk32 = _CP0_GET_COUNT();

    if (clk32 < clk32_prev) {
        clk64_msb++;
    }
    clk32_prev = clk32;
    clk64 = (((unsigned long long) clk64_msb) << 32) | clk32;
}

unsigned long long seconds = 0;

//Gets the source memin_array (FIB/Timer), and inserts it into the main memin array.
void copyAndFillArrays(char source[][9], int sourceSize, char destination[][9], int destinationSize) { //function to copy the program
    //we want to run(fib/timer) to the memin_array.
    // Copy the smaller array elements to the larger array elements
    for (int i = 0; i < sourceSize && i < destinationSize; i++) {
        strncpy(destination[i], source[i], 9);
    }

    // Fill the remaining spots in the larger array with '00000000'
    for (int i = sourceSize; i < destinationSize; i++) {
        strncpy(destination[i], "00000000", 9);
    }
}

//Gets a command line, check the switches and updates the LCD accordingly.
void check_swtiches(char c_line[9])
{
    if (!SWT_GetValue(0) && !SWT_GetValue(1)) //SW1=OFF, SW0=OFF: presents the current instruction
    {
        LCD_WriteStringAtPos("               ", 0, 0);
        sprintf(buffer, "%s", c_line);
        LCD_WriteStringAtPos(buffer, 0, 0);
    }
    else if (SWT_GetValue(0) && !SWT_GetValue(1)) //SW1=OFF, SW0=ON: presents the wanted register
    {
        LCD_WriteStringAtPos("               ", 0, 0);
        sprintf(buffer, "R%d = %08X", reg, registers_arr[reg]); 
//        sprintf(buffer, "R%d = %08s", reg, IOregister[reg]);
        LCD_WriteStringAtPos(buffer, 0, 0);
       
    }
    else if (!SWT_GetValue(0) && SWT_GetValue(1)) //SW1=ON, SW0=OFF: presents the current place in memory
    {
        LCD_WriteStringAtPos("               ", 0, 0);
        sprintf(buffer, "M%03X = %s", curr_memory, memin_arr[curr_memory]);
        LCD_WriteStringAtPos(buffer, 0, 0);
    }
    else if (SWT_GetValue(0) && SWT_GetValue(1)) //SW1=ON, SW0=ON: presents the current instruction count
    {
        LCD_WriteStringAtPos("               ", 0, 0);
        sprintf(buffer, "Ins count:%X", instructions_count);
        LCD_WriteStringAtPos(buffer, 0, 0);
    }
}

//checks the innterupts status and enable values, returns 1 if an irq occured (both status and enable is 1)
int irq_status_check()
{

    // Check if any of the irq interrupt conditions are satisfied and ready to handle interrupts
    if ((Hex_to_2sComp(IOregister[0]) && Hex_to_2sComp(IOregister[3])) ||
        (Hex_to_2sComp(IOregister[1]) && Hex_to_2sComp(IOregister[4])) ||
        (Hex_to_2sComp(IOregister[2]) && Hex_to_2sComp(IOregister[5])))
    {
        return 1;
    }
    else
    {
        return 0;
    }

    //Int_to_Hex8(0, IOregister[5]); // Set irq2status to 0 to indicate no active interrupts
}

//checks the switches status and BTNU presses, and updates reg and curr_memory variables accordingly
void check_btnu_press()
{
    if (SWT_GetValue(0) && !SWT_GetValue(1)) //SW1=OFF, SW0=ON: presents the wanted register
    {
        //if BTNU is pressed updates the presented register (0-15)
        while (BTN_GetValue(0))
        {
            btnu_pressed = 1;
        }
        if (btnu_pressed == 1)
        {
            reg++; //current register shown on the LCD screen
            if (reg > 15)
                reg = 0;
            btnu_pressed = 0;
        }
    }
    else if (!SWT_GetValue(0) && SWT_GetValue(1)) //SW1=ON, SW0=OFF: presents the current place in memory
    {
        //if BTNU is pressed- updates the place in memory that is presented
        while (BTN_GetValue(0))
        {
            btnu_pressed = 1;
        }
        if (btnu_pressed == 1)
        {
            curr_memory++; //current memory shown on the LCD screen
            if (curr_memory == 511)
                curr_memory = 0;
            btnu_pressed = 0;
        }
        
    }
}

int main()
{
    //libraries initialization for the used IO modules
    LCD_Init();
    LED_Init();
    SWT_Init();
    SSD_Init();
    BTN_Init();
    LED_SetGroupValue(0);
    
    //Switch 7 check
    if (SWT_GetValue(7)) //if switch 7 is on enter timer to memory array
    {
        copyAndFillArrays(timer, timer_size, memin_arr, 512);
    }
    else if (!SWT_GetValue(7)) //if switch 7 is off enter fib array to memory array.
    {
        fib_flag = 1;
        copyAndFillArrays(fib, fib_size, memin_arr, 512);
    }
    unsigned long long seconds = 0, frequency = 1024, last_frequency = 0, current_frequency = 0;
    int frequency_test = 1; //is 1 if a new cycle began
    int io12val; //IOregister[12] value

//    int program_checker = 0; //equal 0 if we run fib, equal 1 if we run timer.
    Int_to_Hex(1023, IOregister[13]); //initialize timermax to 1023
    Int_to_Hex(0, IOregister[12]); //initislize timercurrent to 0
    clk_poll(); //polling the current clk64 (before starting the program)

    current_frequency = frequency * clk64 / CLKS_IN_SECOND; //current relative place in the 1024 cycles a second before starting the program
    last_frequency = clk64; //current frequency before starting the program
    
    while (1) {
        clk_poll();//polling the current clk64
        seconds = clk64 / CLKS_IN_SECOND; //current seconds
        current_frequency = frequency * clk64 / CLKS_IN_SECOND; //current relative place in the 1024 cycles a second
        
        //checking whether a new cycle started, by defenition of 1024 cycles a second  
        if (clk64 >= (last_frequency + (CLKS_IN_SECOND / 1024)))
        {
            last_frequency += (CLKS_IN_SECOND / 1024);//updating the last_frequency
            frequency_test = 1;//a new cycle started- a flag to run a new command
        }

        // Fetch the instruction from memin_arr at the current program counter (pc)
        char command_line[9];
        strcpy(command_line, memin_arr[pc]);
        
        check_btnu_press(); //Changes reg/curr_mem according to BTNU presses
        
        //If we are in pause (caused by BTNL press)
        if (pause_btnl == 1)
        {
            if(current_frequency % 512 == 360)//updating the LCD screen- twice a second
            {
                sprintf(buffer, "%03x %03x %08d", pc, registers_arr[13], seconds); //updating second row in LCD 
                LCD_WriteStringAtPos(buffer, 1, 0);
                check_swtiches(command_line); //updating first row in LCD
            }
            
            //if we are in pause and BTNL is pressed again
            while (BTN_GetValue(1) == 1)
            {
                btnl_pressed = 1;
            }
            if (btnl_pressed == 1)
            {
                btnl_pressed = 0;
                pause_btnl = 0;//pause_btnl flag would be 0 (not in pause anymore)
            }
            //single step- if BTNR was pressed
            while (BTN_GetValue(3) == 1)
            {
                btnr_pressed = 1;
            }
            if (btnr_pressed == 1)
            {
                single = 1;//single step flag would be 1 (we are in a single step mode)
                btnr_pressed = 0;
            }
        }
        
        //updating the LCD screen- twice a second
        if(current_frequency % 512 == 360 && !pause_btnl)
        {
            sprintf(buffer, "%03x %03x %08d", pc, registers_arr[13], seconds); //RSP is %sp 
            LCD_WriteStringAtPos(buffer, 1, 0);
            check_swtiches(command_line);
        }
        
        //In case IOregister[12] == IOregister[13]: 1024 cycles have passed- it means one second have passed.
        //also to get in this condition we need frequency_test == 1 (a new cycle began), pause_btnl == 0 (not in pause caused by BTNL), or single == 1 (single step mode)
        if ((Hex_to_2sComp(IOregister[12]) == Hex_to_2sComp(IOregister[13])) && frequency_test && (!pause_btnl || single))
        {
            frequency_test = 0; //take down the frequency_test flag

            Int_to_Hex(1, IOregister[3]); //set irq0status to 1
            Int_to_Hex(0, IOregister[12]); //initialize timercurrent to 0
            if (irq_status_check() == 1 && ready_to_irq) //check if status is 1, enable is 1 and ready to irq
            {
                ready_to_irq = 0; //processor is in irq routine
                Int_to_Hex(pc, IOregister[7]); //saving the cuurent PC in reg 7
                pc = Hex_to_2sComp(IOregister[6]); //jumping to the irq routine
            }
            
            // Fetch the instruction from memin_arr at the current program counter (pc)
            char command_line[9];
            strcpy(command_line, memin_arr[pc]);

            // get current timer from IOregister[10]]
            int timer_reg10 = strtol(IOregister[10], NULL, 16);
            SSD_WriteDigitsGrouped(timer_reg10, 0); //print the current timer value on SSD display.

            //pressing BTNL would cause a pause
            while (BTN_GetValue(1) == 1 && pause_btnl == 0)
            {
                btnl_pressed = 1;
            }
            if (btnl_pressed == 1 && pause_btnl == 0)
            {
                pause_btnl = 1; //pause flag is on
                btnl_pressed = 0;
            }

            // Build the command structure from the command line
            command com;
            build_command(command_line, &com);
            single = 0; //turning off single step flag

            // Perform the operation specified by the command
            perform(&com);
            
            //if timer enable register is 1 / we are urrently running FIB- increase IOregister12 value
            if (Hex_to_2sComp(IOregister[11]) == 1 || fib_flag)
            {
                io12val = strtol(IOregister[12], NULL, 16);
                Int_to_Hex(io12val + 1, IOregister[12]);
            }

        }

        //In every case a second didnt pass (IOreg12 =/= IOreg13), we get into the condition if this is a new cycle (frequency_test == 1), we are not in pause / we are in a single step mode
        else if (((!pause_btnl || single) && (frequency_test)))
        {
            frequency_test = 0; //turning off frequency_test
            
            //IRQs
            while (BTN_GetValue(2)) //Inq1 occurred: BTNC was pressed
            {
                btnc_pressed = 1;
            }
            if (btnc_pressed == 1)
            {
                Int_to_Hex(1, IOregister[4]); //set irq1status to 1
                if (irq_status_check() == 1 && ready_to_irq) //check if status is 1, enable is 1 and ready to irq
                {
                    ready_to_irq = 0; //processor is in irq routine
                    Int_to_Hex(pc, IOregister[7]); //saving the cuurent PC in reg 7
                    pc = Hex_to_2sComp(IOregister[6]); //jumping to the irq routine
                }
                btnc_pressed = 0;
            }

            while (BTN_GetValue(4)) //Inq2 occurred: BTND was pressed
            {
                btnd_pressed = 1;
            }
            if (btnd_pressed == 1)
            {
                Int_to_Hex(1, IOregister[5]); //set irq2status to 1
                if (irq_status_check() == 1 && ready_to_irq) //check if status is 1, enable is 1 and ready to irq
                {
                    ready_to_irq = 0; //processor is in irq routine
                    Int_to_Hex(pc, IOregister[7]); //saving the cuurent PC in reg 7
                    pc = Hex_to_2sComp(IOregister[6]); //jumping to the irq routine
                }
                btnd_pressed = 0;
            }

            // Fetch the instruction from memin_arr at the current program counter (pc)
            char command_line[9];
            strcpy(command_line, memin_arr[pc]);
            
            // get current timer from IOregister[10]
            int timer_reg10 = strtol(IOregister[10], NULL, 16);
            SSD_WriteDigitsGrouped(timer_reg10, 0); //print the current timer value on SSD display.

            //pressing BTNL would cause a pause
            while (BTN_GetValue(1) == 1 && pause_btnl == 0)
            {
                btnl_pressed = 1;
            }
            if (btnl_pressed == 1 && pause_btnl == 0)
            {
                pause_btnl = 1;//pause flag is on
                btnl_pressed = 0;
            }

            //if timer enable register is 1 / we are urrently running FIB- increase IOregister12 value
            if (Hex_to_2sComp(IOregister[11]) == 1 || fib_flag)
            {
                io12val = strtol(IOregister[12], NULL, 16);
                Int_to_Hex(io12val + 1, IOregister[12]);
            }

            // Build the command structure from the command line            
            command com;
            build_command(command_line, &com);
            single = 0; //turning off single step flag

            // Perform the operation specified by the command
            perform(&com);
        }
    }
    return 0;
}
