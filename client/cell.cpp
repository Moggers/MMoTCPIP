#ifndef MAZE
#define MAZE
#include <ncurses.h>
#include <algorithm>
#include <vector>

#define maze_max_width 128
#define maze_max_height 128

namespace Maze
{
	int maze_seed;
	int maze_width;
	int maze_height;
	bool grid_closed[maze_max_width][maze_max_height];
	bool grid_path[maze_max_width * 2][maze_max_height * 2];
	
	struct maze_param
	{
		int maze_seed;
		int maze_width;
		int maze_height;
	};

	std::vector<int> cardinal_points;
	void init_cardinal_points( void )
	{

		cardinal_points.push_back( 0 );
		cardinal_points.push_back( 1 );
		cardinal_points.push_back( 2 );
		cardinal_points.push_back( 3 );
	}

	void draw_cell( int x, int y )
	{
		int key = 0;
		if( grid_path[x-1][y] == false )
			key += 1000;
		if( grid_path[x+1][y] == false )
			key += 100;
		if( grid_path[x][y-1] == false )
			key += 10;
		if( grid_path[x][y+1] == false )
			key += 1;

		switch( key )
		{
			case 1:
				mvaddch( x, y, ACS_HLINE );
				break;
			case 10:
				mvaddch( x, y, ACS_HLINE );
				break;
			case 11:
				mvaddch( x, y, ACS_HLINE );
				break;
			case 100:
				mvaddch( x, y, ACS_VLINE );
				break;
			case 101:
				mvaddch( x, y, ACS_ULCORNER );
				break;
			case 110:
				mvaddch( x, y, ACS_URCORNER );
				break;
			case 111:
				mvaddch( x, y, ACS_TTEE );
				break;
			case 1000:
				mvaddch( x, y, ACS_VLINE );
				break;
			case 1001:
				mvaddch( x, y, ACS_LLCORNER );
				break;
			case 1010:
				mvaddch( x, y, ACS_LRCORNER );
				break;
			case 1011:
				mvaddch( x, y, ACS_BTEE );
				break;
			case 1100:
				mvaddch( x, y, ACS_VLINE );
				break;
			case 1101:
				mvaddch( x, y, ACS_LTEE );
				break;
			case 1110:
				mvaddch( x, y, ACS_RTEE );
				break;
			case 1111:
				mvaddch( x, y, ACS_PLUS );
				break;
		}
	}

	class Cell
	{

		public:
			Cell* parent; 
			Cell* north;
			Cell* south;
			Cell* west;
			Cell* east;
			int * pos;

			Cell( Cell* src_parent, int src_x, int src_y )
			{
				grid_path[src_x * 2][src_y * 2] = true;

				pos = new int[2];
				pos[0] = src_x;
				pos[1] = src_y;
				parent = src_parent;

				std::random_shuffle( cardinal_points.begin(), cardinal_points.end() );
				create_cardinal( cardinal_points.at( 0 ) );
				create_cardinal( cardinal_points.at( 1 ) );
				create_cardinal( cardinal_points.at( 2 ) );
				create_cardinal( cardinal_points.at( 3 ) );
			}

			void create_cardinal( int cardinal_point )
			{
				switch (cardinal_point){
					case 0:
						create_north();
					case 1:
						create_south();
					case 2:
						create_west();
					case 3:
						create_east();
				}
			}

			void create_north( void )
			{
				if( pos[1]-1 == 0 ) return;
				if(grid_closed[pos[0]][pos[1]-1] == false) 
					grid_closed[pos[0]][pos[1]-1] = true;
				else
					return;
				grid_path[ pos[0] * 2 ][ pos[1] * 2 - 1 ] = true;
				north = new Cell( this, pos[0], pos[1] - 1 );
			}

			void create_south( void )
			{
				if( pos[1]+1 == maze_height ) return;
				if(grid_closed[pos[0]][pos[1]+1] == false) 
					grid_closed[pos[0]][pos[1]+1] = true;
				else
					return;
				grid_path[ pos[0] * 2 ][ pos[1] * 2 + 1 ] = true;
				south = new Cell( this, pos[0], pos[1] + 1 );

			}

			void create_west( void )
			{
				if( pos[0]-1 == 0 ) return;
				if(grid_closed[pos[0]-1][pos[1]] == false) 
					grid_closed[pos[0]-1][pos[1]] = true;
				else
					return;
				grid_path[ pos[0] * 2 - 1 ][ pos[1] * 2 ] = true;
				west = new Cell( this, pos[0] - 1, pos[1] );

			}

			void create_east( void )
			{
				if( pos[0]+1 == maze_width ) return;
				if(grid_closed[pos[0]+1][pos[1]] == false) 
					grid_closed[pos[0]+1][pos[1]] = true;
				else
					return;
				grid_path[ pos[0] * 2 + 1 ][ pos[1] * 2 ] = true;
				east =  new Cell( this, pos[0] + 1, pos[1] );

			}
	};

	void init( void )
	{
		std::srand( maze_seed );
		init_cardinal_points();
		Cell root = Maze::Cell( NULL, maze_width / 2, maze_height / 2 );

		for( int i = 1; i < maze_width * 2; i++ )
		{
			for( int k = 1; k < maze_height * 2; k++ )
			{
				if( grid_path[i][k] == true) continue;
				draw_cell( i, k );
			}
		}

		mvaddch( 2, 1, ' ');
		mvaddch( maze_width * 2 - 1, Maze::maze_height * 2 - 2, ' ' );
	}
}
#endif
