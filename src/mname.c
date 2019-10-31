
#include "mname.h"

void mname_response( struct mname_msg* msg_in ) {
   msg_in->fields |= M_NAME_RESPONSE_FIELD;

   msg_in->answers_len = m_htons( 1 );
}

int mname_get_q_domain_len( struct mname_msg* msg_in, uint16_t idx ) {
   uint8_t* ptr = (uint8_t*)msg_in;
   int len_out = 0;

   if( idx > msg_in->questions_len ) {
      return 0;
   }

   /* TODO: Move to the requested index. */

   /* Much quicker and simpler version of the loop from mname_get_q_domain(). */
   ptr += sizeof( struct mname_msg );
   do {
      len_out += *ptr + 1; /* +1 for sz octet. */
      ptr += *ptr + 1;
   } while( 0 != *ptr );

   return len_out;
}

int mname_get_q_domain(
   struct mname_msg* msg_in, uint16_t idx, char* buf, size_t buf_len
) {
   uint8_t* ptr = (uint8_t*)msg_in;
   uint16_t len = 0;
   size_t buf_idx = 0;

   if( idx > msg_in->questions_len ) {
      return 0;
   }

   /* TODO: Move to the requested index. */

   ptr += sizeof( struct mname_msg );

   /* Grab the name into the buffer one segment at a time. */
   do {
      /* Grab the name component length and move up by 2 bytes (16 bits). */
      len = *ptr;
      ptr++;

      /* Grab the name segment into the buffer one char at a time. */
      while( 0 < len && buf_idx + 1 < buf_len ) { /* +1 for . */
         buf[buf_idx++] = (char)*ptr;
         ptr++;
         len--; /* 1 less to go. */
      }
      buf[buf_idx++] = '.';

   /* Until we reach the null terminator or the buffer size. */
   } while( 0 != *ptr && buf_idx < buf_len );

   return buf_idx;
}

uint16_t mname_get_q_type( struct mname_msg* msg_in, uint16_t idx ) {
   uint8_t* ptr = (uint8_t*)msg_in;

   if( idx > msg_in->questions_len ) {
      return 0;
   }

   /* TODO: Move to the requested index. */

   ptr += sizeof( struct mname_msg );
   ptr += mname_get_q_domain_len( msg_in, idx ) + 1; /* +1 for terminator. */

   return (uint16_t)*ptr;
}

uint16_t mname_get_q_class( struct mname_msg* msg_in, uint16_t idx ) {
   uint8_t* ptr = (uint8_t*)msg_in;

   if( idx > msg_in->questions_len ) {
      return 0;
   }

   /* TODO: Move to the requested index. */

   ptr += sizeof( struct mname_msg );  /* Skip header. */
   ptr += mname_get_q_domain_len( msg_in, idx ) + 1; /* Skip Q domain. */
   ptr += 2; /* Skip Q type. */

   return (uint16_t)*ptr;
}

/**
 * \brief   Get a pointer to an answer section.
 * @param idx  The index of the section to return.
 */
uint16_t mname_get_a_ptr( struct mname_msg* msg_in, uint16_t idx ) {
   uint8_t* ptr = (uint8_t*)msg_in;

   ptr += sizeof( struct mname_msg );
   ptr += mname_get_q_domain_len( msg_in, idx ) + 1;

   /* TODO */

   return 0;
}

