
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


// Implement a data structure to store symbol name-to-address mappings in src/tables.c. 
// Multiple SymbolTables may be created at the same time, and each must resize to fit 
// an arbitrary number of entries (so you should use dynamic memory allocation). You 
// may design the data structure in any way you like, as long as you do not change the 
// function definitions. A SymbolTable struct has been defined in src/tables.h, and you 
// may use the existing implementation or create your own if that feels more intuitive. 
// Feel free to declare additional helper methods. See src/tables.c for details.

// You must make sure to free all memory that you allocate. See the Valgrind section 
// under testing for more information.


/* Creates a new SymbolTable containg 0 elements and returns a pointer to that
   table. Multiple SymbolTables may exist at the same time. 
   If memory allocation fails, you should call allocation_failed(). 
   Mode will be either SYMTBL_NON_UNIQUE or SYMTBL_UNIQUE_NAME. You will need
   to store this value for use during add_to_table().
 */
SymbolTable* create_table(int mode) {
    /* YOUR CODE HERE */

    SymbolTable *table; // create new SymbolTable with 0 elements
    table = (SymbolTable*) malloc(sizeof(table));

    if (table == NULL) { // if memory not available
        allocation_failed();
    }

    table->mode = mode; // store whether table unique or nonunique
    return table; // return pointer to table

}

/* Frees the given SymbolTable and all associated memory. */
void free_table(SymbolTable* table) {
    /* YOUR CODE HERE */

    if(table) { // if pointer doesn't point to NULL
      free_table((*table)->tbl); // recursively free each element in linked list
    }

    free(table); // free pointer to table
}

/* Adds a new symbol and its address to the SymbolTable pointed to by TABLE. 
   ADDR is given as the byte offset from the first instruction. The SymbolTable
   must be able to resize itself as more elements are added. 

   Note that NAME may point to a temporary array, so it is not safe to simply
   store the NAME pointer. You must store a copy of the given string.

   If ADDR is not word-aligned, you should call addr_alignment_incorrect() and
   return -1. If the table's mode is SYMTBL_UNIQUE_NAME and NAME already exists 
   in the table, you should call name_already_exists() and return -1. If memory
   allocation fails, you should call allocation_failed(). 

   Otherwise, you should store the symbol name and address and return 0.
 */

int add_to_table(SymbolTable* table, const char* name, uint32_t addr) {
   
    uint32_t space = table->cap;
    table->tbl = (Symbol*) malloc(space * sizeof(Symbol)); // add more space

    if (table->tbl == NULL) {
        allocation_failed();
    }

    if (addr%4 != 0) { // if addr not word-aligned 
        addr_alignment_incorrect();
        return -1;
    }

    // see if name exists in table
    int nameexists = 0;
    for (int i=0; i < table.len; i++) {
        if (strcmp(name, table.tbl[i]) == 0) { // compare each element in array
            nameexists = 1;
        }
    }

    if (table->mode = SYMTBL_UNIQUE_NAME && nameexists) {
        name_already_exists(name);
        return -1;
    }

    Symbol* newsymbol; //initialize new symbol
    strcpy(newsymbol, name); //store copy of NAME string
    newsymbol.addr = addr; // store address

    // add symbol to end of array
    table.tbl[len] = newsymbol;
    
    return 0;
}

/* Returns the address (byte offset) of the given symbol. If a symbol with name
   NAME is not present in TABLE, return -1.
 */
int64_t get_addr_for_symbol(SymbolTable* table, const char* name) {
    /* YOUR CODE HERE */

    for (int i=0; i < table.len; i++) {
        if (strcmp(name, table.tbl[i]) == 0) {  //if symbol is present
            return table.tbl[i].addr;  // return address of symbol
        }
    }
    return -1;
}

/* Writes the SymbolTable TABLE to OUTPUT. You should use write_symbol() to
   perform the write. Do not print any additional whitespace or characters.
 */
void write_table(SymbolTable* table, FILE* output) {
    /* YOUR CODE HERE */

    write_symbol(output, addr, name); // for each symbol in table

}
