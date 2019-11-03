
#include "mname.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#define NAME_BUF_LEN 512
#define PKT_BUF_LEN 512
#define RDATA_BUF_LEN 512

struct pkt_dump_highlight {
   uint16_t offset;
   char ansi[6];
};

void pkt_dump_setup() {

}

void pkt_dump_display(
   const struct mname_msg* dns_msg, size_t sz
) {
   int i = 0;
   const uint8_t* buffer = (const uint8_t*)dns_msg;

   printf( "\n" );

   /* Pretty header for hex dump. */
   i = 0;
   do {
      printf( "%02d ", i++ );
   } while( 0 != i % 20 );
   printf( "\n" );

   /* Dividing border. */
   i = 0;
   do {
      printf( "-" );
   } while( 0 != ++i % 30 );
   printf( "\n" );

   /* Packet hex dump! */
   for( i = 0 ; sz > i ; i++ ) {
      if( 0 != i && 0 == i % 21 ) {
         printf( "\n" );
      }
      
      /* Add some color. */
      if( i < sizeof( struct mname_msg ) ) {
         /* Header */
         printf( "\033[0;31m" );

      } else if(
         i < sizeof( struct mname_msg ) +
         mname_get_domain_len( dns_msg, PKT_BUF_LEN, 0 )
      ) {
         /* Question Domain */
         printf( "\033[0;33m" );

      } else if(
         i < mname_get_offset( dns_msg, PKT_BUF_LEN, 1 )
      ) {
         /* Question Fields */
         printf( "\033[0;34m" );

      } else if(
         i < mname_get_offset( dns_msg, PKT_BUF_LEN, 1 ) +
         mname_get_domain_len( dns_msg, PKT_BUF_LEN, 1 )
      ) {
         /* Answer/Addl Domain */
         printf( "\033[0;33m" );

      } else if(
         i < mname_get_offset( dns_msg, PKT_BUF_LEN, 1 ) +
         mname_get_domain_len( dns_msg, PKT_BUF_LEN, 1 ) +
         M_NAME_WIDTH_TYPE + M_NAME_WIDTH_CLASS + M_NAME_WIDTH_TTL
      ) {
         /* Answer/Addl Fields */
         printf( "\033[0;32m" );

      } else {
         /* ??? */
         printf( "\033[0;36m" );
      }

      printf( "%02hhx ", buffer[i] );
   }
   printf( "\033[0m" );
   printf( "\n\n" );

}

void pkt_dump_file( const char* name, const uint8_t* buffer, size_t sz ) {
   FILE* pkt_file = NULL;

   /* File dump. */
   pkt_file = fopen( name, "wb" );
   if( NULL == pkt_file ) {
      fprintf( stderr, "fopen(): %s\n", strerror( errno ) );
      goto cleanup;
   }

   fwrite( buffer, 1, sz, pkt_file );

cleanup:
   if( NULL != pkt_file ) {
      fclose( pkt_file );
   }
}

void pkt_summarize( const uint8_t* pkt_buf, size_t count ) {
   uint16_t records_count = 0;
   int rd_len = 0;
   char domain_name[NAME_BUF_LEN] = { 0 };
   int i = 0, j = 0;
   struct mname_msg* dns_msg = (struct mname_msg*)pkt_buf;
   uint8_t rdata_buf[RDATA_BUF_LEN] = { 0 };
   uint16_t size = 0;

   /* Handle incoming packets. */
   if( 0 > count ) {
      fprintf( stderr, "%s\n", strerror( errno ) );
      goto cleanup;
   } else if( sizeof( pkt_buf ) == count ) {
      fprintf( stderr, "datagram too large; truncated to: %ld\n", count );
      goto cleanup;
   } else {
   }

   records_count = 
      m_htons( dns_msg->q_count ) + 
      m_htons( dns_msg->a_count ) + 
      m_htons( dns_msg->n_count ) +
      m_htons( dns_msg->l_count );

   pkt_dump_display( dns_msg, count );
   pkt_dump_file( "dnspkt.bin", pkt_buf, count );

   printf( "dns:\n is_response(): %d\n "
      " questions: %d\n answers: %d\n ns: %d\n additional: %d\n",
      m_name_is_response( dns_msg ),
      m_htons( dns_msg->q_count ),
      m_htons( dns_msg->a_count ),
      m_htons( dns_msg->n_count ),
      m_htons( dns_msg->l_count ) );

   size = mname_get_msg_len( dns_msg, PKT_BUF_LEN );
   printf( "%d records, %ld read, %d size\n", records_count, count, size );

   for( i = 0 ; records_count > i ; i++ ) {
      printf( "record %d @ %d bytes:\n", i,
         mname_get_offset( dns_msg, PKT_BUF_LEN, i ) );

      memset( domain_name, '\0', NAME_BUF_LEN );
      assert(
         mname_get_domain( dns_msg, PKT_BUF_LEN, i, domain_name, NAME_BUF_LEN ) 
         == mname_get_domain_len( dns_msg, PKT_BUF_LEN, i ) );
      printf( " domain: %s (%d)\n type: %d\n class: %d\n",
         domain_name,
         mname_get_domain_len( dns_msg, PKT_BUF_LEN, i ),
         mname_get_type( dns_msg, PKT_BUF_LEN, i ),
         mname_get_class( dns_msg, PKT_BUF_LEN, i ) );

      if( 0 < i ) {
         /* Answer/Addl Record. */
         printf( " ttl: %d\n", mname_get_a_ttl( dns_msg, PKT_BUF_LEN, i ) );
         memset( rdata_buf, '\0', RDATA_BUF_LEN );
         mname_get_a_rdata( dns_msg, PKT_BUF_LEN, i, rdata_buf, RDATA_BUF_LEN );
         rd_len = mname_get_a_rdata_len( dns_msg, PKT_BUF_LEN, i  );
         printf( " rdata (%d): ", rd_len );
         for( j = 0 ; rd_len > j ; j++ ) {
            printf( "%02hhx ", rdata_buf[j] );
         }
         printf( "\n" );
      }
   }

   pkt_dump_display( dns_msg, count );

#if 0
   printf( "(%d): %02hhx %02hhx\n", size, pkt_buf[size], pkt_buf[size + 1] );
   fflush( 0 );
   assert( 0 != pkt_buf[size] );
   printf( "after pkt: " );
   for( i = size ; PKT_BUF_LEN > i ; i++ ) {
      printf( "%02hhx ", pkt_buf[i] );
      fflush( 0 );
      assert( 0 == pkt_buf[i] );
   }
   printf( "\n" );
#endif

cleanup:

   return;
}

int main( int argc, char** argv ) {
   int sock = 0;
   int res = 0;
   struct sockaddr_in server;
   struct sockaddr_storage client;
   uint8_t pkt_buf[PKT_BUF_LEN] = { 0 };
   ssize_t count = 0;
   socklen_t client_sz = sizeof( struct sockaddr_storage );
   struct mname_msg* dns_msg = (struct mname_msg*)&pkt_buf;
   int running = 1;

   memset( &server, '\0', sizeof( struct sockaddr_in ) );
   memset( &client, '\0', sizeof( struct sockaddr_in ) );

   /* Create the socket. */
   sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
   if( 0 > sock ) {
      fprintf( stderr, "socket(): %s\n", strerror( errno ) );
      goto cleanup;
   }

   /* Bind to the DNS port. */
   server.sin_family = AF_INET;
   server.sin_port = 53; /* Yes, this is wrong, but it's unpriv. */
   server.sin_addr.s_addr = htonl( INADDR_ANY );

   res = bind( sock, (struct sockaddr*)&server, sizeof( struct sockaddr_in ) );
   if( 0 > res ) {
      fprintf( stderr, "bind(): %s\n", strerror( errno ) );
      goto cleanup;
   }

   while( running ) {

      /* Listen for incoming packets. */
      count = recvfrom( sock, pkt_buf, PKT_BUF_LEN, 0,
         (struct sockaddr*)&client, &client_sz );
      if( 0 > count ) {
         fprintf( stderr, "recvfrom(): %s\n", strerror( errno ) );
         goto cleanup;
      }

      pkt_summarize( pkt_buf, count );

      assert( 1 == mname_get_class( dns_msg, PKT_BUF_LEN, 0 ) );
      assert( 1 == mname_get_type( dns_msg, PKT_BUF_LEN, 0 ) );

      mname_response( dns_msg, PKT_BUF_LEN );

      /* Send response back to requester. */
      count = sendto( sock, pkt_buf, count, 0,
         (struct sockaddr*)&client, client_sz );
      if( 0 > count ) {
         fprintf( stderr, "sendto(): %s\n", strerror( errno ) );
         goto cleanup;
      }

   }

cleanup:

   if( 0 < sock ) {
      close( sock );
   }

   return res;
}

