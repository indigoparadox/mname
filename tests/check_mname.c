
#include <check.h>
#include "../src/mname.h"
#include "../src/mpktdmp.h"
#include "dnspkt.h"

#include <stdio.h>

#define NAME_BUF_SZ 512
#define RDATA_BUF_SZ 512
#define RESPONSE_BUF_SZ 512

#define TEST_A_RDATA "1.1.1.1"
#define TEST_A_RDATA_SZ (sizeof( TEST_A_RDATA ) - 1)

struct mname_msg* dns_pkt = NULL;
unsigned int dns_pkt_len = 0;

uint8_t dns_response_buffer[RESPONSE_BUF_SZ] = { 0 };

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

/* Test Question */

START_TEST( test_q_domain_len ) {
   ck_assert_int_eq( 12, mname_get_domain_len( dns_pkt, dns_pkt_len, 0 ) );
}
END_TEST

START_TEST( test_q_domain ) {
   char domain[NAME_BUF_SZ] = { 0 };
   int len = 0;
   len = mname_get_domain( dns_pkt, dns_pkt_len, 0, domain, NAME_BUF_SZ );
   ck_assert_str_eq( domain, "google.com." );
   ck_assert_int_eq( len, 12 );
}
END_TEST

START_TEST( test_q_class ) {
   ck_assert_int_eq( 1, mname_get_class( dns_pkt, dns_pkt_len, 0 ) );
}
END_TEST

START_TEST( test_q_type ) {
   ck_assert_int_eq( 1, mname_get_type( dns_pkt, dns_pkt_len, 0 ) );
}
END_TEST

/* Test Answer */

START_TEST( test_a_domain ) {
   char domain[NAME_BUF_SZ] = { 0 };
   int len = 0;
   len = mname_get_domain( dns_pkt, dns_pkt_len, _i, domain, NAME_BUF_SZ );
   ck_assert_str_eq( domain, "google.com." );
   ck_assert_int_eq( len, 12 );
}
END_TEST

START_TEST( test_a_class ) {
   ck_assert_int_eq( 1, mname_get_class( dns_pkt, dns_pkt_len, _i ) );
}
END_TEST

START_TEST( test_a_type ) {
   ck_assert_int_eq( 1, mname_get_type( dns_pkt, dns_pkt_len, _i ) );
}
END_TEST

START_TEST( test_a_ttl ) {
   ck_assert_int_eq( 0, mname_get_a_ttl( dns_pkt, dns_pkt_len, _i ) );
}
END_TEST

START_TEST( test_a_rdata_len ) {
   ck_assert_int_eq(
      TEST_A_RDATA_SZ, mname_get_a_rdata_len( dns_pkt, dns_pkt_len, _i ) );
}
END_TEST

/* Test Additional */

START_TEST( test_l_class ) {
   ck_assert_int_eq( 4096, mname_get_class( dns_pkt, dns_pkt_len, _i ) );
}
END_TEST

START_TEST( test_l_type ) {
   ck_assert_int_eq( 41, mname_get_type( dns_pkt, dns_pkt_len, _i ) );
}
END_TEST

START_TEST( test_l_ttl ) {
   ck_assert_int_eq( 0, mname_get_a_ttl( dns_pkt, dns_pkt_len, _i ) );
}
END_TEST

START_TEST( test_l_rdata_len ) {
   ck_assert_int_eq( 12, mname_get_a_rdata_len( dns_pkt, dns_pkt_len, _i ) );
}
END_TEST

START_TEST( test_sz ) {
   ck_assert_int_eq(
      dns_pkt_len, mname_get_msg_len( dns_pkt, dns_pkt_len ) );
}
END_TEST

static void setup_pkt_sample() {
   dns_pkt = (struct mname_msg*)dnspkt_bin;
   dns_pkt_len = dnspkt_bin_len;
}

static void teardown_pkt_sample() {
}

static void setup_pkt_response() {
   char domain_name[NAME_BUF_SZ] = { 0 };
   size_t domain_name_len = 0;

   memcpy( dns_response_buffer, dnspkt_bin, dnspkt_bin_len );
   dns_pkt = (struct mname_msg*)dns_response_buffer;
   dns_pkt_len = dnspkt_bin_len;

   /* Change the message to a response. */
   m_name_set_response( dns_pkt );
   memset( domain_name, '\0', NAME_BUF_SZ );
   domain_name_len = mname_get_domain(
      dns_pkt, RESPONSE_BUF_SZ, 0, domain_name, NAME_BUF_SZ );
   mname_add_answer( dns_pkt, RESPONSE_BUF_SZ, domain_name,
      domain_name_len,
      mname_get_type( dns_pkt, RESPONSE_BUF_SZ, 0 ),
      mname_get_class( dns_pkt, RESPONSE_BUF_SZ, 0 ),
      0, TEST_A_RDATA, TEST_A_RDATA_SZ );

   pkt_dump_file( "apkt.bin", dns_response_buffer, RESPONSE_BUF_SZ );
}

static void teardown_response() {
}

Suite* mname_suite( void ) {
   Suite* s = NULL;
   TCase* tc_sample = NULL;
   TCase* tc_response = NULL;

   s = suite_create( "mname" );

   tc_sample = tcase_create( "Sample" );
   tc_response = tcase_create( "Response" );
   tcase_add_checked_fixture(
      tc_sample, setup_pkt_sample, teardown_pkt_sample );
   tcase_add_checked_fixture(
      tc_response, setup_pkt_response, teardown_response );

   /* Test the parsing of the incoming packet. */
   tcase_add_test( tc_sample, test_m_htons );
   tcase_add_test( tc_sample, test_m_htonl );
   tcase_add_test( tc_sample, test_q_domain_len );
   tcase_add_test( tc_sample, test_q_domain );
   tcase_add_test( tc_sample, test_q_type );
   tcase_add_test( tc_sample, test_q_class );
   tcase_add_loop_test( tc_sample, test_l_type,          1, 2 );
   tcase_add_loop_test( tc_sample, test_l_class,         1, 2 );
   tcase_add_loop_test( tc_sample, test_l_ttl,           1, 2 );
   tcase_add_loop_test( tc_sample, test_l_rdata_len,     1, 2 );
   tcase_add_test( tc_sample, test_sz );

   /* Test our crafted response. */
   tcase_add_test( tc_response, test_m_htons );
   tcase_add_test( tc_response, test_m_htonl );
   tcase_add_test( tc_response, test_q_domain_len );
   tcase_add_test( tc_response, test_q_domain );
   tcase_add_test( tc_response, test_q_type );
   tcase_add_test( tc_response, test_q_class );
   tcase_add_loop_test( tc_response, test_a_domain,      1, 2 );
   tcase_add_loop_test( tc_response, test_a_type,        1, 2 );
   tcase_add_loop_test( tc_response, test_a_class,       1, 2 );
   tcase_add_loop_test( tc_response, test_a_ttl,         1, 2 );
   tcase_add_loop_test( tc_response, test_a_rdata_len,   1, 2 );
   tcase_add_loop_test( tc_response, test_l_type,        2, 3 );
   tcase_add_loop_test( tc_response, test_l_class,       2, 3 );
   tcase_add_loop_test( tc_response, test_l_ttl,         2, 3 );
   tcase_add_loop_test( tc_response, test_l_rdata_len,   2, 3 );
   tcase_add_test( tc_response, test_sz );

   suite_add_tcase( s, tc_sample );
   suite_add_tcase( s, tc_response );

   return s;
}

