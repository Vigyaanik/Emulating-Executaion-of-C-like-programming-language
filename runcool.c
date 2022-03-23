
//  compile with:  cc -std=c11 -Wall -Werror -o runcool runcool.c

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//  THE STACK-BASED MACHINE HAS 2^16 (= 65,536) WORDS OF MAIN MEMORY
#define N_MAIN_MEMORY_WORDS (1<<16)

//  EACH WORD OF MEMORY CAN STORE A 16-bit UNSIGNED ADDRESS (0 to 65535)
#define AWORD               uint16_t
//  OR STORE A 16-bit SIGNED INTEGER (-32,768 to 32,767)
#define IWORD               int16_t

//  THE ARRAY OF 65,536 WORDS OF MAIN MEMORY
AWORD                       main_memory[N_MAIN_MEMORY_WORDS];

//  THE SMALL-BUT-FAST CACHE HAS 32 WORDS OF MEMORY
#define N_CACHE_WORDS       32


//  see:  https://teaching.csse.uwa.edu.au/units/CITS2002/projects/coolinstructions.php
enum INSTRUCTION {
    I_HALT       = 0,
    I_NOP,
    I_ADD,
    I_SUB,
    I_MULT,
    I_DIV,
    I_CALL,
    I_RETURN,
    I_JMP,
    I_JEQ,
    I_PRINTI,
    I_PRINTS,
    I_PUSHC,
    I_PUSHA,
    I_PUSHR,
    I_POPA,
    I_POPR
};

//  USE VALUES OF enum INSTRUCTION TO INDEX THE INSTRUCTION_name[] ARRAY
const char *INSTRUCTION_name[] = {
    "halt",
    "nop",
    "add",
    "sub",
    "mult",
    "div",
    "call",
    "return",
    "jmp",
    "jeq",
    "printi",
    "prints",
    "pushc",
    "pusha",
    "pushr",
    "popa",
    "popr"
};

//  ----  IT IS SAFE TO MODIFY ANYTHING BELOW THIS LINE  --------------


//  THE STATISTICS TO BE ACCUMULATED AND REPORTED
int n_main_memory_reads     = 0;
int n_main_memory_writes    = 0;
int n_cache_memory_hits     = 0;
int n_cache_memory_misses   = 0;

void report_statistics(void)
{
    printf("@number-of-main-memory-reads\t%i\n",    n_main_memory_reads);
    printf("@number-of-main-memory-writes\t%i\n",   n_main_memory_writes);
    printf("@number-of-cache-memory-hits\t%i\n",    n_cache_memory_hits);
    printf("@number-of-cache-memory-misses\t%i\n",  n_cache_memory_misses);
}

//  -------------------------------------------------------------------

//  EVEN THOUGH main_memory[] IS AN ARRAY OF WORDS, IT SHOULD NOT BE ACCESSED DIRECTLY.
//  INSTEAD, USE THESE FUNCTIONS read_memory() and write_memory()
//
//  THIS WILL MAKE THINGS EASIER WHEN WHEN EXTENDING THE CODE TO
//  SUPPORT CACHE MEMORY

AWORD read_memory(int address)
{
    ++n_main_memory_reads;
    return main_memory[address];
}

void write_memory(AWORD address, AWORD value)
{
    ++n_main_memory_writes;
    main_memory[address] = value;
}

//  -------------------------------------------------------------------

//  EXECUTE THE INSTRUCTIONS IN main_memory[]
int execute_stackmachine(void)
{
//  THE 3 ON-CPU CONTROL REGISTERS:
    int PC      = 0;                    // 1st instruction is at address=0
    int SP      = N_MAIN_MEMORY_WORDS;  // initialised to top-of-stack
    int FP      = 0;                    // frame pointer

//  REMOVE THE FOLLOWING LINE ONCE YOU ACTUALLY NEED TO USE FP
    FP = FP+0;

    while(true) {
        IWORD value1;
        IWORD value2;
        IWORD offset;
        AWORD address_val;
//  FETCH THE NEXT INSTRUCTION TO BE EXECUTED
        IWORD instruction   = read_memory(PC);
        ++PC;

//      printf("%s\n", INSTRUCTION_name[instruction]);

        if(instruction == I_HALT) {
            break;
        }

//  SUPPORT OTHER INSTRUCTIONS HERE
//      ....
        switch (instruction) {
            case I_NOP:
                break;
            case I_ADD:
                value1 = (int16_t) read_memory(SP++);
                value2 = (int16_t) read_memory(SP);
                write_memory(SP, value1 + value2);
                break;
            case I_SUB:
                value1 = (int16_t) read_memory(SP++);
                value2 = (int16_t) read_memory(SP);
                write_memory(SP, value2 - value1);
                break;
            case I_MULT:
                value1 = (int16_t) read_memory(SP++);
                value2 = (int16_t) read_memory(SP);
                write_memory(SP, value1 * value2);
                break;
            case I_DIV:
                value1 = (int16_t) read_memory(SP++);
                value2 = (int16_t) read_memory(SP);
                write_memory(SP, value2 / value1);
                break;
            case I_CALL:
                address_val = read_memory(PC++);
                write_memory(--SP, PC); 
                write_memory(--SP, FP);
                FP = SP;
                PC = address_val;
                break;
            case I_RETURN:
                // temp_address = read_memory(PC++);
                // FP_new = FP + temp_address;
                address_val = FP + (int16_t) read_memory(PC++);
                value1 = (int16_t) read_memory(SP++);
                SP = FP;
                FP = read_memory(SP++);
                PC = read_memory(SP++);
                SP = address_val;
                write_memory(address_val, value1);
                break;
            case I_JMP:
                PC = read_memory(PC);
                break;
            case I_JEQ:
                value1 = (int16_t) read_memory(SP++);
                if (value1 == 0) {
                    PC = read_memory(PC);
                }
                else {
                    PC++;
                }
                break;
            case I_PRINTI:
                printf("%i", (int16_t) read_memory(SP++));
                break;
            case I_PRINTS:
                break;
            case I_PUSHC:
                value1 = (int16_t) read_memory(PC++);
                write_memory(--SP, value1);
                break;
            case I_PUSHA:
                address_val = read_memory(PC++);
                value1 = (int16_t) read_memory(address_val);
                write_memory(--SP, address_val);
                break;
            case I_PUSHR:
                offset = (int16_t) read_memory(PC++);
                address_val = FP + offset;
                value1 = (int16_t) read_memory(address_val);
                write_memory(--SP, value1);
                break;
            case I_POPA:
                address_val = read_memory(PC++);
                value1 = (int16_t) read_memory(SP++);
                write_memory(address_val, value1);
                break;
            case I_POPR:
                offset = (int16_t) read_memory(PC++);
                address_val = FP + offset;
                value1 = (int16_t) read_memory(SP++);
                write_memory(address_val, value1);
                break;
        }
    }

//  THE RESULT OF EXECUTING THE INSTRUCTIONS IS FOUND ON THE TOP-OF-STACK
    return read_memory(SP);
}

//  -------------------------------------------------------------------

//  READ THE PROVIDED coolexe FILE INTO main_memory[]
void read_coolexe_file(char filename[])
{
    memset(main_memory, 0, sizeof main_memory);   //  clear all memory

//  READ CONTENTS OF coolexe FILE
    FILE *fp = fopen(filename, "rb");
    
    IWORD buffer[1];
    int read_bytes;
    int mem_location = 0;

    while ((read_bytes = fread(buffer, sizeof buffer, 1, fp)) > 0)
    {
        //printf("Read in value: %i at %i\n", buffer[0], mem_location);     //for debugging
        main_memory[mem_location] = buffer[0];
        mem_location++;
    }
}

//  -------------------------------------------------------------------

int main(int argc, char *argv[])
{
//  CHECK THE NUMBER OF ARGUMENTS
    if(argc != 2) {
        fprintf(stderr, "Usage: %s program.coolexe\n", argv[0]);
        exit(EXIT_FAILURE);
    }

//  READ THE PROVIDED coolexe FILE INTO THE EMULATED MEMORY
    read_coolexe_file(argv[1]);

    for(AWORD loop = 0; loop < 50; loop++)
        printf("%d ", main_memory[loop]);

//  EXECUTE THE INSTRUCTIONS FOUND IN main_memory[]
    int result = execute_stackmachine();

    report_statistics();

    return result;          // or  exit(result);
}
