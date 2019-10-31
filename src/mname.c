
#include "mname.h"

void mname_response( struct mname_msg* msg_in ) {
   msg_in->fields |= M_NAME_RESPONSE_FIELD;

   msg_in->answers_len = m_htons( 1 );
}

int mname_get_q_domain_len( const struct mname_msg* msg_in, uint16_t idx ) {
   uint8_t* ptr = NULL;
   int len_out = 0;

   if( idx > msg_in->questions_len ) {
      return 0;
   }

   /* Much quicker and simpler version of the loop from mname_get_q_domain(). */
   ptr = mname_get_q_ptr( msg_in, idx );
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

uint16_t mname_get_q_type( const struct mname_msg* msg_in, uint16_t idx ) {
   uint8_t* ptr = (uint8_t*)msg_in;

   if( idx > msg_in->questions_len ) {
      return 0;
   }

   /* TODO: Move to the requested index. */

   ptr += sizeof( struct mname_msg );
   ptr += mname_get_q_domain_len( msg_in, idx ); /* +1 for terminator. */
   ptr += 1;

   return m_htons( *((uint16_t*)ptr) );
}

uint16_t mname_get_q_class( const struct mname_msg* msg_in, uint16_t idx ) {
   uint8_t* ptr = (uint8_t*)msg_in;

   if( idx > msg_in->questions_len ) {
      return 0;
   }

   /* TODO: Move to the requested index. */

   ptr += sizeof( struct mname_msg );  /* Skip header. */
   ptr += mname_get_q_domain_len( msg_in, idx ); /* Skip Q domain. */
   ptr += 3; /* Skip Q type. */

   return m_htons( *((uint16_t*)ptr) );
}

/**
 * \brief   Get a pointer to an answer section.
 * @param idx  The index of the section to return.
 */
uint8_t* mname_get_a_ptr( const struct mname_msg* msg_in, uint16_t idx ) {
   uint8_t* ptr = (uint8_t*)msg_in;

   /* TODO: Handle multiple question fields. */
   ptr += sizeof( struct mname_msg );
   ptr += mname_get_q_domain_len( msg_in, idx ) + 1;

   /* TODO */

   return NULL;
}

uint8_t* mname_get_q_ptr( const struct mname_msg* msg_in, uint16_t idx ) {
   uint8_t* ptr = (uint8_t*)msg_in;

   ptr += sizeof( struct mname_msg );

   return NULL;
}

void mname_add_answer( struct mname_msg* msg, uint16_t q_idx, size_t buf_sz ) {
   size_t domain_len = 0;
   size_t offset = 0;
   uint8_t* a_ptr = (uint8_t*)msg;
   uint8_t* q_ptr = (uint8_t*)msg;

   domain_len = mname_get_q_domain_len( msg, idx );
   a_ptr = mname_get_a_ptr( msg, msg->answers_len );
   q_ptr = mname_get_q_ptr( msg, q_idx );

   /* TODO: Shift everything after this answer up. */

   /* Copy the domain name. */
   /* TODO: Implement compression. */
   for( i = 0 ; domain_len > i ; i++ ) {
      a_ptr[i] = q_ptr[i];
   }

   /* TODO: Incremenr answers_len. */
}

