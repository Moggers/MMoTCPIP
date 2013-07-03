#include <ncurses.h>
#include "cell.cpp"

namespace Entities
{
	class Player
	{
		public:
			int * pos;
			int * old_pos;
			Player( void )
			{
				pos = new int[2];
				pos[0] = 2;
				pos[1] = 2;
				old_pos = new int[2];
				old_pos[0] = 2;
				old_pos[1] = 2;
			}
			void draw( void )
			{
				mvaddch( old_pos[0], old_pos[1], ' ' );
				attron( A_BOLD );
				mvaddch( pos[0], pos[1], '@' );
				attroff( A_BOLD );
			}
			void move( char key )
			{
				old_pos[0] = pos[0];
				old_pos[1] = pos[1];

				int new_pos[2];
				new_pos[0] = pos[0];
				new_pos[1] = pos[1];
				switch( key )
				{
					case 'k':
						new_pos[0]--;
						break;
					case 'j':
						new_pos[0]++;
						break;
					case 'h':
						new_pos[1]--;
						break;
					case 'l':
						new_pos[1]++;
						break;
				}
				if( Maze::grid_path[ new_pos[0] ][ new_pos[1] ]  == true )
				{
					pos[0] = new_pos[0];
				pos[1] = new_pos[1];
				}
			}
	};
}
