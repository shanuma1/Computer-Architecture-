#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "file_parser.h"


extern int DEBUG;
extern int lines_skipped;
extern int last_line;
extern int error_pos;

struct ParseResult skip_whitespace(char *c, int pos, int len) {
    struct ParseResult returnVal;
    if(pos < len && (c[pos] == ' ')) {
        // skip spaces
        returnVal.value = 1;
        pos++;
    } else if(pos < len && (c[pos] == 10 || c[pos] == 13)) {
        // skip lines, and update the number of lines travered,
        // along with position of last line
        returnVal.value = 1;
        pos++;
        lines_skipped += 1;
        last_line = pos;
    } else {
        // otherwise we haven't skipped anything, value = 0
        returnVal.value = 0;
    }
    returnVal.parse_status = (pos >= len) ? END_OF_FILE : SUCCESS;
    returnVal.position = pos;
    return returnVal;
}

struct ParseResult skip_whitespaces(char *c, int pos, int len) {
    struct ParseResult returnVal;
    do {
        // skip white spaces unless a character other than
        // whitespace is found
        returnVal = skip_whitespace(c, pos, len);
        pos = returnVal.position;
    } while(returnVal.value != 0);
    return returnVal;
}

struct ParseResult is_digit(char *c, int pos, int len) {
    struct ParseResult returnVal;
    if(pos < len && c[pos] >= '0' && c[pos] <= '9') {
        // if character is a digit
        // get its value
        returnVal.parse_status = SUCCESS;
        returnVal.position = pos + 1;
        returnVal.value = c[pos] - '0';
    } else {
        // else we could not match a digit
        returnVal.parse_status = (pos >= len) ? END_OF_FILE : FAILURE;
    }
    return returnVal;
}

struct ParseResult is_digits(char *c, int pos, int len) {
    struct ParseResult returnVal;
    struct ParseResult result;
    int sign = 1;
    pos = skip_whitespaces(c, pos, len).position;
    if(c[pos] == '-') {
        // the digits could start with a negative sign
        sign = -1;
        pos = skip_whitespaces(c, pos+1, len).position;
    }
    // get the 1st digit
    result = is_digit(c, pos, len);
    int n;
    if(result.parse_status == SUCCESS) {
        n = result.value;
        pos = result.position;
        do {
            result = is_digit(c, pos, len);
            if(result.parse_status == SUCCESS) {
                // convert the string into
                // number, while parsing
                n = n * 10 + result.value;
                pos = result.position;
            }
        } while(result.parse_status == SUCCESS);
        result.parse_status = SUCCESS;
        result.position = pos;
        // set the sign of the final number
        result.value = n * sign;
        return result;
    } else {
        // if there was not even 1 digit, then this is not a number
        // return the result which was either FAILURE or END_OF_FILE
        return result;
    }
}

struct ParseResult is_register(char *c, int pos, int len) {
    struct ParseResult result;
    if(pos < len && (c[pos] == 'r' || c[pos] == 'R')) {
        // if the string starts with r or R
        // and there is a digit or two after it
        // then it is a register
        pos++;
        return is_digits(c, pos, len);
    } else {
        result.parse_status = (pos >= len) ? END_OF_FILE : FAILURE;
        return result;
    }
}

struct ParseResult is_instruction(char *c, int pos, int len) {
    struct ParseResult result;
    result.parse_status = SUCCESS;
    char c1, c2, c3, c4, c5;
    if(pos >= len) {
        result.parse_status = FAILURE;
        return result;
    }
    // GET the 1st character
    c1 = c[pos++];
    if(pos >= len) {
        // return failure if end of string encountered
        result.parse_status = FAILURE;
        return result;
    }
    // GET the 2nd character
    c2 = c[pos++];
    // Check against the 2 letter opcodes
    // OR, BZ
    if(c1 == 'O' && c2 == 'R') {
        result.value = OR;
        result.position = pos;
        return result;
    } else if(c1 == 'B' && c2 == 'Z') {
        result.value = BZ;
        result.position = pos;
        return result;
    } else {
        if(pos >= len) {
            // return failure if end of string encountered
            result.parse_status = FAILURE;
            return result;
        }
    }
    // GET the 3rd character
    c3 = c[pos++];
    // check against the 3 letter opcodes
    // ADD, SUB, AND, BNZ, MUL, LDR, STR, 
    if(c1 == 'A' && c2 == 'D' && c3 == 'D' && (pos == len || c[pos] != 'L')) {
        // if there is an L after `ADD`, it is another opcode
        result.value = ADD;
        result.position = pos;
        return result;
    } else if(c1 == 'S' && c2 == 'U' && c3 == 'B' && (pos == len || c[pos] != 'L')) {
        // if there is an L after `SUB`, it is another opcode
        result.value = SUB;
        result.position = pos;
        return result;
    } else if(c1 == 'A' && c2 == 'N' && c3 == 'D') {
        result.value = AND;
        result.position = pos;
        return result;
    } else if(c1 == 'B' && c2 == 'N' && c3 == 'Z') {
        result.value = BNZ;
        result.position = pos;
        return result;
    } else if(c1 == 'M' && c2 == 'U' && c3 == 'L') {
        result.value = MUL;
        result.position = pos;
        return result;
    } else if(c1 == 'L' && c2 == 'D' && c3 == 'R') {
        result.value = LDR;
        result.position = pos;
        return result;
    } else if(c1 == 'S' && c2 == 'T' && c3 == 'R') {
        result.value = STR;
        result.position = pos;
        return result;
    } else {
        if(pos >= len) {
            // return failure if end of string encountered
            result.parse_status = FAILURE;
            return result;
        }
    }
    // GET the 4th character
    c4 = c[pos++];
    // compare against the 4 letter opcodes
    // MOVC, ADDL, SUBL, LOAD, JUMP, HALT
    if(c1 == 'M' && c2 == 'O' && c3 == 'V' && c4 == 'C') {
        result.value = MOVC;
        result.position = pos;
        return result;
    } else if(c1 == 'A' && c2 == 'D' && c3 == 'D' && c4 == 'L') {
        result.value = ADDL;
        result.position = pos;
        return result;
    } else if(c1 == 'S' && c2 == 'U' && c3 == 'B' && c4 == 'L') {
        result.value = SUBL;
        result.position = pos;
        return result;
    } else if(c1 == 'L' && c2 == 'O' && c3 == 'A' && c4 == 'D') {
        result.value = LOAD;
        result.position = pos;
        return result;
    } else if(c1 == 'J' && c2 == 'U' && c3 == 'M' && c4 == 'P') {
        result.value = JUMP;
        result.position = pos;
        return result;
    } else if(c1 == 'H' && c2 == 'A' && c3 == 'L' && c4 == 'T') {
        result.value = HALT;
        result.position = pos;
        return result;
    } else {
        if(pos >= len) {
            // return failure if end of string encountered
            result.parse_status = FAILURE;
            return result;
        }
    }
    // GET the 5th character
    c5 = c[pos++];
    // Check against the 5 letter opcode
    // EX_OR
    if(c1 == 'E' && c2 == 'X' && c3 == '-' && c4 == 'O' && c5 == 'R') {
        result.value = EX_OR;
        result.position = pos;
        return result;
    } else {
        if(pos >= len) {
            // return failure if end of string encountered
            result.parse_status = FAILURE;
            return result;
        }
    }
}

struct ParseResult is_comma(char *c, int pos, int len) {
    struct ParseResult result;
    if(pos < len && c[pos] == ',') {
        // if comma matches, parsing was successfull
        result.position = pos + 1;
        result.parse_status = SUCCESS;
    } else {
        result.parse_status = (pos >= len) ? END_OF_FILE : FAILURE;
    }
    return result;
}

struct ParseResult is_hash(char *c, int pos, int len) {
    struct ParseResult result;
    if(pos < len && c[pos] == '#') {
        // if hash matches, parsing was successfull
        result.position = pos + 1;
        result.parse_status = SUCCESS;
    } else {
        result.parse_status = (pos >= len) ? END_OF_FILE : FAILURE;
    }
    return result;
}

struct opcodes *parse_opcodes(char *contents, int len) {
    struct ParseResult result;
    result = skip_whitespaces(contents, 0, len);
    int pos = result.position;
    struct opcode *op;
    struct opcodes *ops = opcodes_init();
    while(pos < len) {
        op = (struct  opcode*) malloc(sizeof(struct opcode));
        // line should start with instruction
        result = is_instruction(contents, pos, len);
        if(result.parse_status != SUCCESS) {
            error_pos = pos;
            return NULL;
        }
        pos = result.position;
        // set instruction type
        op->type = (enum instruction) result.value;
        // skip whitespaces
        pos = skip_whitespaces(contents, pos, len).position;
        if(op->type == HALT) {
            // HALT instruction does not need any arguments
            op->args = 0;
            opcodes_add(ops, op);
            continue;
        }
        // parse the comma
        result = is_comma(contents, pos, len);
        if(result.parse_status != SUCCESS) {
            error_pos = pos;
            return NULL;
        }
        pos = result.position;
        pos = skip_whitespaces(contents, pos, len).position;
        // next token should be a register or a number
        result = is_register(contents, pos, len);
        if(result.parse_status != SUCCESS) {
            // parse the hash symbol before the number
            result = is_hash(contents, pos, len);
            if(result.parse_status != SUCCESS) {
                error_pos = pos;
                return NULL;
            }
            pos = result.position;
            pos = skip_whitespaces(contents, pos, len).position;
            // parse the numbers
            result = is_digits(contents, pos, len);
            if(result.parse_status != SUCCESS) {
                error_pos = pos;
                return NULL;
            }
            pos = result.position;
            pos = skip_whitespaces(contents, pos, len).position;
            op->arg1_type = VALUE;
            op->arg1 = result.value;
            if(op->type == BZ || op->type == BNZ) {
                // BZ and BNZ, requires one literal argument, if this instruction
                // is one of them, continue to next line
                op->args = 1;
                opcodes_add(ops, op);
                continue;
            }
        }
        op->arg1_type = REGISTER;
        op->arg1 = result.value;
        pos = result.position;
        pos = skip_whitespaces(contents, pos, len).position;
        // parse the comma
        pos = is_comma(contents, pos, len).position;
        pos = skip_whitespaces(contents, pos, len).position;
        // next token should be a register or a number
        result = is_register(contents, pos, len);
        if(result.parse_status != SUCCESS) {
            // parse the hash symbol before the number
            result = is_hash(contents, pos, len);
            if(result.parse_status != SUCCESS) {
                error_pos = pos;
                return NULL;
            }
            pos = result.position;
            pos = skip_whitespaces(contents, pos, len).position;
            // parse the numbers
            result = is_digits(contents, pos, len);
            if(result.parse_status != SUCCESS) {
                error_pos = pos;
                return NULL;
            }
            pos = result.position;
            op->arg2_type = VALUE;
            op->arg2 = result.value;
        } else {
            op->arg2_type = REGISTER;
            op->arg2 = result.value;
            pos = result.position;
        }
        // skip white spaces
        pos = skip_whitespaces(contents, pos, len).position;
        if(op->type == MOVC || op->type == JUMP) {
            // only MOVC and JUMP instructions have 2 arguments
            // if this is one of them and in proper structure, proceed to next instruction
            if(op->arg1_type != REGISTER || op->arg2_type != VALUE) {
                error_pos = pos;
                return NULL;
            }
            op->args = 2;
            opcodes_add(ops, op);
            continue;
        }
        // parse comma
        result = is_comma(contents, pos, len);
        if(result.parse_status != SUCCESS) {
            error_pos = pos;
            return NULL;
        }
        pos = result.position;
        pos = skip_whitespaces(contents, pos, len).position;
        // next token is either a register or a number
        result = is_register(contents, pos, len);
        if(result.parse_status != SUCCESS) {
            // parse the hash symbol
            result = is_hash(contents, pos, len);
            if(result.parse_status != SUCCESS) {
                error_pos = pos;
                return NULL;
            }
            pos = result.position;
            pos = skip_whitespaces(contents, pos, len).position;
            // parse the number
            result = is_digits(contents, pos, len);
            if(result.parse_status != SUCCESS) {
                error_pos = pos;
                return NULL;
            }
            pos = result.position;
            op->arg3_type = VALUE;
            op->arg3 = result.value;
        } else {
            op->arg3_type = REGISTER;
            op->arg3 = result.value;
            pos = result.position;
        }
        pos = skip_whitespaces(contents, pos, len).position;
        if(op->type == ADDL || op->type == SUBL || op->type == LOAD || op->type == STORE) {
            // only ADDL, SUBL, LOAD and STORE instructions have 3 arguments with structure <ins> <Register> <Register> <value>
            // if this is one of them and the structure is correct,
            // proceed to the next instruction
            op->args = 3;
            if(op->arg1_type != REGISTER || op->arg2_type != REGISTER || op->arg3_type != VALUE) {
                error_pos = pos;
                return NULL;
            }
            op->args = 3;
            opcodes_add(ops, op);
            continue;
        } else if(op->type == ADD || op->type == SUB || op->type == AND || op->type == OR || op->type == EX_OR || op->type == MUL || op->type == LDR || op->type == STR) {
            // only ADD, SUB, AND, OR, EX_OR, MUL, LDR and STR instructions have 3 argumentswith structure <ins> <Register> <Register> <Register>
            // if this is one of them and the structure is correct,
            // proceed to the next instruction
            if(op->arg1_type != REGISTER || op->arg2_type != REGISTER || op->arg3_type != REGISTER) {
                error_pos = pos;
                return NULL;
            }
            op->args = 3;
            opcodes_add(ops, op);
            continue;
        }
    }
    return ops;
}