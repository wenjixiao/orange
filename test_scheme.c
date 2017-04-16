#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <math.h>
#include <ctype.h>
#include <check.h>

#include "object.h"

Object* num(int i){
    return new_integer(i);
}

void test_print(){
    Object* n1 = num(1);
    Object* n2 = num(2);
    Object* n3 = num(3);
    Object* n4 = num(4);
    Object* n5 = num(5);
    Object* n6 = num(6);

    Object* c12 = cons(n1,n2);
    Object* c34 = cons(n3,n4);
    Object* cc = cons(c12,c34);

    Object* l1 = list4(n1,n2,n3,n4);
    printf("---------------\n");
    printObject(l1);
    printf("\n");
    printObject1(l1);
    printf("\n");
    printf("---------------\n");
    printObject(cc);
    printf("\n");
    printObject1(cc);
    printf("\n");
}

START_TEST (test_add){
    //ck_assert_int_eq(3,5);
    //ck_assert_msg(3==5,"3==5?");
    //fail_unless(3 == 2,"error,3!=2");
	
}
END_TEST

Suite *make_scheme_suite(void){
    Suite *s = suite_create("Scheme");
    TCase *tc_basic = tcase_create("basic");
    suite_add_tcase(s,tc_basic);
    tcase_add_test(tc_basic,test_add);
    return s;
}

int main(){
    int n;
    SRunner *sr;
    sr = srunner_create(make_scheme_suite());
    srunner_run_all(sr,CK_NORMAL);
    n = srunner_ntests_failed(sr);
    srunner_free(sr);
    return n == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
