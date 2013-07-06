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
	//Error check for correct arguments
	if( argc != 4 )
	{
		fprintf( stderr, "Usage: ./server.o width height seed\n" );
		exit( 1 );
	}

	//Create a list of players
	struct player ply[32];
	int plycount = 0;

	//Clear the player list
	memset( ply, 0, sizeof( ply ) );

	//Initialize maze parameters
	maze_param maze;
	maze.maze_seed = atoi( argv[1] );
	maze.maze_width = atoi( argv[2] );
	maze.maze_height = atoi( argv[3] );

	//file descripters for select()
	fd_set master;
	fd_set read_fds;
	int fdmax;

	//file descripter for accepting new connections
	int listener;
	//file descriptor for new connections
	int newfd;
	//Store address for ipv4/6
	struct sockaddr_storage remoteaddr;
	socklen_t addrlen;

	//Storage for length of packet
	int nbytes;
	
	char remoteIP[INET6_ADDRSTRLEN];

	int yes = 1;
	int i, j, rv;

	//Shit for address creation
	struct addrinfo hints, *ai, *p;

	//Clear the file descriptors
	FD_ZERO( &master );
	FD_ZERO( &read_fds );
	
	//Prepare hints
	memset( &hints, 0, sizeof hints );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	//Get the addres info, stick it in ai
	if(( rv = getaddrinfo( NULL, PORT, &hints, &ai )) != 0 )
	{
		fprintf( stderr, "selectserver: %s\n", gai_strerror( rv ) );
		exit( 2 );
	}
	
	//Bind to the first available connection
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
	
	//Free the now not needed address info
	freeaddrinfo( ai );

	if( listen( listener, 10 ) == -1 )
	{
		perror( "listen" );
		exit( 4 );
	}
	
	//Add the listener file descripter to the set of file descripters
	FD_SET( listener, &master );
	
	//Highest becomes listener
	fdmax = listener;

	for( ;; )
	{
		//Copy master into read_fds, select modifies read_fds, we don't want it modifying master
		read_fds = master;
		
		//Get a list of file descripters with packets to recv
		if( select( fdmax+1, &read_fds, NULL, NULL, NULL ) == -1 )
		{
			perror( "select" );
			exit( 5 );
		}

		for( i = 0; i <= fdmax; i++ )
		{
			//If the file descriptor wants our attention
			if( FD_ISSET( i, &read_fds ) )
			{
				//If the file descripter requesting our attention is the listener
				if( i == listener)
				{
					addrlen = sizeof remoteaddr;
					
					//Create a new file descripter
					newfd = accept( listener, (struct sockaddr *)&remoteaddr, &addrlen );
					if( newfd == -1 )
					{
						perror( "accept" );
					}
					else
					{
						//Add it into the list of file descripters
						FD_SET( newfd, &master );
						if( newfd > fdmax )
						{
							fdmax = newfd;
						}
						//Send initialization data
						struct init_struct tmpstruct;
						tmpstruct.maze = maze;
						tmpstruct.player_id = newfd; 
						int status  = send( newfd, &tmpstruct, sizeof( tmpstruct ), 0 );

						printf( "selectserver: new connection from %s on socket %d, we sent the initialization in %d bytes\n", inet_ntop( remoteaddr.ss_family, get_in_addr(( struct sockaddr* )&remoteaddr ), remoteIP, INET6_ADDRSTRLEN), newfd, status );
					}
				}
				else//Handle packets sent by the client
				{
					printf( "Received something.\n" );
					struct player tmp_ply;
					if( ( nbytes = recv( i, &tmp_ply, sizeof( tmp_ply ), 0 ) ) <= 0 ) //Receive packet
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
							send( k, &tmp_ply, sizeof( tmp_ply ), 0 ); //Forward it to everyone else
						}
					}
				}
			}
		}
	}

	return 0;
}
