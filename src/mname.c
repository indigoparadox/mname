
#include "mname.h"

void mname_response( struct mname_msg* msg_in ) {
   msg_in->fields |= M_NAME_RESPONSE_FIELD;

   msg_in->answers_len = m_htons( 1 );
}

int mname_get_q_domain( struct mname_msg* msg_in, char* buf, size_t buf_len ) {
   uint8_t* ptr = (uint8_t*)msg_in;
   uint16_t len = 0;
   size_t buf_idx = 0;

   /* Grab the name into the buffer one segment at a time. */
   ptr += sizeof( struct mname_msg );
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

