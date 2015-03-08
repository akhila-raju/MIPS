#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "tables.h"

const int SYMTBL_NON_UNIQUE = 0;
const int SYMTBL_UNIQUE_NAME = 1;

/*******************************
 * Helper Functions
 *******************************/

void allocation_failed() {
    write_to_log("Error: allocation failed\n");
    exit(1);
}

void addr_alignment_incorrect() {
    write_to_log("Error: address is not a multiple of 4.\n");
}

void name_already_exists(const char* name) {
    write_to_log("Error: name '%s' already exists in table.\n", name);
}

void write_symbol(FILE* output, uint32_t addr, const char* name) {
    fprintf(output, "%u\t%s\n", addr, name);
}

/*******************************
 * Symbol Table Functions
 *******************************/

/* Creates a new SymbolTable containg 0 elements and returns a pointer to that
   table. 
   Multiple SymbolTables may exist at the same time. 
  
   If memory allocation fails, you should call allocation_failed(). 
   Mode will be either SYMTBL_NON_UNIQUE or SYMTBL_UNIQUE_NAME. 
   You will need to store this value for use during add_to_table().
 */
SymbolTable* create_table(int mode) {
    SymbolTable *table;
    table = (SymbolTable*) malloc(sizeof(SymbolTable));

    if (table == NULL) allocation_failed();
    
    table->mode = mode;
    table->len = 0;
    table->cap = 10; //initially, stores 10 symbols allowed
    table->tbl = (Symbol*) malloc((table->cap) * sizeof(Symbol));
    if (table->tbl == NULL) allocation_failed();
    return table;
}

/* Frees the given SymbolTable and all associated memory. */
void free_table(SymbolTable *table) {
    for(unsigned int i = 0; i < table->len; i++) {
         Symbol sym = table->tbl[i];
         free(sym.name);             
    }
    table->len = 0;
    free(table->tbl);
}

/* Adds a new symbol and its address to the SymbolTable pointed to by TABLE. 
   ADDR is given as the byte offset from the first instruction.
   The SymbolTable must be able to resize itself as more elements are added. 
   Note that NAME may point to a temporary array, so it is not safe to simply
   store the NAME pointer. You must store a copy of the given string.
   If ADDR is not word-aligned, you should call addr_alignment_incorrect() and
   return -1. 
   If the table's mode is SYMTBL_UNIQUE_NAME and NAME already exists 
   in the table, you should call name_already_exists() and return -1. If memory
   allocation fails, you should call allocation_failed(). 
   Otherwise, you should store the symbol name and address and return 0.
 */
int add_to_table(SymbolTable* table, const char* name, uint32_t addr) {
   
    //resize if needed 
    if ((table->len) >= (table->cap)) {
        table->cap *= 2;
        table->tbl = (Symbol*) realloc(table->tbl, ((table->cap) * sizeof(Symbol)));
        if (table->tbl == NULL) allocation_failed();
    }

    //word alignment check
    if ((addr % 4) != 0) {
        addr_alignment_incorrect();
        return -1;
    }
    
    //unique table check
    if (table->mode == SYMTBL_UNIQUE_NAME) {
        for (unsigned int i = 0; i < (table->len); i++) {
            Symbol sym = table->tbl[i];
            if (strcmp(name, sym.name) == 0) {
                name_already_exists(name);
                return -1;  
            }
        }
    }

    char* namecopy = (char*) malloc(sizeof(name));
    if (namecopy == NULL) allocation_failed();
    strcpy(namecopy, name);
     
    // add symbol to array 
    table->tbl[table->len].name = namecopy; 
    table->tbl[table->len].addr = addr;
    table->len += 1;

    return 0;
}

/* Returns the address (byte offset) of the given symbol. If a symbol with name
   NAME is not present in TABLE, return -1.
   Address look up.
 */
int64_t get_addr_for_symbol(SymbolTable* table, const char* name) {
    for (unsigned int i = 0; i < table->len; i++) {
        Symbol sym = table->tbl[i];
        if (strcmp(name, sym.name) == 0) {
            return sym.addr;
        }
    }
    return -1;
}

/* Writes the SymbolTable TABLE to OUTPUT. You should use write_symbol() to
   perform the write. Do not print any additional whitespace or characters.
 */
void write_table(SymbolTable* table, FILE* output) {
    for(unsigned int i = 0; i < table->len; i++) {
        Symbol sym = table->tbl[i];
        write_symbol(output, sym.addr, sym.name);       
    }
}
