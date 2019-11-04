
#include "mname.h"

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

/*!
 * @return New msg packet length, or -1 on failure.
 */
int mname_add_answer(
   struct mname_msg* msg, size_t msg_buf_sz,
   char* domain, uint8_t domain_len,
   uint16_t type, uint16_t class, uint32_t ttl,
   char* rdata, uint16_t rdata_len
) {
   uint8_t* ptr = (uint8_t*)msg;
   uint16_t i = 0;
   size_t a_record_sz = 0;
   size_t a_record_offset = 0;
   size_t last_domain_sz = 0;
   uint8_t last_domain_sz_offset = 0;
   uint16_t* field_ptr = NULL;
   uint32_t* ttl_ptr = NULL;

#if 0
   for( i = 0 ; ip_len > i ; i++ ) {
      a_record_sz++;
      if( '.' == ip[i] ) {
         a_record_sz++;
      }
   }
#endif

   a_record_sz = domain_len + M_NAME_WIDTH_TYPE +
      M_NAME_WIDTH_CLASS + M_NAME_WIDTH_TTL + M_NAME_WIDTH_RDATA_SZ +
      rdata_len;
   a_record_offset = mname_get_offset( msg, msg_buf_sz, 1 );
 
   /* Shift everything after this answer up. */
   /* TODO: Bounds checking. */
   for(
      i = mname_get_msg_len( msg, msg_buf_sz ) - 1;
      a_record_offset < i;
      i--
   ) {
      ptr[i + a_record_sz] = ptr[i];
      ptr[i] = 0;
   }

   /* Copy the domain name. */
   /* TODO: Implement compression. */
   last_domain_sz_offset = a_record_offset;
   for(
      i = a_record_offset + M_NAME_WIDTH_DOMAIN_SZ;
      a_record_offset + domain_len > i;
      i++
   ) {
      if( '.' == domain[i - a_record_offset - 1] ) {
         /* Add length byte. */
         ptr[last_domain_sz_offset] = last_domain_sz;
         last_domain_sz_offset = i;
         last_domain_sz = 0;
      } else {
         /* Copy domain segment character. */
         ptr[i] = domain[i - a_record_offset - 1];
         last_domain_sz++;
      }
   }

   /* Set the response type and class. */
   field_ptr =
      (uint16_t*)&(ptr[a_record_offset + domain_len]);
   *field_ptr = m_htons( type );
   field_ptr =
      (uint16_t*)&(ptr[a_record_offset + domain_len +
      M_NAME_WIDTH_TYPE]);
   *field_ptr = m_htons( class );

   /* Set the TTL. */
   ttl_ptr =
      (uint32_t*)&(ptr[a_record_offset + domain_len +
      M_NAME_WIDTH_TYPE + M_NAME_WIDTH_CLASS]);
   *ttl_ptr = m_htonl( ttl );

   /* Set the RDATA length. */
   field_ptr =
      (uint16_t*)&(ptr[a_record_offset + domain_len +
      M_NAME_WIDTH_TYPE + M_NAME_WIDTH_CLASS + M_NAME_WIDTH_TTL]);
   *field_ptr = m_htons( rdata_len );

   /* Copy the rdata. */
   ptr = (uint8_t*)field_ptr;
   ptr += M_NAME_WIDTH_RDATA_SZ;
   for( i = 0 ; i < rdata_len ; i++ ) {
      ptr[i] = rdata[i];
   }

   /* Increment a_count. */
   msg->a_count = m_htons( (m_htons( msg->a_count ) + 1) );

   return a_record_sz;
}

