#include <pthread.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define PORT "9034"

namespace clthread
{
	//Store maze infomation for server and clients
	struct maze_param
	{
		int seed;
		int width;
		int height;
	};
	//Sent to the client when they connect
	struct init_struct
	{
		int seed;
		int width;
		int height;
		int player_id;
	};
	//Received from client and forwarded to all other clients
	struct player_update
	{
		int index;
		int x;
		int y;
	};

	//Input for pthread_create on beginthread
	struct thread_param
	{
		int sockfd;
	};

	//List of players
	struct player_update ply_list[32];
	
	//Threads and thread count
	pthread_t clients[32];
	int thread_count;

	//Global level instantiation of maze data
	maze_param server_maze;
	//Set of clients as socket file descriptors 
	fd_set master;
	int maxfd;

	//Prepare maze, networking, and threads
	void initialize_threads( int src_maze_seed, int src_maze_width, int src_maze_height )
	{
		memset( &clients, 0, sizeof( clients ) );
		memset( ply_list, 0, sizeof( ply_list ) );

		server_maze.seed = src_maze_seed;
		server_maze.width = src_maze_width;
		server_maze.height = src_maze_height;
		maxfd = 0;
		FD_ZERO( &master );
		thread_count = 0;
	}

	//PThread method, one for each client
	void * beginthread( void * param )
	{
		int sockfd = ((struct thread_param*)param)->sockfd;
		init_struct cl_init;
		
		cl_init.seed = server_maze.seed;
		cl_init.width = server_maze.width;
		cl_init.height = server_maze.height;
		cl_init.player_id = sockfd;

		int status;

		if( (status = send( sockfd, &cl_init, sizeof( cl_init ), 0 ) ) != 16 )
		{
			printf( "Failed to send initialization data to new connected client.\n" );
			pthread_exit( param );
		}
		printf( "Sent the initialization data to newly connected client in %d bytes.\n", status );
		FD_SET( sockfd, &master );
		maxfd = ( sockfd > maxfd ) ? sockfd : maxfd;
		
		for( ;; )
		{
			player_update ply_u;
			if( status = recv( sockfd, &ply_u, sizeof( ply_u ), 0 ) <= 0 )
			{
				if( status == 0 )
				{
					printf( "Client disconnected.\n" );
					close( sockfd );
					FD_CLR( sockfd, &master );
					pthread_exit( param );
				}
				if( status == -1 )
				{
					printf( "Something's gone wrong when receiving a packet from the client.\n" );
					close( sockfd );
					FD_CLR( sockfd, &master );
					pthread_exit( param );
				}
			}	
			printf( "Received a packet measuring %d bytes from player %d containing:\nPlayer: %d\nX: %d\ny: %d\n", status, sockfd, ply_u.index, ply_u.x, ply_u.y );
			if( ply_u.x == -1 )
			{
				printf( "Client disconnected.\n" );
				close( sockfd );
				FD_CLR( sockfd, &master );
				pthread_exit( param );
			}

			int i;
			for( i = 0; i <= maxfd; i++ )
			{
				if( !FD_ISSET( i, &master ) ) continue;
				if( ( status = send( i, &ply_u, sizeof( ply_u ), 0 ) ) == -1 )
				{
					printf( "Something went wrong while sending an update packet to a client.\n" );
					close( i );
					FD_CLR( i, &master );
					pthread_exit( param );
				}
			}
		}
	}
}
