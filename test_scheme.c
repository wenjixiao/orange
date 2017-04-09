#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <math.h>
#include <ctype.h>
#include <check.h>


START_TEST (test_add){
    ck_assert_int_eq(3,5);
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
