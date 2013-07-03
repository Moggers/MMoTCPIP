#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT "9034"

void *get_in_addr( struct sockaddr *sa)
{
	if( sa->sa_family == AF_INET )
	{
		return &((( struct sockaddr_in*)sa)->sin_addr );
	}

	return &((( struct sockaddr_in6*)sa)->sin6_addr );
}
struct maze_param
{
	int maze_seed;
	int maze_width;
	int maze_height;
};
struct init_struct 
{
	struct maze_param maze;
	int player_id;
};

struct player
{
	int index;
	int x;
	int y;
};

int main( int argc, char * argv[] )
{
	if( argc != 4 )
	{
		fprintf( stderr, "Usage: ./server.o width height seed\n" );
		exit( 1 );
	}

	struct player ply[32];
	int plycount = 0;

	memset( ply, 0, sizeof( ply ) );
	maze_param maze;
	maze.maze_seed = atoi( argv[1] );
	maze.maze_width = atoi( argv[2] );
	maze.maze_height = atoi( argv[3] );

	fd_set master;
	fd_set read_fds;
	int fdmax;

	int listener;
	int newfd;
	struct sockaddr_storage remoteaddr;
	socklen_t addrlen;

	char buf[256];
	int nbytes;

	char remoteIP[INET6_ADDRSTRLEN];

	int yes = 1;
	int i, j, rv;

	struct addrinfo hints, *ai, *p;

	FD_ZERO( &master );
	FD_ZERO( &read_fds );

	memset( &hints, 0, sizeof hints );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if(( rv = getaddrinfo( NULL, PORT, &hints, &ai )) != 0 )
	{
		fprintf( stderr, "selectserver: %s\n", gai_strerror( rv ) );
		exit( 2 );
	}

	for( p = ai; p != NULL; p = p->ai_next )
	{
		listener = socket( p->ai_family, p->ai_socktype, p->ai_protocol );
		if( listener < 0 )
		{
			continue;
		}

		setsockopt( listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) );

		if( bind(listener, p->ai_addr, p->ai_addrlen ) < 0 )
		{
			close( listener );
			continue;
		}
		break;
	}
	if( p == NULL )
	{
		fprintf( stderr, "selectserver: failed to bind\n");
		exit( 3 );

	}

	freeaddrinfo( ai );

	if( listen( listener, 10 ) == -1 )
	{
		perror( "listen" );
		exit( 4 );
	}

	FD_SET( listener, &master );

	fdmax = listener;

	for( ;; )
	{
		read_fds = master;
		if( select( fdmax+1, &read_fds, NULL, NULL, NULL ) == -1 )
		{
			perror( "select" );
			exit( 5 );
		}

		for( i = 0; i <= fdmax; i++ )
		{
			if( FD_ISSET( i, &read_fds ) )
			{
				if( i == listener)
				{
					addrlen = sizeof remoteaddr;
					newfd = accept( listener, (struct sockaddr *)&remoteaddr, &addrlen );
					if( newfd == -1 )
					{
						perror( "accept" );
					}
					else
					{
						FD_SET( newfd, &master );
						if( newfd > fdmax )
						{
							fdmax = newfd;
						}
						struct init_struct tmpstruct;
						tmpstruct.maze = maze;
						tmpstruct.player_id = plycount++; 
						send( newfd, &tmpstruct, sizeof( tmpstruct ), 0 );

						printf( "selectserver: new connection from %s on socket %d\n", inet_ntop( remoteaddr.ss_family, get_in_addr(( struct sockaddr* )&remoteaddr ), remoteIP, INET6_ADDRSTRLEN), newfd );
					}

				}
				else
				{
					printf( "Received something.\n" );
					struct player tmp_ply;
					if( ( nbytes = recv( i, &tmp_ply, sizeof( tmp_ply ), 0 ) ) <= 0 )
					{
						if( nbytes == 0 )
						{
							printf( "selectserver: socket %d hung up\n", i );
						}
						else
						{
							perror( "recv" );
						}
						close( i );
						FD_CLR( i, &master );
					}
					else
					{
						printf( "Received position update from player: %d", tmp_ply.index );
						for( int k = 0; k <= fdmax; j++ )
						{
							send( k, &tmp_ply, sizeof( tmp_ply ), 0 ); 
						}
					}
				}
			}
		}
	}

	return 0;
}
