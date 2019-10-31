
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

int main( int argc, char** argv ) {
   int sock = 0;
   int res = 0;
   struct sockaddr_in server;
   struct sockaddr_in client;
   char buffer[1024] = { 0 };
   ssize_t count = 0;
   socklen_t client_sz = 0;
   struct mname_msg* dns_msg = (struct mname_msg*)&buffer;

   memset( &server, '\0', sizeof( struct sockaddr_in ) );
   memset( &client, '\0', sizeof( struct sockaddr_in ) );

   /* Create the socket. */
   sock = socket( AF_INET, SOCK_DGRAM, 0 );
   if( 0 > sock ) {
      fprintf( stderr, "socket(): %s\n", strerror( errno ) );
      goto cleanup;
   }

   /* Bind to the DNS port. */
   server.sin_family = AF_INET;
   server.sin_port = 53;
   server.sin_addr.s_addr = INADDR_ANY;

   res = bind( sock, (struct sockaddr*)&server, sizeof( struct sockaddr_in ) );
   if( 0 > res ) {
      fprintf( stderr, "bind(): %s\n", strerror( errno ) );
      goto cleanup;
   }

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
      fprintf( stderr, "datagram too large; truncated.\n" );
      goto cleanup;
   } else {
   }

   printf( "dns:\nis_response(): %d\n nslen: %d\n",
      m_name_is_response( dns_msg ), dns_msg->ns_len );

cleanup:

   if( 0 < sock ) {
      close( sock );
   }

   return res;
}

