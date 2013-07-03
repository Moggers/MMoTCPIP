#include <ncurses.h>
#include "cell.cpp"
#include "player.cpp"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "9034"

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
	struct Maze::maze_param maze;
	int player_id;
};

struct ply_update
{
	int index;
	int x;
	int y;
};

int main( int argc, char *argv[])
{
	//Networking variables
	int sockfd, numbytes;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	//List of players
	Entities::Player ply[32];

	//Error checking on hostname input
	if( argc != 2 )
	{
		fprintf( stderr, "Usage: ./client hostname\n" );
		exit( 1 );
	}

	//Prepare hints for getaddrinfo
	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	//Get address info from hostname
	if( ( rv = getaddrinfo( argv[1], PORT, &hints, &servinfo ) ) != 0 )
	{
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( rv ) );
		return 1;
	}

	//Iterate through the list of potential connections and connect to the first available
	for( p = servinfo; p != NULL; p = p->ai_next )
	{
		if( ( sockfd = socket( p->ai_family, p->ai_socktype, p->ai_protocol ) ) == -1 )
		{
			perror( "client: socket" );
			continue;
		}

		if( connect( sockfd, p->ai_addr, p->ai_addrlen ) == -1 )
		{
			close( sockfd);
			perror( "client: connect" );
			continue;
		}

		break;
	}

	if( p == NULL )
	{
		fprintf( stderr, "client: failed to connect\n");
		return 2;
	}
	
	//network to presentation, not sure what this is for
	inet_ntop( p->ai_family, get_in_addr( ( struct sockaddr *)p->ai_addr ), s, sizeof( s ) );

	//Free up the now not needed addressing info.
	freeaddrinfo( servinfo );
		
	//Receive initialization data.
	struct init_struct tmpstruct;
	if( ( numbytes = recv( sockfd, &tmpstruct, sizeof(tmpstruct), 0 ) ) == -1 )
	{
		perror( "recv" );
		exit( 1 );
	}

	//Populate fields with received data
	int cl_id = tmpstruct.player_id;
	ply[cl_id].pos[0] = 2;
	ply[cl_id].pos[1] = 2;
	Maze::maze_width = tmpstruct.maze.maze_width; 
	Maze::maze_height = tmpstruct.maze.maze_width;
	Maze::maze_seed = tmpstruct.maze.maze_width;

	//Prepare the screen for ncurses
	initscr();
	raw();
	noecho();
	curs_set( 0 );

	//Initialize the maze
	Maze::init( );

	//Output help display
	attron( A_BOLD );
	mvaddstr( Maze::maze_width * 2, 1, "Keys" );
	attroff( A_BOLD );
	mvaddstr( Maze::maze_width * 2 + 1, 1, "Move: hjkl:" );
	mvaddstr( Maze::maze_width * 2 + 2, 1, "Quit: q" );
	
	for( ; ; )
	{
		for( int i = 0; i < 32; i++ )
		{
			ply[i].draw();
		}

		refresh();
		char key = getch();
		if( key == 'q' )
		{
			break;
		}
		ply[cl_id].move( key );
		
		struct ply_update update;
		update.index = cl_id;
		update.x = ply[cl_id].pos[0];
		update.y = ply[cl_id].pos[1];
		
		int status = send( sockfd, &update, sizeof( update ), 0 );
		char str[64];
		sprintf( str, "Sent %d bytes", status );
		mvaddstr( Maze::maze_width * 2 + 3, 1, str ); 
		
	}

	close( sockfd );
	curs_set( 1 );
	endwin();
	return 0;
}
