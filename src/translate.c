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
unsigned write_pass_one(FILE* output, const char* name, char** args, int num_args) {
    if (strcmp(name, "li") == 0) {
        //ex: li $8, 0x3BF20
        int instructions_written = 0; 
        long int imm;

        int i = translate_num(&imm, args[1], -2147483648, 4294967295);
        if (num_args == 2) {
          if (i == -1) {
            //immediate is too large 
            return 0; 
          } else if (-32768 <= imm && imm <= 65535) {
            //addiu case
            char charImm[20];
            snprintf(charImm, 16, "%lu", imm);
            char instructions[50]; //addiu $t3 $0 5
            strcpy(instructions, "addiu ");
            strcat(instructions, args[0]);
            strcat(instructions, " $0 ");
            strcat(instructions, charImm);
            strcat(instructions, "\n");
            instructions_written += 1;
            fprintf(output, "%s\n", instructions);
            //printf("\n\n%s\n", instructions); //delete me (for testing)
          } else {
            //lui ori
            long int imm2 = imm;
            char luiInst[30];
            char oriInst[30];
            char luiImm[30];
            char oriImm[30];

            strcpy(luiInst, "lui ");
            strcat(luiInst, args[0]);
            strcat(luiInst, " ");
            //get upper 16 bits
            imm = imm >> 16;
            imm = imm & 0x0000ffff;
            snprintf(luiImm, 16, "%lu", imm);
            strcat(luiInst, luiImm);

            strcpy(oriInst, "ori ");
            strcat(oriInst, args[0]);
            strcat(oriInst, " ");
            strcat(oriInst, args[0]);
            strcat(oriInst, " ");

            //get the lower 16 bits
            imm2 = imm2 & 0xffff;
            snprintf(oriImm, 16, "%lu", imm2);
            strcat(oriInst, oriImm);
            
            instructions_written += 2;
            fprintf(output, "%s\n", luiInst);
            fprintf(output, "%s\n", oriInst);
            //printf("%s\n", luiInst); //delete me (for testing)
            //printf("%s\n", oriInst); //delete me (for testing)
          }
        }
        return instructions_written;
    } else if (strcmp(name, "blt") == 0) {
        //ex: blt $8, $9, label
        int instructions_written = 0;
        if (num_args == 3) {
          char sltInst[30];
          char bneInst[30]; 

          strcpy(sltInst, "slt ");
          strcat(sltInst, "$at "); 
          strcat(sltInst, args[0]);
          strcat(sltInst, " ");
          strcat(sltInst, args[1]);
          strcat(sltInst, " ");

          strcpy(bneInst, "bne ");
          strcat(bneInst, "$at ");
          strcat(bneInst, "$0 ");
          strcat(bneInst, args[3]);

          fprintf(output, "%s\n", sltInst);
          fprintf(output, "%s\n", bneInst);

          //printf("\n%s\n", sltInst); //delete me (for testing)
          //printf("%s\n", bneInst); //delete me (for testing)
          
          instructions_written += 2;
        }
        return instructions_written;
    } else {
        /*Writes the instruction as a string to OUTPUT. NAME is the name of the 
         instruction, and its arguments are in ARGS. NUM_ARGS is the length of
         the array. */
        write_inst_string(output, name, args, num_args);
        return 1;
    }
}

/* Writes the instruction in hexadecimal format to OUTPUT during pass #2.
   
   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS. 

   The symbol table (SYMTBL) is given for any symbols that need to be resolved
   at this step. If a symbol should be relocated, it should be added to the
   relocation table (RELTBL), and the fields for that symbol should be set to
   all zeros. 

   You must perform error checking on all instructions and make sure that their
   arguments are valid. If an instruction is invalid, you should not write 
   anything to OUTPUT but simply return -1. MARS may be a useful resource for
   this step.

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
    else                                 return -1;
}

/* A helper function for writing most R-type instructions. You should use
   translate_reg() to parse registers and write_inst_hex() to write to 
   OUTPUT. Both are defined in translate_utils.h.

   This function is INCOMPLETE. Complete the implementation below. You will
   find bitwise operations to be the cleanest way to complete this function.
 */
int write_rtype(uint8_t funct, FILE* output, char** args, size_t num_args) {
    // Perhaps perform some error checking?

    int rd = translate_reg(args[0]);
    int rs = translate_reg(args[1]);
    int rt = translate_reg(args[2]);

    uint32_t instruction = 0;
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

    long int shamt;
    int rd = translate_reg(args[0]);
    int rt = translate_reg(args[1]);
    int err = translate_num(&shamt, args[2], 0, 31);

    uint32_t instruction = 0;
    write_inst_hex(output, instruction);
    return 0;
}
