
#include <check.h>
#include "../src/mname.h"
#include "dnspkt.h"

#include <stdio.h>

#define NAME_BUF_SZ 512
#define RDATA_BUF_SZ 512

struct mname_msg* dns_pkt = (struct mname_msg*)dnspkt_bin;

#define TEST_RDATA_SZ
uint8_t test_rdata[TEST_RDATA_SZ] = { 0x00, 0x0a, 0x00, 0x08, 0xf5, 0x0e,
   0x49, 0x53, 0x8d, 0xa3, 0x48, 0xa5 };

START_TEST( test_m_htons ) {
   uint8_t test_buf[2] = { 0x00, 0x01 };
   ck_assert_int_eq( 1, m_htons( *((uint16_t*)test_buf)) );
}
END_TEST

START_TEST( test_m_htonl ) {
   uint8_t test_buf[4] = { 0x12, 0x34, 0x56, 0x78 };
   ck_assert_int_eq( 305419896, m_htonl( *((uint32_t*)test_buf) ) );
}
END_TEST

START_TEST( test_q_domain_len ) {
   ck_assert_int_eq( 12, mname_get_domain_len( dns_pkt, dnspkt_bin_len, 0 ) );
}
END_TEST

START_TEST( test_q_domain ) {
   char domain[NAME_BUF_SZ] = { 0 };
   int len = 0;
   len = mname_get_domain( dns_pkt, dnspkt_bin_len, 0, domain, NAME_BUF_SZ );
   ck_assert_str_eq( domain, "google.com." );
   ck_assert_int_eq( len, 12 );
}
END_TEST

START_TEST( test_q_class ) {
   ck_assert_int_eq( 1, mname_get_class( dns_pkt, dnspkt_bin_len, 0 ) );
}
END_TEST

START_TEST( test_q_type ) {
   ck_assert_int_eq( 1, mname_get_type( dns_pkt, dnspkt_bin_len, 0 ) );
}
END_TEST

START_TEST( test_a_class ) {
   ck_assert_int_eq( 4096, mname_get_class( dns_pkt, dnspkt_bin_len, 1 ) );
}
END_TEST

START_TEST( test_a_type ) {
   ck_assert_int_eq( 41, mname_get_type( dns_pkt, dnspkt_bin_len, 1 ) );
}
END_TEST

START_TEST( test_a_ttl ) {
   ck_assert_int_eq( 0, mname_get_a_ttl( dns_pkt, dnspkt_bin_len, 1 ) );
}
END_TEST

START_TEST( test_a_rdata_len ) {
   ck_assert_int_eq( 12, mname_get_a_rdata_len( dns_pkt, dnspkt_bin_len, 1 ) );
}
END_TEST

START_TEST( test_sz ) {
   ck_assert_int_eq(
      dnspkt_bin_len, mname_get_msg_len( dns_pkt, dnspkt_bin_len ) );
}
END_TEST

Suite* mname_suite( void ) {
   Suite* s = NULL;
   TCase* tc_respond = NULL;

   s = suite_create( "mname" );

   tc_respond = tcase_create( "Respond" );

   tcase_add_test( tc_respond, test_m_htons );
   tcase_add_test( tc_respond, test_m_htonl );
   tcase_add_test( tc_respond, test_q_domain_len );
   tcase_add_test( tc_respond, test_q_domain );
   tcase_add_test( tc_respond, test_q_type );
   tcase_add_test( tc_respond, test_q_class );
   tcase_add_test( tc_respond, test_a_type );
   tcase_add_test( tc_respond, test_a_class );
   tcase_add_test( tc_respond, test_a_ttl );
   tcase_add_test( tc_respond, test_a_rdata_len );
   tcase_add_test( tc_respond, test_sz );

   suite_add_tcase( s, tc_respond );

   return s;
}

