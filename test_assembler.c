#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <CUnit/Basic.h>

#include "src/utils.h"
#include "src/tables.h"
#include "src/translate_utils.h"
#include "src/translate.h"

const char* TMP_FILE = "test_output.txt";
const int BUF_SIZE = 1024;

/****************************************
 *  Helper functions 
 ****************************************/

int do_nothing() {
    return 0;
}

int init_log_file() {
    set_log_file(TMP_FILE);
    return 0;
}

int check_lines_equal(char **arr, int num) {
    char buf[BUF_SIZE];

    FILE *f = fopen(TMP_FILE, "r");
    if (!f) {
        CU_FAIL("Could not open temporary file");
        return 0;
    }
    for (int i = 0; i < num; i++) {
        if (!fgets(buf, BUF_SIZE, f)) {
            CU_FAIL("Reached end of file");
            return 0;
        }
        CU_ASSERT(!strncmp(buf, arr[i], strlen(arr[i])));
    }
    fclose(f);
    return 0;
}

/****************************************
 *  Test cases for translate_utils.c 
 ****************************************/

void test_translate_reg() {
    CU_ASSERT_EQUAL(translate_reg("$0"), 0);
    CU_ASSERT_EQUAL(translate_reg("$at"), 1);
    CU_ASSERT_EQUAL(translate_reg("$v0"), 2);
    CU_ASSERT_EQUAL(translate_reg("$a0"), 4);
    CU_ASSERT_EQUAL(translate_reg("$a1"), 5);
    CU_ASSERT_EQUAL(translate_reg("$a2"), 6);
    CU_ASSERT_EQUAL(translate_reg("$a3"), 7);
    CU_ASSERT_EQUAL(translate_reg("$t0"), 8);
    CU_ASSERT_EQUAL(translate_reg("$t1"), 9);
    CU_ASSERT_EQUAL(translate_reg("$t2"), 10);
    CU_ASSERT_EQUAL(translate_reg("$t3"), 11);
    CU_ASSERT_EQUAL(translate_reg("$s0"), 16);
    CU_ASSERT_EQUAL(translate_reg("$s1"), 17);
    CU_ASSERT_EQUAL(translate_reg("$3"), -1);
    CU_ASSERT_EQUAL(translate_reg("asdf"), -1);
    CU_ASSERT_EQUAL(translate_reg("hey there"), -1);
}

void test_translate_num() {
    long int output;

    CU_ASSERT_EQUAL(translate_num(&output, "35", -1000, 1000), 0);
    CU_ASSERT_EQUAL(output, 35);
    CU_ASSERT_EQUAL(translate_num(&output, "145634236", 0, 9000000000), 0);
    CU_ASSERT_EQUAL(output, 145634236);
    CU_ASSERT_EQUAL(translate_num(&output, "0xC0FFEE", -9000000000, 9000000000), 0);
    CU_ASSERT_EQUAL(output, 12648430);
    CU_ASSERT_EQUAL(translate_num(&output, "72", -16, 72), 0);
    CU_ASSERT_EQUAL(output, 72);
    CU_ASSERT_EQUAL(translate_num(&output, "72", -16, 71), -1);
    CU_ASSERT_EQUAL(translate_num(&output, "72", 72, 150), 0);
    CU_ASSERT_EQUAL(output, 72);
    CU_ASSERT_EQUAL(translate_num(&output, "72", 73, 150), -1);
    CU_ASSERT_EQUAL(translate_num(&output, "35x", -100, 100), -1);
    CU_ASSERT_EQUAL(translate_num(&output, "-10", -15, 5), 0);
    CU_ASSERT_EQUAL(output, -10);
    CU_ASSERT_EQUAL(translate_num(&output, "-125", -125, -125), 0);
    CU_ASSERT_EQUAL(output, -125);
    CU_ASSERT_EQUAL(translate_num(&output, "-125", -123, -123), -1);

}

/****************************************
 *  Test cases for tables.c 
 ****************************************/

void test_table_1() {
    int retval;

    SymbolTable* tbl = create_table(SYMTBL_UNIQUE_NAME);
    CU_ASSERT_PTR_NOT_NULL(tbl);

    retval = add_to_table(tbl, "abc", 8);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl, "efg", 12);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl, "q45", 16);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl, "q45", 24); 
    CU_ASSERT_EQUAL(retval, -1); 
    retval = add_to_table(tbl, "bob", 14); 
    CU_ASSERT_EQUAL(retval, -1); 

    retval = get_addr_for_symbol(tbl, "abc");
    CU_ASSERT_EQUAL(retval, 8); 
    retval = get_addr_for_symbol(tbl, "q45");
    CU_ASSERT_EQUAL(retval, 16); 
    retval = get_addr_for_symbol(tbl, "ef");
    CU_ASSERT_EQUAL(retval, -1); 
    
    free_table(tbl);
    

    char* arr[] = { "Error: name 'q45' already exists in table.",
                    "Error: address is not a multiple of 4." };
    check_lines_equal(arr, 2);

    SymbolTable* tbl2 = create_table(SYMTBL_NON_UNIQUE);
    CU_ASSERT_PTR_NOT_NULL(tbl2);

    retval = add_to_table(tbl2, "q45", 16);
    CU_ASSERT_EQUAL(retval, 0);
    retval = add_to_table(tbl2, "q45", 24); 
    CU_ASSERT_EQUAL(retval, 0);

    free_table(tbl2);
}

void test_table_2() {
    int retval, max = 100;

    SymbolTable* tbl = create_table(SYMTBL_UNIQUE_NAME);
    CU_ASSERT_PTR_NOT_NULL(tbl);

    char buf[10];
    for (int i = 0; i < max; i++) {
        sprintf(buf, "%d", i);
        retval = add_to_table(tbl, buf, 4 * i);
        CU_ASSERT_EQUAL(retval, 0);
    }

    for (int i = 0; i < max; i++) {
        sprintf(buf, "%d", i);
        retval = get_addr_for_symbol(tbl, buf);
        CU_ASSERT_EQUAL(retval, 4 * i);
    }

    free_table(tbl);
}


/****************************************
 * Test for step 3
 ****************************************/

void test_translate_inst() {
    int retval;

    // create args
    char *args[3]; // array of pointers to char arrays
    char functReg3[4] = "$a1"; // array of char
    args[0] = functReg3; // set first element of bad array to $s0
    char functReg[4] = "$a1";
    args[1] = functReg;
    char functReg2[4] = "$a0";
    args[2] = functReg2;

    uint32_t addr = 0;

    // create symtbl & reltbl
    SymbolTable* symtbl = create_table(0);
    SymbolTable* reltbl = create_table(0);


    // TEST WRITE_RTYPE, valid args, return 0
    FILE* testrtype = fopen("testrtype.txt", "w");
    const char *rtype = "addu"; //create good name
    retval = translate_inst(testrtype, rtype, args, 3, addr, symtbl, reltbl);
    CU_ASSERT_EQUAL(retval, 0);
    fclose(testrtype);




    // TEST WRITE_SHIFT
    char *testshift[3]; // array of pointers to char arrays
    char shiftReg3[4] = "$a1"; // array of char
    testshift[0] = shiftReg3; // set first element of bad array to $s0
    char shiftReg[4] = "$a1";
    testshift[1] = shiftReg;
    char shiftReg2[2] = "1";
    testshift[2] = shiftReg2;
    const char *shift = "sll"; //create good name

    // test write_shift, valid args, return 0
    FILE* goodshift = fopen("goodshift.txt", "w");
    retval = translate_inst(goodshift, shift, testshift, 3, addr, symtbl, reltbl);
    CU_ASSERT_EQUAL(retval, 0);
    fclose(goodshift);

    // test write_shift, invalid shiftamt, return -1
    char newshiftReg2[2] = "32";
    testshift[2] = newshiftReg2;
    FILE* badshift = fopen("badshift.txt", "w");
    retval = translate_inst(badshift, shift, testshift, 3, addr, symtbl, reltbl);
    CU_ASSERT_EQUAL(retval, -1);
    fclose(badshift);



    // TEST WRITE_JR
    char *testjr[1]; // array of pointers to char arrays
    char jrReg[4] = "$a1"; // array of char
    testjr[0] = jrReg; // set first element of bad array to $s0
    const char *jr = "jr"; //create good name
    FILE* goodjr = fopen("goodjr.txt", "w");
    retval = translate_inst(goodjr, jr, testjr, 1, addr, symtbl, reltbl);
    CU_ASSERT_EQUAL(retval, 0);
    fclose(goodjr);



    // TEST WRITE_ADDIU & WRITE_ITYPE & WRITE_ORI
    char *testaddiu[3]; // array of pointers to char arrays
    char addiuReg[4] = "$a1"; // array of char
    testaddiu[0] = addiuReg; // set first element of bad array to $s0
    char addiuReg2[4] = "$a1";
    testaddiu[1] = addiuReg2;
    char addiuReg3[6] = "10000";
    testaddiu[2] = addiuReg3;

    // addiu
    const char *addiu = "addiu"; //create good name
    FILE* goodaddiu = fopen("goodaddiu.txt", "w");
    retval = translate_inst(goodaddiu, addiu, testaddiu, 3, addr, symtbl, reltbl);
    CU_ASSERT_EQUAL(retval, 0);
    fclose(goodaddiu);

    // itype
    // const char *lb = "lb"; //create good name
    // FILE* goodlb = fopen("goodlb.txt", "w");
    // retval = translate_inst(goodlb, lb, testaddiu, 3, addr, symtbl, reltbl);
    // CU_ASSERT_EQUAL(retval, 0);
    // fclose(goodlb);

    // ori
    const char *ori = "ori"; //create good name
    FILE* goodori = fopen("goodori.txt", "w");
    retval = translate_inst(goodori, ori, testaddiu, 3, addr, symtbl, reltbl);
    CU_ASSERT_EQUAL(retval, 0);
    fclose(goodori);

    char addiuReg4[6] = "32768";
    testaddiu[2] = addiuReg4;

    //test bad itype (overflow)
    const char *lbu = "lbu"; //create good name
    FILE* goodlbu = fopen("goodlbu.txt", "w");
    retval = translate_inst(goodlbu, lbu, testaddiu, 3, addr, symtbl, reltbl);
    CU_ASSERT_EQUAL(retval, -1);
    fclose(goodlbu);


    // TEST WRITE_LUI
    char *testlui[2]; // array of pointers to char arrays
    char luireg[4] = "$a1"; // array of char
    testlui[0] = luireg; // set first element of bad array to $s0
    char luireg2[4] = "-11"; // array of char                      
    testlui[1] = luireg2; // set first element of bad array to $s0
    const char *lui = "lui"; //create good name
    FILE* goodlui = fopen("goodlui.txt", "w");
    retval = translate_inst(goodlui, lui, testlui, 2, addr, symtbl, reltbl);
    CU_ASSERT_EQUAL(retval, 0);
    fclose(goodlui);



    // TEST WRITE_MEM
    char *testmem[2]; // array of pointers to char arrays
    char memreg[4] = "$a1"; // array of char
    testmem[0] = memreg; // set first element of bad array to $s0
    char memreg2[3] = "11"; // array of char
    testmem[1] = memreg2; // set first element of bad array to $s0
    char memreg3[4] = "$a1"; // array of char
    testmem[2] = memreg3; // set first element of bad array to $s0
    const char *sb = "sb"; //create good name
    FILE* goodmem = fopen("goodmem.txt", "w");
    retval = translate_inst(goodmem, sb, testmem, 3, addr, symtbl, reltbl);
    CU_ASSERT_EQUAL(retval, 0);
    fclose(goodmem);


    // TEST WRITE_BRANCH
    const char *beq = "beq"; //create good name 
    char *testbeq[3]; // array of pointers to char arrays                       
    char beqreg[4] = "$a0"; // array of char
    testbeq[0] = beqreg; // set first element of bad array to $s0
    char beqreg2[4] = "$a1";
    testbeq[1] = beqreg2;
    char beqreg3[6] = "10000";
    testbeq[2] = beqreg3;

    FILE* goodbeq = fopen("goodbeq.txt", "w");
    retval = translate_inst(goodbeq, beq, testbeq, 3, addr, symtbl, reltbl);
    CU_ASSERT_EQUAL(retval, 0);
    
    retval = get_addr_for_symbol(symtbl, beq);
    CU_ASSERT_EQUAL(retval, 0);              
    fclose(goodbeq);


    // TEST WRITE_JUMP
    const char *jal = "jal"; //create good name
    char *testjump[1]; // array of pointers to char arrays
    char jumpreg[4] = "$ra"; // array of char
    testjump[0] = jumpreg; // set first element of bad array to $s0

    FILE* goodjal = fopen("goodjal.txt", "w");
    retval = translate_inst(goodjal, jal, testjump, 1, addr, symtbl, reltbl);
    CU_ASSERT_EQUAL(retval, 0);

    retval = get_addr_for_symbol(reltbl, jal);
    CU_ASSERT_EQUAL(retval, 0);               
    fclose(goodjal);



    // TEST BAD REGISTER; should return -1
    FILE* regs = fopen("regs.txt", "w");
    char *testReg[3]; // array of pointers to char arrays
    char badReg[5] = "$100"; // array of char
    testReg[0] = badReg; // set first element of bad array to $s0
    char goodReg[4] = "$a1";
    testReg[1] = goodReg;
    char goodReg2[4] = "$a0";
    testReg[2] = goodReg2;
    retval = translate_inst(regs, shift, testReg, 3, addr, symtbl, reltbl);
    CU_ASSERT_EQUAL(retval, -1);
    fclose(regs);

    // TEST BAD NAME, return -1
    FILE* bad = fopen("bad.txt", "w");
    const char *badargname = "auuu"; //create bad name
    retval = translate_inst(bad, badargname, args, 3, addr, symtbl, reltbl);
    CU_ASSERT_EQUAL(retval, -1);
    fclose(bad);

    // TEST BAD ARG ARRAY, return -1
    FILE* badargsfile = fopen("badargsfile.txt", "w");
    char *badArray[1]; // array of pointers to char arrays
    char badArg[4] = "$s0"; // array of char
    badArray[0] = badArg; // set first element of bad array to $s0
    retval = translate_inst(badargsfile, rtype, badArray, 3, addr, symtbl, reltbl);
    CU_ASSERT_EQUAL(retval, -1);
    fclose(badargsfile);

}


/****************************************
 * Test for step 4
 ****************************************/

void test_li_expansion() {
    int retval;
    char li[]  = "li";

    //error case: wrong num of args
    FILE* asdf = fopen("asdf.txt", "w");
    CU_ASSERT_PTR_NOT_NULL(asdf);
    char *badArray[1];
    char badArg[4] = "$s0";
    badArray[0] = badArg;
    retval = write_pass_one(asdf, li, badArray, 1);
    fclose(asdf);
    CU_ASSERT_EQUAL(retval, 0);

    //correct case: addiu
    FILE* asdf1 = fopen("asdf1.txt", "w");
    CU_ASSERT_PTR_NOT_NULL(asdf1);
    //create an array of pointers to strings
    char *array[2];
    char arg1[4] = "$t0";
    char arg2[6] = "65535";
    array[0] = arg1;
    array[1] = arg2;
    retval = write_pass_one(asdf1, li, array, 2);
    fclose(asdf1);
    CU_ASSERT_EQUAL(retval, 1);

    //correct case: lui ori
    FILE* asdf2 = fopen("asdf2.txt", "w");
    CU_ASSERT_PTR_NOT_NULL(asdf2);
    char *array2[2];
    char arg3[4] = "$t0";
    char arg4[8] = "0x3BF20";
    array2[0] = arg3;
    array2[1] = arg4;
    retval = write_pass_one(asdf2, li, array2, 2);
    fclose(asdf2);
    CU_ASSERT_EQUAL(retval, 2);

    //correct case: deadbeef
    FILE* asdf3 = fopen("asdf3.txt", "w");
    CU_ASSERT_PTR_NOT_NULL(asdf3);
    char *array3[2];
    char arg5[4] = "$t0";
    char arg6[11] = "0xDEADBEEF";
    array3[0] = arg5;
    array3[1] = arg6;
    retval = write_pass_one(asdf3, li, array3, 2);
    fclose(asdf3);
    CU_ASSERT_EQUAL(retval, 2); 
}

void test_blt_expansion() {
  int retval;
  char blt[] = "blt";

  //incorrect case: not enough arguments
  FILE* asdf = fopen("asdf.txt", "w");
  CU_ASSERT_PTR_NOT_NULL(asdf);
  char *array[3];
  char arg1[4] = "$8";
  char arg2[4] = "$9";
  array[0] = arg1;
  array[1] = arg2;
  array[2] = NULL;
  retval = write_pass_one(asdf, blt, array, 1);
  fclose(asdf);
  CU_ASSERT_EQUAL(retval, 0);

  //correct case
  FILE* asdf2= fopen("asdf2.txt", "w");
  CU_ASSERT_PTR_NOT_NULL(asdf2);
  char *array2[3];
  char arg3[4] = "$8";
  char arg4[4] = "$9";
  char arg5[10] = "label";
  array2[0] = arg3;
  array2[1] = arg4;
  array2[2] = arg5;

  retval = write_pass_one(asdf2, blt, array2, 3); 
  fclose(asdf2);
  CU_ASSERT_EQUAL(retval, 2);
}

int main(int argc, char** argv) {
    CU_pSuite pSuite1 = NULL, pSuite2 = NULL, pSuite3 = NULL, pSuite4 = NULL;

    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    /* Suite 1 */
    pSuite1 = CU_add_suite("Testing translate_utils.c", NULL, NULL);
    if (!pSuite1) {
        goto exit;
    }
    if (!CU_add_test(pSuite1, "test_translate_reg", test_translate_reg)) {
        goto exit;
    }
    if (!CU_add_test(pSuite1, "test_translate_num", test_translate_num)) {
        goto exit;
    }

    /* Suite 2 */
    pSuite2 = CU_add_suite("Testing tables.c", init_log_file, NULL);
    if (!pSuite2) {
        goto exit;
    }
    if (!CU_add_test(pSuite2, "test_table_1", test_table_1)) {
        goto exit;
    }
    if (!CU_add_test(pSuite2, "test_table_2", test_table_2)) {
        goto exit;
    }

    /* Suite 3 */
    pSuite3 = CU_add_suite("Testing li and blt expansion", NULL, NULL);
    if (!pSuite3) {
      goto exit;
    }
    if (!CU_add_test(pSuite3, "test_li_expansion", test_li_expansion)) {
        goto exit;
    }   
    if (!CU_add_test(pSuite3, "test_blt_expansion", test_blt_expansion)) {
        goto exit;
    }

    /* Suite 4 */
    pSuite4 = CU_add_suite("Testing step 3", NULL, NULL);
    if (!pSuite4) {
      goto exit;
    }
    if (!CU_add_test(pSuite4, "test_translate_inst", test_translate_inst)) {
        goto exit;
    }   

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

exit:
    CU_cleanup_registry();
    return CU_get_error();;
}
