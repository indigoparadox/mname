
#include "mname.h"

void mname_response( struct mname_msg* msg_in ) {
   msg_in->fields |= M_NAME_RESPONSE_FIELD;

   msg_in->answers_len = m_htons( 1 );
}

int mname_get_domain_len( const struct mname_msg* msg_in, uint16_t idx ) {
   uint8_t* ptr = (uint8_t*)msg_in;
   int len_out = 0;

   ptr += mname_get_offset( msg_in, idx );

   /* Much quicker and simpler version of the loop from mname_get_domain(). */
   do {
      len_out += *ptr + 1; /* +1 for sz octet. */
      ptr += *ptr + 1;
   } while( 0 != *ptr );

   return len_out;
}

int mname_get_domain(
   const struct mname_msg* msg_in, uint16_t idx, char* buf, size_t buf_len
) {
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

uint16_t mname_get_q_type( const struct mname_msg* msg_in ) {
   uint8_t* ptr = (uint8_t*)msg_in;

   /* Move past the header and domain. */
   ptr += sizeof( struct mname_msg );
   ptr += mname_get_domain_len( msg_in, 0 ); /* Q is always 0. */
   ptr += 1; /* +1 for first byte of short. */

   return m_htons( *((uint16_t*)ptr) );
}

uint16_t mname_get_q_class( const struct mname_msg* msg_in ) {
   uint8_t* ptr = (uint8_t*)msg_in;

   /* Move past the header and domain. */
   ptr += sizeof( struct mname_msg );  /* Skip header. */
   ptr += mname_get_domain_len( msg_in, 0 ); /* Skip Q domain. */
   ptr += 3; /* Skip Q type and first byte of short. */

   return m_htons( *((uint16_t*)ptr) );
}

/**
 * \brief   Get a pointer to a record. Determine the record based on count
 *          fields in header and reasoning.
 * @param idx  The index of the section to return.
 */
uint16_t mname_get_offset( const struct mname_msg* msg_in, uint16_t idx ) {
   uint8_t* ptr = (uint8_t*)msg_in;
   uint16_t search_idx = 0;
   uint16_t offset = 0;

   offset = sizeof( struct mname_msg );

   while( search_idx < idx ) {
      if( 0 == search_idx ) {
         /* Must be a question record. */

         offset += mname_get_domain_len( msg_in, search_idx ); /* Skip domain. */
         offset += 4; /* Skip type and class. */

      } else if( m_htons( msg_in->answers_len ) > search_idx ) {
         /* Must be an answer record. */

         offset += mname_get_domain_len( msg_in, search_idx ); /* Skip dom. */
         offset += 8; /* Skip type, class, and TTL. */
         offset += m_htons( (uint16_t)ptr[offset] ); /* Skip response. */
         offset += 2; /* Skip response data length. */

      } else if(
         m_htons( msg_in->ns_len ) >
         search_idx - m_htons( msg_in->answers_len )
      ) {
         /* Must be a nameserver record. */
         /* TODO */
   
      } else {
         /* Must be an additional record. */
         offset += mname_get_domain_len( msg_in, search_idx ); /* Skip dom. */
         offset += 6; /* Skip type, class, and TTL. */
         offset += m_htons( (uint16_t)ptr[offset] ); /* Skip response. */
         offset += 2; /* Skip response data length. */
      }

      search_idx++;
   }

   return offset;
}

#if 0
void mname_add_answer( struct mname_msg* msg, size_t buf_sz ) {
   size_t domain_len = 0;
   //uint16_t offset = 0;
   uint16_t size = 0;
   uint8_t* a_ptr = (uint8_t*)msg;
   uint8_t* q_ptr = (uint8_t*)msg;
   uint16_t i = 0;

   size = mname_get_offset(
      msg, (m_htons( msg->answers_len ) + m_htons( msg->ns_len ) +
         m_htons( msg->addl_len ) + 2) ) /* +2 for Q and end. */

   domain_len = mname_get_domain_len( msg, 0 ); /* Q is always 0. */
   a_ptr += mname_get_offset( msg, m_htons( msg->answers_len ) + 1 );
   q_ptr += mname_get_offset( msg, 0 );

   /* TODO: Shift everything after this answer up. */

   /* Copy the domain name. */
   /* TODO: Implement compression. */
   for( i = 0 ; domain_len > i ; i++ ) {
      a_ptr[i] = q_ptr[i];
   }

   /* TODO: Incremenr answers_len. */
}
#endif

