#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tables.h"
#include "translate_utils.h"
#include "translate.h"

/* Writes instructions during the assembler's first pass to OUTPUT. The case
   for general instructions has already been completed, but you need to write
   code to translate the li and blt pseudoinstructions. Your pseudoinstruction 
   expansions should not have any side effects.

   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS.

   Error checking for regular instructions are done in pass two. However, for
   pseudoinstructions, you must make sure that ARGS contains the correct number
   of arguments. You do NOT need to check whether the registers / label are 
   valid, since that will be checked in part two.

   Also for li:
    - make sure that the number is representable by 32 bits. (Hint: the number 
        can be both signed or unsigned).
    - if the immediate can fit in the imm field of an addiu instruction, then
        expand li into a single addiu instruction. Otherwise, expand it into 
        a lui-ori pair.

   And for blt:
    - your expansion should use the fewest number of instructions possible.

   MARS has slightly different translation rules for li, and it allows numbers
   larger than the largest 32 bit number to be loaded with li. You should follow
   the above rules if MARS behaves differently.

   Use fprintf() to write. If writing multiple instructions, make sure that 
   each instruction is on a different line.

   Returns the number of instructions written (so 0 if there were any errors).
 */

// Implement write_pass_one() in src/translate.c, which should perform pseudoinstruction 
// expansion on the load immediate (li) and branch on less than (blt) instructions. The 
// load immediate instruction normally get expanded into an lui-ori pair. However, an 
// optimization can be made when the immediate is small. If the immediate can fit inside 
// the imm field of an addiu instruction, we will use an addiu instruction instead. Other 
// assemblers may implement additional optimizations, but ours will not. For the blt 
// instruction, use the fewest number of instructions possible. Also, make sure that your 
// pseudoinstruction expansions do not produce any unintended side effects. You will also 
// be performing some error checking on the pseudoinstructions (see src/translate.c for 
//   details). If there is an error, do NOT write anything to the intermediate file, and 
// return 0 to indicate that 0 lines have been written.


//name = instruction, args = array of args; num_args = number of items in args
//pseudoinstructions, you must make sure that ARGS contains the correct number
unsigned write_pass_one(FILE* output, const char* name, char** args, int num_args) {
    if (strcmp(name, "li") == 0) {
        /* YOUR CODE HERE */

        // if immediate can fit in imm field of addiu instruction
            // expand li into single addiu instruction
            return 1;
        // expand into lui-ori pair
        return 2;



    } else if (strcmp(name, "blt") == 0) {
        /* YOUR CODE HERE */
        // one blt instruction-->   slt instrution, bne instruction


        return 2; // 2 instructions




    } else {
        write_inst_string(output, name, args, num_args);
        return 1;
    }
}


// translate_inst() should translate instructions to hexadecimal.

// You will find the translate_reg(), translate_num(), and 
// write_inst_hex() functions, all defined in translate_utils.h helpful in this step. 

// Some instructions may also require the symbol and/or relocation table, which are 
// give to you by the symtbl and reltbl pointers. This step may require writing a lot 
// of code, but the code should be similar in nature, and therefore not difficult. 
   
// The more important issue is input validation -- you must make sure that all arguments 
// given are valid. If an input is invalid, you should NOT write anything to output but return -1 instead.

// Use your knowledge about MIPS instruction formats and think carefully about how inputs 
// could be invalid. You are encouraged to use MARS as a resource. Do note that MARS has 
// more pseudoinstruction expansions than our assembler, which means that instructions 
// with invalid arguments for our assembler could be treated as a pseduoinstruction by MARS. 
// Therefore, you should check the text section after assembling to make sure that the instruction 
// has not been expanded by MARS .

// If a branch offset cannot fit inside the immediate field, you should treat it as an error.

/* Writes the instruction in hexadecimal format to OUTPUT during pass #2.
   
   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS. 

   The symbol table (SYMTBL) is given for any symbols that need to be resolved
   at this step. If a symbol should be relocated, it should be added to the
   relocation table (RELTBL), and the fields for that symbol should be set to
   all zeros. 

   Note the use of helper functions. Consider writing your own! If the function
   definition comes afterwards, you must declare it first (see translate.h).

   Returns 0 on success and -1 on error. 
 */

int translate_inst(FILE* output, const char* name, char** args, size_t num_args, uint32_t addr,
    SymbolTable* symtbl, SymbolTable* reltbl) {
    if (strcmp(name, "addu") == 0)       return write_rtype (0x21, output, args, num_args);
    else if (strcmp(name, "or") == 0)    return write_rtype (0x25, output, args, num_args);
    else if (strcmp(name, "slt") == 0)   return write_rtype (0x2a, output, args, num_args);
    else if (strcmp(name, "sltu") == 0)  return write_rtype (0x2b, output, args, num_args);
    
    else if (strcmp(name, "sll") == 0)   return write_shift (0x00, output, args, num_args);
    
    /* YOUR CODE HERE */
    
    else if (strcmp (name, "jr") == 0)   return write_jr (0x08, output, args, num_args);
    
    else if (strcmp (name, "addiu") == 0)return write_addiu (0x09, output, args, num_args);
    
    else if (strcmp (name, "ori") == 0)  return write_ori (0x0d, output, args, num_args);
    
    else if (strcmp (name, "lui") == 0)  return write_lui (0x0f, output, args, num_args);
    
    else if (strcmp (name, "lb") == 0)   return write_itype (0x09, output, args, num_args);
    else if (strcmp (name, "lbu") == 0)  return write_itype (0x24, output, args, num_args);
    else if (strcmp (name, "lw") == 0)   return write_itype (0x23, output, args, num_args);
    else if (strcmp (name, "sb") == 0)   return write_mem (0x28, output, args, num_args);
    else if (strcmp (name, "sw") == 0)   return write_mem (0x2b, output, args, num_args);

    // require symbol table
    else if (strcmp (name, "beq") == 0)  return write_branch (0x04, output, args, num_args, addr, symtbl);
    else if (strcmp (name, "bne") == 0)  return write_branch (0x05, output, args, num_args, addr, symtbl);
    else if (strcmp (name, "j") == 0)    return write_jump (0x02, output, args, num_args, addr, reltbl);
    else if (strcmp (name, "jal") == 0)  return write_jump (0x03, output, args, num_args, addr, reltbl);
    else                                 return -1;
}

int write_branch(uint8_t opcode, FILE* output, char** args, size_t num_args, 
    uint32_t addr, SymbolTable* symtbl) {

    // store into symtbl
    int rs = translate_reg(args[0]);
    int rt = translate_reg(args[1]);

    if (rs == -1 || rt == -1) { 
      return -1;
    }

    opcode << 25;
    rs << 20;
    rt << 15;

    char* name = **args; // name is a pointer to args. symbol name is stored in args[0]
    add_to_table(symtbl, name, addr);

    uint32_t instruction = rs + rt + opcode;
    write_inst_hex(output, instruction);
    return 0;
}

int write_mem(uint8_t opcode, FILE* output, char** args, size_t num_args) {
  
    // for storing
    // sb = sw: rs + signextimm = rt

    int rs = translate_reg(args[0]);
    int imm; //args[1]
    int rt = translate_reg(args[2]);
    int lower = pow(2, 16 - 1);
    int upper = pow(2, 16 - 1);
    upper -= 1;
    int err = translate_num(&imm, args[1], lower, upper);
    // lower bound = 2^(n-1). upper bound = 2^(n-1) - 1

    if (rt == -1 || rs == -1 || err == -1) { 
      return -1;
    }

    opcode << 25;
    rs << 20;
    rt << 15;

    uint32_t instruction = rt + rs + opcode + imm;
    write_inst_hex(output, instruction);
    return 0;
}


int write_lui(uint8_t opcode, FILE* output, char** args, size_t num_args) {

    // rt = imm | b

    int rt = translate_reg(args[0]);
    int imm;
    int lower = pow(2, 16 - 1);
    int upper = pow(2, 16 - 1);
    upper -= 1;
    int err = translate_num(&imm, args[1], lower, upper);
    // lower bound = 2^(n-1). upper bound = 2^(n-1) - 1

    if (rt == -1 || err == -1) { 
      return -1;
    }

    opcode << 25;
    rt << 15;

    uint32_t instruction = rt + opcode + imm;
    write_inst_hex(output, instruction);
    return 0;  
}

int write_jr(uint8_t funct, FILE* output, char** args, size_t num_args) {

    // jr. PC = rs 
    int rs = translate_reg(args[0]);

    if (rs == -1) { 
      return -1;
    }

    rs << 20;

    uint32_t instruction = rs + funct;
    write_inst_hex(output, instruction);
    return 0;
}


int write_ori(uint8_t opcode, FILE* output, char** args, size_t num_args) {

    // rt = rs | imm

    int rt = translate_reg(args[0]);
    int rs = translate_reg(args[1]);
    int imm;
    int lower = pow(2, 16 - 1);
    int upper = pow(2, 16 - 1);
    upper -= 1;
    int err = translate_num(&imm, args[2], lower, upper);
    // lower bound = 2^(n-1). upper bound = 2^(n-1) - 1

    if (rs == -1 || rt == -1 || err == -1) { 
      return -1;
    }

    opcode << 25;
    rs << 20;
    rt << 15;

    uint32_t instruction = opcode + rs + rt + imm;
    write_inst_hex(output, instruction);
    return 0;
}

int write_itype(uint8_t funct, FILE* output, char** args, size_t num_args) {
    // Perhaps perform some error checking?

    int rt = translate_reg(args[0]);
    int rs = translate_reg(args[1]);
    int imm;
    int lower = pow(2, 16 - 1);
    int upper = pow(2, 16 - 1);
    upper -= 1;
    int err = translate_num(&imm, args[2], lower, upper);
    // lower bound = 2^(n-1). upper bound = 2^(n-1) - 1

    if (rs == -1 || rt == -1 || err == -1) { 
      return -1;
    }

    rs << 20;
    rt << 15;

    uint32_t instruction = rs + rt + funct + imm;
    write_inst_hex(output, instruction);
    return 0;
}

int write_addiu(uint8_t opcode, FILE* output, char** args, size_t num_args) {

    // addiu. rt = rs + signextimm

    int rt = translate_reg(args[0]);
    int rs = translate_reg(args[1]);
    int imm;
    int lower = pow(2, 16 - 1);
    int upper = pow(2, 16 - 1);
    upper -= 1;
    int err = translate_num(&imm, args[2], lower, upper);
    // lower bound = 2^(n-1). upper bound = 2^(n-1) - 1

    if (rs == -1 || rt == -1 || err == -1) { 
      return -1;
    }

    opcode << 25;
    rs << 20;
    rt << 15;

    uint32_t instruction = rs + rt + opcode + imm;
    write_inst_hex(output, instruction);
    return 0;
}

int write_jump(uint8_t opcode, FILE* output, char** args, size_t num_args, 
    uint32_t addr, SymbolTable* reltbl) {
    // put relative address in relocation table. 

    // store into reltbl
    char* name = **args; // name is a pointer to args. symbol name is stored in args[0]
    add_to_table(reltbl, name, addr);
    return 0;
}

/* A helper function for writing most R-type instructions. You should use
   translate_reg() to parse registers and write_inst_hex() to write to 
   OUTPUT. Both are defined in translate_utils.h.

   This function is INCOMPLETE. Complete the implementation below. You will
   find bitwise operations to be the cleanest way to complete this function.
 */
int write_rtype(uint8_t funct, FILE* output, char** args, size_t num_args) {
    // Perhaps perform some error checking?

    //addu = rd = rs + rt
    //or = rd = rs | rt
    //slt = rd = rs < rt
    //sltu = rd = rs < rt

    int rd = translate_reg(args[0]);
    int rs = translate_reg(args[1]);
    int rt = translate_reg(args[2]);

    if (rd == -1 || rs == -1 || rt == -1) { 
      return -1;
    }

    rs << 20;
    rt << 15;
    rd << 10;

    uint32_t instruction = rs + rt + rd + funct;
    write_inst_hex(output, instruction);
    return 0;
}

/* A helper function for writing shift instructions. You should use 
   translate_num() to parse numerical arguments. translate_num() is defined
   in translate_utils.h.

   This function is INCOMPLETE. Complete the implementation below. You will
   find bitwise operations to be the cleanest way to complete this function.
 */
int write_shift(uint8_t funct, FILE* output, char** args, size_t num_args) {
	// Perhaps perform some error checking?

  // sll: rd = rt << shiftamt

    int rd = translate_reg(args[0]);
    int rt = translate_reg(args[1]);
    int shamt;
    int err = translate_num(&shamt, args[2], 0, 31); // 2^5 = 32

    if (rd == -1 || rt == -1 || err == -1) {
      return -1;
    }

    rt << 15;
    rd << 10;
    shamt << 5;

    uint32_t instruction = rd + rt + shamt + funct;
    write_inst_hex(output, instruction);
    return 0;
}