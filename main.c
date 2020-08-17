#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_parser.h"
#include "cpu.h"

int file_size = 0, error_pos = 0, lines_skipped = 1, last_line = 0;
int DEBUG = 1;

char *readFile(char *filename) {
    FILE *f = fopen(filename, "rt");
    if(f) {
        // get file size
        fseek(f, 0, SEEK_END);
        long length = ftell(f);
        file_size = length;
        fseek(f, 0, SEEK_SET);
        // allocate memory according the file size
        char *buffer = (char *) malloc(length + 1);
        buffer[length] = '\0';
        fread(buffer, 1, length, f);
        fclose(f);
        return buffer;
    }
    return NULL;
}


int main(int argc, char **argv) {
    char *file_path;
    // check whether user has specified commandline arguments
    if(argc == 1) {
        printf("No Input files specified!\n");
        return 1;
    } else {
        // the 2nd agrument is the file path / file name
        file_path = argv[1];
    }
    // read whole file to character array
    char *contents = readFile(file_path);
    if(contents == NULL) {
        printf("Could Not Open File '%s' for Reading", file_path);
        return 1;
    }
    // parse the opcodes
    struct opcodes *ops = parse_opcodes(contents, file_size);
    if(ops == NULL) {
        printf("Syntax Error!, Line: %d, Position: %d\n", lines_skipped, (error_pos - last_line) + 1);
        return 1;
    }
    // Initialize the CPU
    struct CPU cpu;
    CPU_Initialize(&cpu);
    cpu.ops = ops;
    int n;
    if(argc >= 4) {
        if(strcmp(argv[2], "simulate") == 0) {
            // if we need to simulate
            // get the number of cycles
            struct ParseResult result = is_digits(argv[3], 0, strlen(argv[3]));
            if(result.parse_status != SUCCESS) {
                printf("Invalid argument '%s', expected number", argv[3]);
            }
            // don't print the stages for the cycles
            DEBUG = 0;
            // start simulation
            CPU_simulate_cycles(&cpu, result.value);
            // Show the registers
            printf("\n___________Registers_____________\n");
            for(int i=0; i<15; i++) {
                printf("R%d: %d\n", i, cpu.regs[i]);
            }
            // Show the memory
            printf("_____________________________________\n\n");
            printf("\n___________Memory_________________\n");
            for(int i=0; i<10; i++) {
                for(int j=0; j<10; j++) {
                    printf("%c", cpu.memory[i*10 + j]);
                }
                printf("\n");
            }
            printf("\n\n\n");
        } else if(strcmp(argv[2], "display") == 0) {
            // if we need to display the stages
            // get the number of cycles
            struct ParseResult result = is_digits(argv[3], 0, strlen(argv[3]));
            if(result.parse_status != SUCCESS) {
                printf("Invalid argument '%s', expected number", argv[3]);
            }
            // we want to print the stages for each cycle
            DEBUG = 1;
            // start simulation
            CPU_simulate_cycles(&cpu, result.value);
            // Show the registers
            printf("\n___________Registers_____________\n");
            for(int i=0; i<15; i++) {
                printf("R%d: %d\n", i, cpu.regs[i]);
            }
            printf("_____________________________________\n\n");
            // Show the memory
            printf("\n___________Memory_________________\n");
            for(int i=0; i<10; i++) {
                for(int j=0; j<10; j++) {
                    printf("%c", cpu.memory[i*10 + j]);
                }
                printf("\n");
            }
            printf("\n\n\n");
        }
    }
    return 0;
}