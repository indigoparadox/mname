
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

int main( int argc, char** argv ) {
   int sock = 0;
   int res = 0;
   struct sockaddr_in server;
   struct sockaddr_storage client;
   char buffer[MSG_BUF_LEN] = { 0 };
   ssize_t count = 0;
   socklen_t client_sz = sizeof( struct sockaddr_storage );
   struct mname_msg* dns_msg = (struct mname_msg*)&buffer;
   int running = 1;
   char domain_name[NAME_BUF_LEN] = { 0 };
   int i = 0;
   uint16_t size = 0;
   FILE* pkt_file = NULL;

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
      count = recvfrom( sock, buffer, sizeof( buffer ), 0,
         (struct sockaddr*)&client, &client_sz );
      if( 0 > count ) {
         fprintf( stderr, "recvfrom(): %s\n", strerror( errno ) );
         goto cleanup;
      }

      /* Handle incoming packets. */
      if( 0 > count ) {
         fprintf( stderr, "%s\n", strerror( errno ) );
         goto cleanup;
      } else if( sizeof( buffer ) == count ) {
         fprintf( stderr, "datagram too large; truncated to: %ld\n", count );
         goto cleanup;
      } else {
      }

      memset( domain_name, '\0', NAME_BUF_LEN );
      mname_get_domain( dns_msg, 0, domain_name, NAME_BUF_LEN );

      size = mname_get_offset(
         dns_msg, (m_htons( dns_msg->answers_len ) + 
            m_htons( dns_msg->ns_len ) +
            m_htons( dns_msg->addl_len ) + 2) ); /* +2 for Q and end. */
      printf( "%ld read, %d size\n", count, size );

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
      for( i = 0 ; count > i ; i++ ) {
         if( 0 != i && 0 == i % 20 ) {
            printf( "\n" );
         }
         
         /* Add some color. */
         if( i < sizeof( struct mname_msg ) ) {
            printf( "\033[0;31m" );
         } else if(
            i <= sizeof( struct mname_msg ) +
            mname_get_domain_len( dns_msg, 0 )
         ) {
            printf( "\033[0;33m" );
         } else if(
            i <= sizeof( struct mname_msg ) +
            mname_get_domain_len( dns_msg, 0 ) + 4
         ) {
            printf( "\033[0;34m" );
         } else {
            printf( "\033[0;36m" );
         }

         printf( "%02hhx ", buffer[i] );
      }
      printf( "\033[0m" );
      printf( "\n" );

      pkt_file = fopen( "dnspkt.bin", "wb" );
      fwrite( buffer, 1, count, pkt_file );
      fclose( pkt_file );
      pkt_file = NULL;

      printf( "dns:\n cli_addr_sz: %d\n is_response(): %d\n nslen: %d\n"
         " questions: %d\n answers: %d\n ns: %d\n additional: %d\n"
         " domain: %s (%d)\n type: %d\n class: %d\n",
         client_sz, m_name_is_response( dns_msg ), dns_msg->ns_len,
         m_htons( dns_msg->questions_len ),
         m_htons( dns_msg->answers_len ),
         m_htons( dns_msg->ns_len ),
         m_htons( dns_msg->addl_len ),
         domain_name,
         mname_get_domain_len( dns_msg, 0 ),
         mname_get_q_type( dns_msg ),
         mname_get_q_class( dns_msg ) );

      assert( size == count );
      assert( 1 == mname_get_q_class( dns_msg ) );
      assert( 1 == mname_get_q_type( dns_msg ) );

      mname_response( dns_msg );

      /* Send response back to requester. */
      count = sendto( sock, buffer, count, 0,
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

