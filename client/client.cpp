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

void * get_in_addr( struct sockaddr *sa )
{
	if( sa->sa_family == AF_INET )
	{
		return &((( struct sockaddr_in * )sa )->sin_addr );
	}

	return &((( struct sockaddr_in * ) sa )->sin_addr );
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

int main( int argc, char *argv[])
{
	maze_param maze;

	int sockfd, numbytes;
	char buf[sizeof( maze )];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if( argc != 2 )
	{
		fprintf( stderr, "Usage: client hostname\n" );
		exit( 1 );
	}

	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if( ( rv = getaddrinfo( argv[1], PORT, &hints, &servinfo ) ) != 0 )
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

	inet_ntop( p->ai_family, get_in_addr( ( struct sockaddr *)p->ai_addr ), s, sizeof( s ) );

	freeaddrinfo( servinfo );
	
	struct init_struct tmpstruct;
	if( ( numbytes = recv( sockfd, &tmpstruct, sizeof(tmpstruct), 0 ) ) == -1 )
	{
		perror( "recv" );
		exit( 1 );
	}
	maze = tmpstruct.maze;
	initscr();
	raw();
	noecho();
	curs_set( 0 );


	
	Entities::Player ply[32];
	int cl_id = tmpstruct.player_id;
	ply[cl_id].pos[0] = 2;
	ply[cl_id].pos[1] = 2;
	Maze::maze_width = maze.maze_width;
	Maze::maze_height= maze.maze_height;
	Maze::maze_seed = maze.maze_seed;

	std::srand( Maze::maze_seed );
	Maze::init_cardinal_points();

	Maze::Cell root = Maze::Cell( NULL, Maze::maze_width / 2, Maze::maze_height / 2 );

	for( int i = 1; i < Maze::maze_width * 2; i++ )
	{
		for( int k = 1; k < Maze::maze_height * 2; k++ )
		{
			if( Maze::grid_path[i][k] == true ) continue;
			int key = 0;
			if( Maze::grid_path[i-1][k] == false )
				key += 1000;

			if( Maze::grid_path[i+1][k] == false )
				key += 100;
			if( Maze::grid_path[i][k-1] == false )
				key += 10;
			if( Maze::grid_path[i][k+1] == false )
				key += 1;

			switch( key )
			{
				case 1:
					mvaddch( i, k, ACS_HLINE );
					break;
				case 10:
					mvaddch( i, k, ACS_HLINE );
					break;
				case 11:
					mvaddch( i, k, ACS_HLINE );
					break;
				case 100:
					mvaddch( i, k, ACS_VLINE );
					break;
				case 101:
					mvaddch( i, k, ACS_ULCORNER );
					break;
				case 110:
					mvaddch( i, k, ACS_URCORNER );
					break;
				case 111:
					mvaddch( i, k, ACS_TTEE );
					break;
				case 1000:
					mvaddch( i, k, ACS_VLINE );
					break;
				case 1001:
					mvaddch( i, k, ACS_LLCORNER );
					break;
				case 1010:
					mvaddch( i, k, ACS_LRCORNER );
					break;
				case 1011:
					mvaddch( i, k, ACS_BTEE );
					break;
				case 1100:
					mvaddch( i, k, ACS_VLINE );
					break;
				case 1101:
					mvaddch( i, k, ACS_LTEE );
					break;
				case 1110:
					mvaddch( i, k, ACS_RTEE );
					break;
				case 1111:
					mvaddch( i, k, ACS_PLUS );
					break;
			}
		}	
	}

	mvaddch( 2, 1, ' ' );
	mvaddch( Maze::maze_width * 2 - 1, Maze::maze_height * 2 - 2, ' ' );
	attron( A_BOLD );
	mvaddstr( Maze::maze_width * 2, 1, "Keys" );
	attroff( A_BOLD );
	mvaddstr( Maze::maze_width * 2 + 1, 1, "Move: hjkl:" );

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
	}

	curs_set( 1 );
	endwin();
	return 0;
}
