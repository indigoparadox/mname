
#ifndef MNAME_H
#define MNAME_H

#include <stdint.h>
#include <stddef.h>

#ifndef m_htons
/* TODO: Only have this work when we're on a CPU that needs it. */
#define m_htons( n ) (((n >> 8) & 0x00ff) | ((n << 8) & 0xff00))
#endif /* m_htons */

#ifndef m_htonl
#define m_htonl( n ) ((((uint32_t)(n) & 0xff000000) >> 24) | \
   (((uint32_t)(n) & 0x00ff0000) >> 8) | \
   (((uint32_t)(n) & 0x0000ff00) << 8) | \
   (((uint32_t)(n) & 0x000000ff) << 24))
#endif /* m_htonl */

/* TODO: Define these here in native order and use m_htons in code. */
#define M_NAME_RESPONSE_FIELD             0x0080

#define M_NAME_OP_FIELD                   0x0070
#define M_NAME_OP_STATUS                  0x0010

#define M_NAME_AUTH_ANSWER_FIELD          0x0004

#define M_NAME_TRUNC_FIELD                0x0002

#define M_NAME_RECURSE_REQ_FIELD          0x0001

#define M_NAME_RECURSE_AVAIL_FIELD        0x0000

#define M_NAME_RESPONSE_CODE_FIELD        0x0f00

/* Below here are already in native order! */

#define M_NAME_RESPONSE_CODE_NONE         0x000
#define M_NAME_RESPONSE_CODE_FORMAT       0x001
#define M_NAME_RESPONSE_CODE_SERVER       0x002
#define M_NAME_RESPONSE_CODE_NAME         0x003
#define M_NAME_RESPONSE_CODE_REFUSED      0x005
#define M_NAME_RESPONSE_CODE_YX_DOMAIN    0x006
#define M_NAME_RESPONSE_CODE_YX_RR_SET    0x007
#define M_NAME_RESPONSE_CODE_NX_RR_SET    0x008
#define M_NAME_RESPONSE_CODE_NOT_AUTH     0x009
#define M_NAME_RESPONSE_CODE_NOT_ZONE     0x010

#define M_NAME_TYPE_A                     0x0001
#define M_NAME_TYPE_MX                    0x000f
#define M_NAME_TYPE_NS                    0x0002
#define M_NAME_TYPE_OPT                   0x0041

#define M_NAME_WIDTH_CLASS                2
#define M_NAME_WIDTH_TYPE                 2
#define M_NAME_WIDTH_TTL                  4
#define M_NAME_WIDTH_DOMAIN_SZ            1
#define M_NAME_WIDTH_RDATA_SZ             2

#define M_NAME_WIDTH_Q( msg, buf_sz ) \
   (mname_get_domain_len( msg, buf_sz, 0 ) + \
   M_NAME_WIDTH_TYPE + M_NAME_WIDTH_CLASS)

#define M_NAME_WIDTH_A( msg, buf_sz, idx ) \
   (mname_get_domain_len( msg, buf_sz, idx ) + \
   M_NAME_WIDTH_TYPE + M_NAME_WIDTH_CLASS + M_NAME_WIDTH_TTL + \
   mname_get_a_rdata_len( msg, buf_sz, idx ) + M_NAME_WIDTH_RDATA_SZ)

struct mname_msg {
   uint16_t id;
   uint16_t fields;
   uint16_t q_count;
   uint16_t a_count;
   uint16_t n_count;
   uint16_t l_count;
} __attribute__( (packed) );

/*
struct mname_question {
   uint16_t 
} __attribute__( (packed) );

struct mname_answer {
   uint16_t 
} __attribute__( (packed) );
*/

#define m_name_is_response( pkt ) \
   ((pkt->fields) & M_NAME_RESPONSE_FIELD)

#define m_name_set_response( pkt ) \
   (pkt->fields |= M_NAME_RESPONSE_FIELD)

#define m_name_is_op_status( pkt ) \
   ((pkt->fields) & M_NAME_OP_STATUS)

#define m_name_is_authoritative( pkt ) \
   ((pkt->fields) & M_NAME_AUTH_ANSWER_FIELD)

#define m_name_is_truncated( pkt ) \
   ((pkt->fields) & M_NAME_TRUNC_FIELD)

#define m_name_is_recursion_req( pkt ) \
   ((pkt->fields) & M_NAME_RECURSE_REQ_FIELD)

#define m_name_is_recursion_avail( pkt ) \
   ((pkt->fields) & M_NAME_RECURSE_AVAIL_FIELD)

#define _mname_cast_ptr_to_bytes( ptr ) \
   ((uint8_t*)ptr)

#define _mname_cast_ptr_to_short( ptr ) \
   ((uint16_t*)ptr)

#define _mname_cast_ptr_to_long( ptr ) \
   ((uint32_t*)(ptr))

#define mname_get_records_count( msg_in ) \
   (m_htons( msg_in->q_count ) + m_htons( msg_in->a_count ) + \
      m_htons( msg_in->n_count ) + m_htons( msg_in->l_count ) )

#define mname_get_type( msg_in, msg_buf_sz, idx ) \
   m_htons( *(_mname_cast_ptr_to_short( \
      (_mname_cast_ptr_to_bytes( msg_in ) + \
         mname_get_offset( msg_in, msg_buf_sz, idx ) + \
         mname_get_domain_len( msg_in, msg_buf_sz, idx ) ))) )

#define mname_get_class( msg_in, msg_buf_sz, idx ) \
   m_htons( *(_mname_cast_ptr_to_short( \
      (_mname_cast_ptr_to_bytes( msg_in ) + \
         mname_get_offset( msg_in, msg_buf_sz, idx ) + \
         mname_get_domain_len( msg_in, msg_buf_sz, idx ) + \
         M_NAME_WIDTH_TYPE))) )

#define mname_get_a_ttl( msg_in, msg_buf_sz, idx ) \
   m_htonl( *(_mname_cast_ptr_to_long( \
      (_mname_cast_ptr_to_bytes( msg_in ) + \
         mname_get_offset( msg_in, msg_buf_sz, idx ) + \
         mname_get_domain_len( msg_in, msg_buf_sz, idx ) + \
         M_NAME_WIDTH_TYPE + M_NAME_WIDTH_CLASS))) )

#define mname_get_msg_len( msg_in, msg_buf_sz ) \
   mname_get_offset( msg_in, msg_buf_sz, mname_get_records_count( msg_in ) )

int mname_get_domain_len(
   const struct mname_msg* msg_in, size_t msg_buf_sz, uint16_t idx );
int mname_get_domain(
   const struct mname_msg* msg_in, size_t msg_buf_sz, uint16_t idx,
   char* buf, size_t buf_len );
int mname_get_a_rdata_len(
   const struct mname_msg* msg_in, size_t msg_buf_sz, uint16_t idx );
int mname_get_a_rdata(
   const struct mname_msg* msg_in, size_t msg_buf_sz,  uint16_t idx,
   uint8_t* buf, size_t buf_len );
int mname_get_offset( const struct mname_msg* msg_in, size_t msg_buf_sz,
   uint16_t idx );
int mname_add_answer( struct mname_msg* msg, size_t msg_buf_sz,
   char* domain, uint8_t domain_len,
   uint16_t type, uint16_t class, uint32_t ttl,
   char* rdata, uint16_t rdata_len );

#endif /* MNAME_H */

