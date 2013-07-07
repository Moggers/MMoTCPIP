#include <ncurses.h>
#include "cell.cpp"
#include "player.cpp"
#include "net.cpp"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

//Convert sockaddr into a pointer for ipv4/v6(This line is fucked, it converts ipv4/ipv6 into ipv4.
void * get_in_addr( struct sockaddr *sa )
{
	if( sa->sa_family == AF_INET )
	{
		return &((( struct sockaddr_in * )sa )->sin_addr );
	}

	return &((( struct sockaddr_in6 * ) sa )->sin6_addr );
}

//On connection, server will send this struct to the client, maze_param is used to generate the maze, and player_id is an identifier to know which of the players is us.
struct init_struct
{
	int seed;
	int width;
	int height;
	int player_id;
};

int main( int argc, char *argv[])
{
	//Networking variables
	int sockfd, numbytes;
	int cl_id;

	//Error checking on hostname input
	if( argc != 2 )
	{
		fprintf( stderr, "Usage: ./client.o hostname\n" );
		exit( 1 );
	}

	//Connect
	sockfd = network::connect( argv[2] );
			
	//Receive initialization data.
	struct init_struct tmpstruct;
	if( ( numbytes = recv( sockfd, &tmpstruct, sizeof(tmpstruct), 0 ) ) == -1 )
	{
		perror( "recv" );
		exit( 1 );
	}
	//Populate fields with received data
	cl_id = tmpstruct.player_id;
	Entities::ply_list[cl_id].pos[0] = 2;
	Entities::ply_list[cl_id].pos[1] = 2;
	Maze::maze_width = tmpstruct.width; 
	Maze::maze_height = tmpstruct.height;
	Maze::maze_seed = tmpstruct.seed;
	printf( "Received the following initialization:\nSeed: %d\nHeight: %d\nWidth: %d\n", Maze::maze_seed, Maze::maze_height, Maze::maze_width );

	//Prepare the screen for ncurses
	initscr();
	raw();
	noecho();

	//Initialize the maze
	Maze::init();

	//Disable blocking
	timeout(1);

	//Output help display
	mvprintw( Maze::maze_width * 2, 1, "You are player: %d", cl_id );
	attron( A_BOLD );
	mvaddstr( Maze::maze_width * 2 + 1, 1, "Keys" );
	attroff( A_BOLD );
	mvaddstr( Maze::maze_width * 2 + 2, 1, "Move: hjkl" );
	mvaddstr( Maze::maze_width * 2 + 3, 1, "Quit: q" );


	pthread_create( &network::servthread, NULL, network::beginthread, NULL );

	curs_set( 0 );
	//Main loop
	for( ; ; )
	{
		char key = getch();
		if( key == 'q' )
		{
			break;
		}
		if( key == 'h' || key == 'j' || key == 'k' || key == 'l' )
		{
			Entities::ply_list[cl_id].move( key );
			Entities::ply_update send_update;
			send_update.index = cl_id;
			send_update.x = Entities::ply_list[cl_id].pos[0];
			send_update.y = Entities::ply_list[cl_id].pos[1];
			send( sockfd, &send_update, sizeof( send_update ), 0 );
		}

		for( int i = 0; i < 32; i++ )
		{
			Entities::ply_list[i].draw( ( ( i == cl_id ) ? true : false ) );
		}

		refresh();
	}
	pthread_cancel( network::servthread );
	Entities::ply_update close_notice;
	close_notice.index = cl_id;
	close_notice.x = -1;
	send( sockfd, &close_notice, sizeof( close_notice ), 0 );
	close( sockfd );
	curs_set( 1 );
	endwin();
	return 0;
}
