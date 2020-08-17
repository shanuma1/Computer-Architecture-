#include <stdio.h>
#include <stdlib.h>
#include "file_parser.h"
#include "cpu.h"

extern int DEBUG;

int print_instruction(struct opcode *op) {
    // print String representation of the name of the opcode
    if(op->type == MOVC) {
        printf("MOVC ");
    } else if(op->type == LOAD) {
        printf("LOAD ");
    } else if(op->type == STORE) {
        printf("STORE ");
    } else if(op->type == ADDL) {
        printf("ADDL ");
    } else if(op->type == SUBL) {
        printf("SUBL ");
    } else if(op->type == ADD) {
        printf("ADD ");
    } else if(op->type == SUB) {
        printf("SUB ");
    } else if(op->type == LDR) {
        printf("LDR ");
    } else if(op->type == STR) {
        printf("STR ");
    } else if(op->type == MUL) {
        printf("MUL ");
    } else if(op->type == OR) {
        printf("OR ");
    } else if(op->type == EX_OR) {
        printf("EX-OR ");
    } else if(op->type == AND) {
        printf("AND ");
    } else if(op->type == BZ) {
        printf("BZ ");
    } else if(op->type == BNZ) {
        printf("BNZ ");
    } else if(op->type == JUMP) {
        printf("JUMP ");
    } else if(op->type == HALT) {
        printf("HALT ");
    }
    if(op->args > 0) {
        if(op->arg1_type == REGISTER) {
            printf("R%d", op->arg1);
        } else {
            printf("%d", op->arg1);
        }
    }
    if(op->args > 1) {
        if(op->arg2_type == REGISTER) {
            printf(", R%d", op->arg2);
        } else {
            printf(", %d", op->arg2);
        }
    }
    if(op->args > 2) {
        if(op->arg3_type == REGISTER) {
            printf(", R%d", op->arg3);
        } else {
            printf(", %d", op->arg3);
        }
    }
    printf("\n");
}

int print_stage_content(char *name, struct CPU_Stage *stage) {
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage->op);
    printf("\n");
}

struct opcodes *opcodes_init() {
    // initialize opcodes array with initial capacity
    struct opcodes *ops = malloc(sizeof(struct opcodes));
    ops->capacity = 10000;
    ops->size = 0;
    ops->opcode_data = calloc(sizeof(struct opcode**), ops->capacity);
    return ops;
}

void opcodes_add(struct opcodes *ops, struct opcode *op) {
    // if size is reached, increase capacity
    if(ops->capacity >= ops->size) {
        ops->capacity = ((ops->capacity * 2) / 3) + 1;
        ops->opcode_data = realloc(ops->opcode_data, sizeof(struct opcode**) * ops->capacity);
    }
    ops->opcode_data[ops->size] = op;
    ops->size += 1;
}

void opcodes_free(struct opcodes *ops) {
    if(ops != NULL) {
        for(int i=0; i<ops->size; i++) {
            free(ops->opcode_data[i]);
        }
    }
    free(ops);
}

int CPU_Initialize(struct CPU *cpu) {
    // Set initial values for all the
    // data members in struct CPU
    cpu->halted = 0;
    cpu->stalled = 0;
    cpu->Z = 0;
    cpu->mem_size = 4000;
    cpu->clock = 0;
    cpu->pc = 4000;
    for(int i=0; i<16; i++) {
        cpu->regs[i] = 0;
    }
    for(int i=0; i<4000; i++) {
        cpu->memory[i] = 0;
    }
    for(int i=0; i<7; i++) {
        cpu->stages[i].empty = 1;
    }
}

int CPU_Fetch(struct CPU *cpu) {
    struct opcode *op;
    if(cpu->halted || cpu->stalled || (cpu->pc-4000)/4 >= cpu->ops->size) {
        // move the current stage by 1 to the right
        // if we can't fetch now, set current stage as empty
        cpu->stages[0].empty = 1;
        cpu->stages[1] = cpu->stages[0];
        for(int i=0; i<16; i++) {
            cpu->stages[1].regs[i] = cpu->regs[i];
        }
        cpu->stages[1].Z = cpu->Z;
        cpu->stages[0].empty = 1;
        return 0;
    }
    // move the current stage by 1 to the right
    // set current stage as empty
    cpu->stages[0].pc = cpu->pc;
    op = cpu->ops->opcode_data[(cpu->pc - 4000) / 4];
    cpu->stages[0].op = op;
    cpu->stages[0].empty = 0;
    // Perform jump so that, the next instruction
    // to be fetched, is the destination of the jump
    if(op->type == BZ && cpu->Z == 1 || op->type == BNZ && cpu->Z == 0) {
        cpu->pc = op->arg1;
    } else if(op->type == JUMP) {
        cpu->pc = cpu->regs[op->arg1] + op->arg2;
    } else {
        cpu->pc += 4;
    }
    if(DEBUG) {
        print_stage_content("Fetch", &cpu->stages[0]);
    }
    cpu->stages[1] = cpu->stages[0];
    for(int i=0; i<16; i++) {
        cpu->stages[1].regs[i] = cpu->regs[i];
    }
    cpu->stages[1].Z = cpu->Z;
    cpu->stages[0].empty = 1;
}

int CPU_Decode(struct CPU *cpu) {
    struct opcode *op;
    if(!cpu->stalled && cpu->stages[1].empty != 1) {
        op = cpu->stages[1].op;
        if(op->type == HALT) {
            // halt the CPU, we dont, want to fetch any
            // more instructions but we let the instructions
            // in stages to complete
            cpu->halted = 1;
        }
        if(DEBUG) {
            print_stage_content("Decode/RF", &cpu->stages[1]);
        }
        cpu->stages[2] = cpu->stages[1];
        for(int i=0; i<16; i++) {
            cpu->stages[2].regs[i] = cpu->regs[i];
        }
        cpu->stages[2].Z = cpu->Z;
        cpu->stages[1].empty = 1;
    }
}

int CPU_Execute1(struct CPU *cpu) {
    struct opcode *op;
    if(!cpu->stalled && cpu->stages[2].empty != 1) {
        if(DEBUG) {
            print_stage_content("Execute1", &cpu->stages[2]);
        }
        cpu->stages[3] = cpu->stages[2];
        for(int i=0; i<16; i++) {
            cpu->stages[3].regs[i] = cpu->regs[i];
        }
        cpu->stages[3].Z = cpu->Z;
        cpu->stages[2].empty = 1;
    }
}

int CPU_Execute2(struct CPU *cpu) {
    struct opcode *op;
    if(!cpu->stalled && cpu->stages[3].empty != 1) {
        if(cpu->stages[3].op->type == MOVC) {
        }
        if(DEBUG) {
            print_stage_content("Execute2", &cpu->stages[3]);
        }
        cpu->stages[4] = cpu->stages[3];
        for(int i=0; i<16; i++) {
            cpu->stages[4].regs[i] = cpu->regs[i];
        }
        cpu->stages[4].Z = cpu->Z;
        cpu->stages[3].empty = 1;
    }
}

int CPU_Memory1(struct CPU *cpu) {
    struct opcode *op;
    if(!cpu->stalled && cpu->stages[4].empty != 1) {
        op = cpu->stages[4].op;
        if(DEBUG) {
            print_stage_content("Memory1", &cpu->stages[4]);
        }
        cpu->stages[5] = cpu->stages[4];
        for(int i=0; i<16; i++) {
            cpu->stages[5].regs[i] = cpu->regs[i];
        }
        cpu->stages[5].Z = cpu->Z;
        cpu->stages[4].empty = 1;
    }
}

int CPU_Memory2(struct CPU *cpu) {
    struct opcode *op;
    if(!cpu->stalled && cpu->stages[5].empty != 1) {
        op = cpu->stages[4].op;
        if(op->type == MOVC) {
        } else if(op->type == LOAD) {
            // LOAD the value in memory to register
            cpu->regs[op->arg1] = cpu->memory[cpu->stages[5].regs[op->arg2] + op->arg3];
        } else if(op->type == STORE) {
            // Store the value of (Register + Literal) to memory
            cpu->memory[cpu->stages[5].regs[op->arg1]] = cpu->stages[5].regs[op->arg2] + op->arg3;
        } else if(op->type == LDR) {
            // Load the value in memory position (sum of the registers)
            cpu->regs[op->arg1] = cpu->memory[cpu->stages[5].regs[op->arg2] + cpu->stages[5].regs[op->arg3]];
        } else if(op->type == STR) {
            // Store the value in (Sum of the registers) to memory
            cpu->memory[cpu->stages[5].regs[op->arg1]] = cpu->stages[5].regs[op->arg2] + cpu->stages[5].regs[op->arg3];
        }
        if(DEBUG) {
            print_stage_content("Memory2", &cpu->stages[5]);
        }
        cpu->stages[6] = cpu->stages[5];
        for(int i=0; i<16; i++) {
            cpu->stages[6].regs[i] = cpu->regs[i];
        }
        cpu->stages[6].Z = cpu->Z;
        cpu->stages[5].empty = 1;
    }
}

int CPU_WriteBack(struct CPU *cpu) {
    struct opcode *op;
    if(!cpu->stalled && cpu->stages[6].empty != 1) {
        op = cpu->stages[6].op;
        if(op->type == MOVC) {
            // MOV <dest>, <literal>
            cpu->stages[6].regs[op->arg1] = op->arg2;
        } else if(op->type == ADD) {
            // ADD <dest>, <src1>, <src2>
            cpu->stages[6].regs[op->arg1] = cpu->stages[6].regs[op->arg2] + cpu->stages[6].regs[op->arg3];
            if(cpu->stages[6].regs[op->arg1] == 0) {
                cpu->stages[6].Z = 1;
            }
        } else if(op->type == ADDL) {
            // ADDL <dest>, <src1>, <literal>
            cpu->stages[6].regs[op->arg1] = cpu->stages[6].regs[op->arg2] + op->arg3;
            if(cpu->regs[op->arg1] == 0) {
                cpu->stages[6].Z = 1;
            }
        } else if(op->type == SUB) {
            // SUB <dest>, <src1>, <src2>
            cpu->stages[6].regs[op->arg1] = cpu->stages[6].regs[op->arg2] - cpu->stages[6].regs[op->arg3];
            if(cpu->regs[op->arg1] == 0) {
                cpu->stages[6].Z = 1;
            }
        } else if(op->type == SUBL) {
            // SUBL <dest>, <src1>, <literal>
            cpu->stages[6].regs[op->arg1] = cpu->stages[6].regs[op->arg2] - op->arg3;
            if(cpu->regs[op->arg1] == 0) {
                cpu->stages[6].Z = 1;
            }
        } else if(op->type == MUL) {
            // MUL <dest>, <src1>, <src2>
            cpu->stages[6].regs[op->arg1] = cpu->stages[6].regs[op->arg2] * cpu->stages[6].regs[op->arg3];
            if(cpu->stages[6].regs[op->arg1] == 0) {
                cpu->stages[6].Z = 1;
            }
        } else if(op->type == AND) {
            // AND <dest>, <src1>, <src2>
        } else if(op->type == OR) {
            // OR <dest>, <src1>, <src2>
        } else if(op->type == EX_OR) {
            // EX-OR <dest>, <src1>, <src2>
        }
        for(int i=0; i<16; i++) {
            cpu->regs[i] = cpu->stages[6].regs[i];
        }
        cpu->Z = cpu->stages[6].Z;
        if(DEBUG) {
            print_stage_content("WriteBack", &cpu->stages[6]);
        }
        cpu->stages[6].empty = 1;
    }
}

int CPU_Finished(struct CPU *cpu) {
    // CPU has finished if all
    // the stages are empty
    for(int i=0; i<7; i++) {
        if(cpu->stages[i].empty == 0) {
            return 0;
        }
    }
    return 1;
}

int CPU_simulate_cycles(struct CPU *cpu, int n) {
    int i = 0;
    do {
        cpu->clock += 1;
        i++;
        if (DEBUG) {
            // Show this only in DEBUG mode
            printf("--------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock);
            printf("--------------------------------\n");
        }
        CPU_WriteBack(cpu);
        CPU_Memory2(cpu);
        CPU_Memory1(cpu);
        CPU_Execute2(cpu);
        CPU_Execute1(cpu);
        CPU_Decode(cpu);
        CPU_Fetch(cpu);
        if(i == n) {
            return 0;
        }
    // loop until all the stages get empty
    } while(!CPU_Finished(cpu));
}