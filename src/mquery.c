
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

#include "mpktdmp.h"

int main( int argc, char** argv ) {
   int sock = 0;
   int res = 0;
   struct sockaddr_in server;
   struct sockaddr_storage client;
   uint8_t pkt_buf[PKT_BUF_SZ] = { 0 };
   ssize_t count = 0;
   socklen_t client_sz = sizeof( struct sockaddr_storage );
   struct mname_msg* dns_msg = (struct mname_msg*)&pkt_buf;
   int running = 1;
   char domain_name[NAME_BUF_SZ] = { 0 };
   size_t domain_name_len = 0;
   uint32_t ip_res = 0x01010101;

   memset( &server, '\0', sizeof( struct sockaddr_in ) );
   memset( &client, '\0', sizeof( struct sockaddr_in ) );

   printf( "opening socket...\n" );

   /* Create the socket. */
   sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
   if( 0 > sock ) {
      fprintf( stderr, "socket(): %s\n", strerror( errno ) );
      goto cleanup;
   }

   /* Bind to the DNS port. */
   server.sin_family = AF_INET;
   server.sin_port = htons( 53 );
   server.sin_addr.s_addr = htonl( INADDR_ANY );

   printf( "attempting to listen on %x port %d...\n",
      server.sin_addr.s_addr, server.sin_port );

   res = bind( sock, (struct sockaddr*)&server, sizeof( struct sockaddr_in ) );
   if( 0 > res ) {
      fprintf( stderr, "bind(): %s\n", strerror( errno ) );
      goto cleanup;
   }

   printf( "listening on %x port %d...\n",
      server.sin_addr.s_addr, server.sin_port );

   while( running ) {

      /* Listen for incoming packets. */
      count = recvfrom( sock, pkt_buf, PKT_BUF_SZ, 0,
         (struct sockaddr*)&client, &client_sz );
      if( 0 > count ) {
         fprintf( stderr, "recvfrom(): %s\n", strerror( errno ) );
         goto cleanup;
      }

      printf( "received:\n" );
      pkt_summarize( pkt_buf, count );

      assert( 1 == mname_get_class( dns_msg, PKT_BUF_SZ, 0 ) );
      assert( 1 == mname_get_type( dns_msg, PKT_BUF_SZ, 0 ) );

      /* Change the message to a response. */
      m_name_set_response( dns_msg );
      memset( domain_name, '\0', PKT_BUF_SZ );
      domain_name_len = mname_get_domain(
         dns_msg, PKT_BUF_SZ, 0, domain_name, NAME_BUF_SZ );
      mname_add_answer( dns_msg, PKT_BUF_SZ, domain_name,
         domain_name_len,
         mname_get_type( dns_msg, PKT_BUF_SZ, 0 ),
         mname_get_class( dns_msg, PKT_BUF_SZ, 0 ),
         0,
         (char*)&ip_res, 4 );

      printf( "sending:\n" );

      pkt_summarize( pkt_buf, count + 29 );

      /* Send response back to requester. */
      count = sendto( sock, pkt_buf, count, 0,
         (struct sockaddr*)&client, client_sz );
      if( 0 > count ) {
         fprintf( stderr, "sendto(): %s\n", strerror( errno ) );
         goto cleanup;
      }

      printf( "--- done! ---\n" );

   }

cleanup:

   if( 0 < sock ) {
      close( sock );
   }

   return res;
}

