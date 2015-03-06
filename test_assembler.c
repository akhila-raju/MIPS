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
 * Test for step 4
 ****************************************/

void test_li_expansion() {
    int retval;

    char li[]  = "li";

    //error case: wrong num of args
    FILE* asdf = fopen("asdf.txt", "w");
    CU_ASSERT_PTR_NOT_NULL(asdf);
    char *badArray[1][20];
    char badArg[4] = "$s0";
    badArray[0][0] = badArg;
    retval = write_pass_one(asdf, li, badArray, 1);
    fclose(asdf);
    CU_ASSERT_EQUAL(retval, 0);

    //correct case: addiu
    FILE* asdf1 = fopen("asdf1.txt", "w");
    CU_ASSERT_PTR_NOT_NULL(asdf1);
    char *array[2][20];
    char arg1[4] = "$t0";
    char arg2[6] = "65535";
    array[0][0] = arg1;
    array[0][1] = arg2;
    retval = write_pass_one(asdf1, li, array, 2);
    fclose(asdf1);
    CU_ASSERT_EQUAL(retval, 1);

    //correct case: lui ori
    FILE* asdf2 = fopen("asdf2.txt", "w");
    CU_ASSERT_PTR_NOT_NULL(asdf2);
    char *array2[2][30];
    char arg3[4] = "$t0";
    char arg4[20] = "0x3BF20";
    array2[0][0] = arg3;
    array2[0][1] = arg4;
    retval = write_pass_one(asdf2, li, array2, 2);
    fclose(asdf2);
    CU_ASSERT_EQUAL(retval, 2);

    //correct case: deadbeef
    FILE* asdf3 = fopen("asdf3.txt", "w");
    CU_ASSERT_PTR_NOT_NULL(asdf3);
    char *array3[2][30];
    char arg5[4] = "$t0";
    char arg6[20] = "0xDEADBEEF";
    array3[0][0] = arg5;
    array3[0][1] = arg6;
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
  char *array[3][20];
  char arg1[4] = "$8";
  char arg2[4] = "$9";
  array[0][0] = arg1;
  array[0][1] = arg2;
  retval = write_pass_one(asdf, blt, array, 1);
  fclose(asdf);
  CU_ASSERT_EQUAL(retval, 0);

  //correct case
  FILE* asdf2= fopen("asdf2.txt", "w");
  CU_ASSERT_PTR_NOT_NULL(asdf2);
  char *array2[3][20];
  char arg3[4] = "$8";
  char arg4[4] = "$9";
  char arg5[10] = "label";
  array2[0][0] = arg3;
  array2[0][1] = arg4;
  array2[0][2] = arg5;

  retval = write_pass_one(asdf2, blt, array2, 3); 
  fclose(asdf2);
  CU_ASSERT_EQUAL(retval, 2);
}

int main(int argc, char** argv) {
    CU_pSuite pSuite1 = NULL, pSuite2 = NULL, pSuite3 = NULL;

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


    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

exit:
    CU_cleanup_registry();
    return CU_get_error();;
}
