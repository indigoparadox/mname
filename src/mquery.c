
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
   const char* hostname = NULL;
   const char* portname = "domain";
   struct addrinfo hints;
   struct addrinfo* addresses = NULL;
   int res = 0;
   char buffer[1024] = { 0 };
   struct sockaddr_storage src_addr;
   ssize_t count = 0;
   socklen_t src_addr_len = 0;

   memset( &hints, '\0', sizeof( struct addrinfo ) );

   /* Get the local listening address. */
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_protocol = 0;
   hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;

   res = getaddrinfo( hostname, portname, &hints, &addresses );

   /* Create the socket. */
   sock = socket(
      addresses->ai_family, addresses->ai_socktype, addresses->ai_protocol );

   if( 0 > sock ) {
      fprintf( stderr, "%s\n", strerror( errno ) );
      goto cleanup;
   }

   /* Listen for incoming packets. */
   src_addr_len = sizeof( src_addr ); 
   count = recvfrom( sock, buffer, sizeof( buffer ), 0,
      (struct sockaddr*)&src_addr, &src_addr_len );

   /* Handle incoming packets. */
   if( 0 > count ) {
      fprintf( stderr, "%s\n", strerror( errno ) );
      goto cleanup;
   } else if( sizeof( buffer ) == count ) {
      fprintf( stderr, "Datagram too large; truncated.\n" );
      goto cleanup;
   } else {
   }

cleanup:

   if( NULL != addresses ) {
      freeaddrinfo( addresses );
   }

   return res;
}

