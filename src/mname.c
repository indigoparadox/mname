
#include "mname.h"

void mname_response( struct mname_msg* msg_in, size_t msg_buf_sz ) {
   msg_in->fields |= M_NAME_RESPONSE_FIELD;

   msg_in->a_count = m_htons( 1 );
}

int mname_get_domain_len(
   const struct mname_msg* msg_in, size_t msg_buf_sz, uint16_t idx
) {
   const uint8_t* ptr = (const uint8_t*)msg_in;
   size_t msg_offset = 0;
   int len_out = 0;

   msg_offset += mname_get_offset( msg_in, msg_buf_sz, idx );

   /* Much quicker and simpler version of the loop from mname_get_domain(). */
   do {
      len_out += ptr[msg_offset] + 1; /* +1 for . */
      msg_offset += ptr[msg_offset] + M_NAME_WIDTH_DOMAIN_SZ;

      /* Garbage data emergency brake. */
      if( len_out > msg_buf_sz ) {
         return -1;
      }
   } while( 0 != ptr[msg_offset] );

   /* Add a terminator if there was a domain name. */
   if( 1 < len_out ) {
      len_out++;
   }

   return len_out;
}

int mname_get_domain(
   const struct mname_msg* msg_in, size_t msg_buf_sz, uint16_t idx,
   char* buf, size_t buf_sz
) {
   const uint8_t* ptr = (const uint8_t*)msg_in;
   uint8_t segment_len = 0;
   size_t out_buf_idx = 0;
   size_t msg_offset = 0;

   /* Grab the name into the buffer one segment at a time. */
   msg_offset += mname_get_offset( msg_in, msg_buf_sz, idx );
   do {
      /* Grab the name component length and move up by 1 byte. */
      segment_len = ptr[msg_offset];
      msg_offset += 1;

      /* Grab the name segment into the buffer one char at a time. */
      while( 0 < segment_len && out_buf_idx + 1 < buf_sz ) { /* +1 for . */
         buf[out_buf_idx++] = (char)ptr[msg_offset];
         msg_offset++;
         segment_len--; /* 1 less to go. */

         /* Garbage data emergency brake. */
         if( msg_offset > buf_sz ) {
            return -1;
         }
      }
      buf[out_buf_idx++] = '.';

   /* Until we reach the null terminator or the buffer size. */
   } while( 0 != ptr[msg_offset] && out_buf_idx < buf_sz );

   /* Add a terminator if there was a domain name. */
   if( 1 < out_buf_idx ) {
      out_buf_idx++;
   }
   
   return out_buf_idx;
}

int mname_get_a_rdata_len(
   const struct mname_msg* msg_in, size_t msg_buf_sz, uint16_t idx
) {
   const uint8_t* ptr = (const uint8_t*)msg_in;

   /* TODO: Check for exceeding msg buffer. */

   /* Grab the name into the buffer one segment at a time. */
   ptr += mname_get_offset( msg_in, msg_buf_sz, idx );
   ptr += mname_get_domain_len( msg_in, msg_buf_sz, idx ); /* Skip Q domain. */
   ptr += M_NAME_WIDTH_TYPE +
      M_NAME_WIDTH_CLASS +
      M_NAME_WIDTH_TTL;

   return m_htons( *((uint16_t*)ptr) );
}

int mname_get_a_rdata(
   const struct mname_msg* msg_in, size_t msg_buf_sz, uint16_t idx,
   uint8_t* buf, size_t buf_len
) {
   const uint8_t* ptr = (const uint8_t*)msg_in;
   uint16_t len = 0;
   int i = 0;
   int offset = 0;

   /* Grab the name into the buffer one segment at a time. */
   offset += mname_get_offset( msg_in, msg_buf_sz, idx );
   offset += mname_get_domain_len( msg_in, msg_buf_sz, idx ); /* Skip domain. */
   offset += M_NAME_WIDTH_TYPE +
      M_NAME_WIDTH_CLASS +
      M_NAME_WIDTH_TTL;

   len = m_htons( *((uint16_t*)&(ptr[offset])) );
   offset += M_NAME_WIDTH_RDATA_SZ;

   for( i = 0 ; len > i ; i++ ) {
      /* Garbage data emergency brake. */
      if( offset > msg_buf_sz ) {
         return -1;
      }

      buf[i] = ptr[offset++];
   }

   return len;
}

/**
 * \brief   Get a pointer to a record. Determine the record based on count
 *          fields in header and reasoning.
 * @param idx  The index of the section to return.
 */
int mname_get_offset(
   const struct mname_msg* msg_in, size_t msg_buf_sz, uint16_t idx
) {
   const uint8_t* ptr = (const uint8_t*)msg_in;
   uint16_t search_idx = 0;
   int offset = 0;

   offset = sizeof( struct mname_msg );
   while( search_idx < idx ) {
      if( 0 == search_idx ) {
         /* Must be a question record. */

         offset += mname_get_domain_len(
            msg_in, msg_buf_sz, search_idx ); /* Skip dom. */
         offset += M_NAME_WIDTH_TYPE + M_NAME_WIDTH_CLASS;

      } else {
         /* Must be an answer/ns/additional record. */

         offset += mname_get_domain_len(
            msg_in, msg_buf_sz, search_idx ); /* Skip dom. */
         offset += 
            M_NAME_WIDTH_TYPE + M_NAME_WIDTH_CLASS + M_NAME_WIDTH_TTL;
         offset += m_htons( *((uint16_t*)&ptr[offset]) ); /* Skip response. */
         offset += M_NAME_WIDTH_RDATA_SZ;
      }

      /* Garbage data emergency brake. */
      if( offset > msg_buf_sz ) {
         return -1;
      }

      search_idx++;
   }

   return offset;
}

#if 0
/*!
 * @return New msg packet length, or -1 on failure.
 */
int mname_add_answer( struct mname_msg* msg, size_t buf_sz ) {
   size_t domain_len = 0;
   //uint16_t offset = 0;
   uint16_t size = 0;
   uint8_t* a_ptr = (uint8_t*)msg;
   uint8_t* q_ptr = (uint8_t*)msg;
   uint16_t i = 0;

   size = mname_get_offset(
      msg, (m_htons( msg->a_count ) + m_htons( msg->n_count ) +
         m_htons( msg->l_count ) + 2) ) /* +2 for Q and end. */

   domain_len = mname_get_domain_len( msg, 0 ); /* Q is always 0. */
   a_ptr += mname_get_offset( msg, m_htons( msg->a_count ) + 1 );
   q_ptr += mname_get_offset( msg, 0 );

   /* TODO: Shift everything after this answer up. */

   /* Copy the domain name. */
   /* TODO: Implement compression. */
   for( i = 0 ; domain_len > i ; i++ ) {
      a_ptr[i] = q_ptr[i];
   }

   /* TODO: Incremenr a_count. */
}
#endif

