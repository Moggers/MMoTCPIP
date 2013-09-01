#include <ncurses.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include "player.cpp"
#include <pthread.h>

#define PORT "1337"

namespace network
{
	pthread_t servthread;
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	fd_set master, readset;

	void * get_in_addr( struct sockaddr *sa )
	{
		if( sa->sa_family == AF_INET )
		{
			return &((( struct sockaddr_in * )sa )->sin_addr );
		}
		
		return &((( struct sockaddr_in6 * )sa )->sin6_addr );
	}

	int connect( char * address )
	{
		memset( &hints, 0, sizeof( hints ) );
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		if( ( rv = getaddrinfo( address, PORT, &hints, &servinfo ) ) != 0 )
		{
			fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( rv ) );
			return 1;
		}

		for( p = servinfo; p != NULL; p = p->ai_next )
		{
			if( ( sockfd = socket( p->ai_family, p->ai_socktype, p->ai_protocol ) ) == -1 )
			{
				perror( "client: socket" );
				continue;
			}
			if( connect( sockfd, p->ai_addr, p->ai_addrlen ) == -1 )
			{
				close( sockfd );
				perror( "client: connect" );
				continue;
			}
			break;
		}
		
		if( p == NULL )
		{
			fprintf( stderr, "client: failed to connect\n" );
			return 2;
		}

		inet_ntop( p->ai_family, get_in_addr( ( struct sockaddr *)p->ai_addr ), s, sizeof( s ) );

		freeaddrinfo( servinfo );

		return sockfd;
	}

	void *beginthread( void * param )
	{
		for( ;; )
		{
			Entities::ply_update recv_ply_update;
			recv( sockfd, &recv_ply_update, sizeof( recv_ply_update ), 0 );
			Entities::ply_list[recv_ply_update.index].update( &recv_ply_update );
		}
	}
}
