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

	std::vector<int> cardinal_points;
	void init_cardinal_points( void )
	{

		cardinal_points.push_back( 0 );
		cardinal_points.push_back( 1 );
		cardinal_points.push_back( 2 );
		cardinal_points.push_back( 3 );
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
}
#endif
