
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
#define MSG_BUF_LEN 512

void pkt_dump_display( const struct mname_msg* dns_msg, size_t sz ) {
   int i = 0;
   const uint8_t* buffer = (const uint8_t*)dns_msg;

   /* Pretty header for hex dump. */
   i = 0;
   do {
      printf( "%02d ", i++ );
   } while( 0 != i % 20 );
   printf( "\n" );

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
         mname_get_domain_len( dns_msg, 0 )
      ) {
         /* Question Domain */
         printf( "\033[0;33m" );

      } else if(
         i < mname_get_offset( dns_msg, 1 )
      ) {
         /* Question Fields */
         printf( "\033[0;34m" );

      } else if(
         i < mname_get_offset( dns_msg, 1 ) +
         mname_get_domain_len( dns_msg, 1 )
      ) {
         /* Answer/Addl Domain */
         printf( "\033[0;33m" );

      } else if(
         i < mname_get_offset( dns_msg, 1 ) +
         mname_get_domain_len( dns_msg, 1 ) +
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
   printf( "\n" );
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

int main( int argc, char** argv ) {
   int sock = 0;
   int res = 0;
   struct sockaddr_in server;
   struct sockaddr_storage client;
   uint8_t pkt_buf[MSG_BUF_LEN] = { 0 };
   ssize_t count = 0;
   socklen_t client_sz = sizeof( struct sockaddr_storage );
   struct mname_msg* dns_msg = (struct mname_msg*)&pkt_buf;
   int running = 1;
   char domain_name[NAME_BUF_LEN] = { 0 };
   int i = 0, j = 0;
   uint16_t size = 0;
   uint16_t records_count = 0;
   int rd_len = 0;

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
      count = recvfrom( sock, pkt_buf, MSG_BUF_LEN, 0,
         (struct sockaddr*)&client, &client_sz );
      if( 0 > count ) {
         fprintf( stderr, "recvfrom(): %s\n", strerror( errno ) );
         goto cleanup;
      }

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
         m_htons( dns_msg->questions_len ) + 
         m_htons( dns_msg->answers_len ) + 
         m_htons( dns_msg->ns_len ) +
         m_htons( dns_msg->addl_len );

      pkt_dump_display( dns_msg, count );
      pkt_dump_file( "dnspkt.bin", pkt_buf, count );

      printf( "dns:\n cli_addr_sz: %d\n is_response(): %d\n "
         " questions: %d\n answers: %d\n ns: %d\n additional: %d\n",
         client_sz, m_name_is_response( dns_msg ),
         m_htons( dns_msg->questions_len ),
         m_htons( dns_msg->answers_len ),
         m_htons( dns_msg->ns_len ),
         m_htons( dns_msg->addl_len ) );

      /* +2 for Q and end. */
      size = mname_get_offset( dns_msg, records_count + 2);
      printf( "%d records, %ld read, %d size\n", records_count, count, size );

      for( i = 0 ; records_count > i ; i++ ) {
         printf( "record %d @ %d bytes:\n", i, mname_get_offset( dns_msg, i ) );

         memset( domain_name, '\0', NAME_BUF_LEN );
         assert( mname_get_domain( dns_msg, i, domain_name, NAME_BUF_LEN ) ==
            mname_get_domain_len( dns_msg, i ) );
         printf( " domain: %s (%d)\n type: %d\n class: %d\n",
            domain_name,
            mname_get_domain_len( dns_msg, i ),
            mname_get_type( dns_msg, i ),
            mname_get_class( dns_msg, i ) );

         if( 0 < i ) {
            printf( " ttl: %d\n", mname_get_a_ttl( dns_msg, i ) );
            //rd_len = mname_get_a_rdata( dns_msg, i, pkt_buf, NAME_BUF_LEN  );
            printf( " rdata (%d): ", rd_len );
            for( j = 0 ; rd_len > j ; j++ ) {
               //printf( "%02x ", buffer[j] );
            }
            printf( "\n" );
         }
      }

      pkt_dump_display( dns_msg, count );

      //assert( size == count );
      printf( "%d\n", mname_get_class( dns_msg, 0 ) );
      assert( 1 == mname_get_class( dns_msg, 0 ) );
      assert( 1 == mname_get_type( dns_msg, 0 ) );

      mname_response( dns_msg );

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

