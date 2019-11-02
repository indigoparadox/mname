
#include <check.h>
#include "../src/mname.h"

#include <stdio.h>

#define NAME_BUF_SZ 512
#define RDATA_BUF_SZ 512
#define PKT_BUF_SZ 512

#define PKT_LEN 51

uint8_t pkt_buffer[PKT_BUF_SZ] = { 0 };
struct mname_msg* dns_pkt = (struct mname_msg*)pkt_buffer;

static void setup_mname() {
   FILE* pkt_file = NULL;
   pkt_file = fopen( "dnspkt.bin", "rb" );
   fread( pkt_buffer, 1, PKT_BUF_SZ, pkt_file );
   fclose( pkt_file );
}

static void teardown_mname() {
}

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

START_TEST( test_q_class ) {
   ck_assert_int_eq( 1, mname_get_class( dns_pkt, 0 ) );
}
END_TEST

START_TEST( test_q_type ) {
   ck_assert_int_eq( 1, mname_get_type( dns_pkt, 0 ) );
}
END_TEST

START_TEST( test_a_class ) {
   ck_assert_int_eq( 4096, mname_get_class( dns_pkt, 1 ) );
}
END_TEST

START_TEST( test_a_type ) {
   ck_assert_int_eq( 41, mname_get_type( dns_pkt, 1 ) );
}
END_TEST

START_TEST( test_a_ttl ) {
   ck_assert_int_eq( 0, mname_get_a_ttl( dns_pkt, 1 ) );
}
END_TEST

START_TEST( test_a_rdata_len ) {
   ck_assert_int_eq( 12, mname_get_a_rdata_len( dns_pkt, 1 ) );
}
END_TEST

START_TEST( test_sz ) {
   ck_assert_int_eq( PKT_LEN, mname_get_offset( dns_pkt, 2 ) );
}
END_TEST

Suite* mname_suite( void ) {
   Suite* s = NULL;
   TCase* tc_respond = NULL;

   s = suite_create( "mname" );

   tc_respond = tcase_create( "Respond" );
   tcase_add_checked_fixture( tc_respond, setup_mname, teardown_mname );

   tcase_add_test( tc_respond, test_m_htons );
   tcase_add_test( tc_respond, test_m_htonl );
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

