
#ifndef MNAME_H
#define MNAME_H

#include <stdint.h>

#define M_NAME_RESPONSE_FIELD             0x8000

#define M_NAME_OP_FIELD                   0x7000
#define M_NAME_OP_STATUS                  0x1000

#define M_NAME_AUTH_ANSWER_FIELD          0x0400

#define M_NAME_TRUNC_FIELD                0x0200

#define M_NAME_RECURSE_REQ_FIELD          0x0100

#define M_NAME_RECURSE_AVAIL_FIELD        0x0080

#define M_NAME_RESPONSE_CODE_FIELD        0x000f

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

struct mname_msg {
   uint16_t id;
   uint16_t fields;
   uint16_t questions_len;
   uint16_t answers_len;
   uint16_t ns_len;
   uint16_t addl_len;
} __attribute__( (packed) );

#define m_name_is_response( pkt ) \
   ((pkt->fields) & M_NAME_RESPONSE_FIELD)

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

void mname_response( struct mname_msg* msg_in );

#endif /* MNAME_H */

