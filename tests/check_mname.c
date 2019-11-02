
#include <check.h>
#include "../src/mname.h"

static void setup_mname() {
}

static void teardown_mname() {
}

START_TEST( test_sz ) {
}
END_TEST

Suite* mname_suite( void ) {
   Suite* s = NULL;
   TCase* tc_respond = NULL;

   s = suite_create( "mname" );

   tc_respond = tcase_create( "Respond" );
   tcase_add_checked_fixture( tc_respond, setup_mname, teardown_mname );

   tcase_add_test( tc_respond, test_sz );

   suite_add_tcase( s, tc_respond );

   return s;
}

