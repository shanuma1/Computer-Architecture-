// all instructions
enum instruction {
    ADD,
    ADDL,
    SUB,
    SUBL,
    MOVC,
    AND,
    LDR,
    STR,
    MUL,
    OR,
    EX_OR,
    LOAD,
    STORE,
    BZ,
    BNZ,
    JUMP,
    HALT
};

struct CPU_Stage {
    int pc;
    struct opcode *op;
    int busy;
    int empty;
    int regs[16];
    int Z;
};

struct CPU {
    struct opcodes *ops;
    int halted;
    int stalled;
    int pc;
    int regs[16];
    int memory[4000];
    int clock;
    int mem_size;
    int Z;
    struct CPU_Stage stages[7];
};

enum argument_type {
    REGISTER,
    VALUE
};

// stages
enum stage {
    NONE,
    FETCH,
    DECODE,
    EX1,
    EX2,
    MEM1,
    MEM2,
    WB
};

struct opcode {
    enum instruction type;
    enum argument_type arg1_type;
    int arg1;
    enum argument_type arg2_type;
    int arg2;
    enum argument_type arg3_type;
    int arg3;
    int args;
};

struct opcodes {
    int size;
    int capacity;
    struct opcode **opcode_data;
};

int CPU_simulate_cycles(struct CPU *cpu, int n);
int CPU_Initialize(struct CPU *cpu);
struct opcodes *opcodes_init();
void opcodes_add(struct opcodes *ops, struct opcode *op);
void opcodes_free(struct opcodes *ops);

