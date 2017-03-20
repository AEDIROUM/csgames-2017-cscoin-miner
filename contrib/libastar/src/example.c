/* 

$Id

Copyright (C) 2009 Alexios Chouchoulas

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  



This is an example of how to use libastar.

*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <libastar/astar.h>


#define WIDTH  70
#define HEIGHT 20
#define NUM_WALLS (WIDTH * HEIGHT) / 50


uint8_t map [WIDTH * HEIGHT];


#define MAP_FLOOR 0
#define MAP_WALL  1
#define MAP_ROUTE 2 // We use this to mark the route on the map.
#define MAP_START 3
#define MAP_END   4


// The characters used in print out the map.
char * map_chars = " X.SE";


#define set_map(x, y, val) map[(y) * WIDTH + (x)] = (val)

#define get_map(x, y) map[(y) * WIDTH + (x)]


// Define the map cost getter, a callback which is used by A* to get
// map information from our own representation. The getter inspects
// the map and returns a cost between 0 and 255 denoting how difficult
// it is to walk on this type of map terrain. 0 is the easiest to move
// on, 254 is the most difficult passable terrain, and 255
// (COST_BLOCKED) is impassable. If your map only has 'clear' and
// 'blocked' squares, return 1 (or a low non-zero number) and
// COST_BLOCKED respectively. In this case we use a simple, two value
// map with only perfectly passable and impenetrable blocks.

uint8_t get_map_cost (const uint32_t x, const uint32_t y)
{
	return get_map(x,y) == MAP_WALL ? COST_BLOCKED : 1;
}


static void
map_init()
{
	int i, x, y;

	// First, clear it.
	for (i = 0; i < WIDTH * HEIGHT; i++) {
		map[i] = MAP_FLOOR;
	}

	// Now, fill it with some random walls (one wall per 50 spaces)
	for (i = 0; i < WIDTH * HEIGHT / 3; i++) {
		// Fill random spots with 2-column, 1-row blocks
		x = rand() % (WIDTH - 1);
		y = rand() % HEIGHT;
		// Don't put anything near the start/end positions.
		if (((x <= 6) && (y <= 3)) ||
		    ((x >= (WIDTH - 7)) && (y >= (HEIGHT - 4)))) {
			--i;
			continue;
		}
		set_map (x, y, MAP_WALL);
		set_map (x + 1, y, MAP_WALL);
	}
}


static void
map_print()
{
	int x, y;

	printf ("\n");
	for (y = 0; y < HEIGHT; y++) {
		for (x = 0; x < WIDTH; x++) {
			printf ("%c", map_chars [get_map(x,y)]);
		}
		printf ("\n");
	}
	printf ("\n");
}


int main (int argc, char ** argv)
{
	astar_t * as;
	int x0, y0, x1, y1;
	int result;

	srand(time(NULL) ^ getpid());
  
	// Initialise the map.
	map_init();
	
	// Allocate an A* context.
	as = astar_new (WIDTH, HEIGHT, get_map_cost, NULL);

	// Set the map origin. This allows us to look for a route in
	// an area smaller than the full game map (by setting the
	// origin to the origin of that area, and giving astar_new() a
	// width and height smaller than the full map's. It's
	// mandatory to set this, even if it's just zero. If you don't set it,
	// the algorithm will fail with an ASTAR_ORIGIN_NOT_SET error.

	astar_set_origin (as, 0, 0);
	
	// By setting the steering penalty, you penalise direction
	// changes. This can lead in more direct routes, but it can also make
	// the algorithm work harder to find a route. The default pre-bundled
	// value for this cost is 5 (5 less than the cost of moving in the four
	// cardinal directions). Set it to zero, and the route can meander
	// more.
	
	// astar_set_steering_penalty (as, 5);

	// If you have a preference for movement in the cardinal directions
	// only, assign a high cost to the other four directions. If the only
	// way to get from A to B is to move dianogally, a diagonal move will
	// still be used though.

	// astar_set_cost (as, DIR_NE, 100);
	// astar_set_cost (as, DIR_NW, 100);
	// astar_set_cost (as, DIR_SW, 100);
	// astar_set_cost (as, DIR_SE, 100);

	// Instead, if you want to only route using the four cardinal
	// directions, say this:

	// astar_set_movement_mode (as, DIR_CARDINAL);
	// astar_set_movement_mode (as, DIR_8WAY); // This is the default.
	
	// Starting near the upper left corner of the map.
	x0 = 2;
	y0 = 1;

	// Destination near the lower right corner.
	x1 = WIDTH - 3;
	y1 = HEIGHT - 1;

	// Look for a route.
	result = astar_run (as, x0, y0, x1, y1);

	// What's the result?
	printf ("Route from (%d, %d) to (%d, %d). Result: %s (%d)\n",
		as->x0, as->y0,
		as->x1, as->y1,
		as->str_result, result);
	
	// Do we have a route? We don't care if the route is partial
	// or full here. If you need a full route, you must ensure the return
	// value of astar_run is ASTAR_FOUND. If it isn't, and
	// astar_have_route() returns non-zero, there's a (possibly partial)
	// route.

	if (astar_have_route (as)) {
		direction_t * directions, * dir;
		uint32_t i, num_steps;
		int x, y;

		num_steps = astar_get_directions (as, &directions);

		if (result == ASTAR_FOUND) {
			printf ("We have a route. It has %d step(s).\n", num_steps);
		} else {
			printf ("The best compromise route has %d step(s).\n", num_steps);
		}
		
		// The directions start at our (x0, y0) and give us
		// step-by-step directions (e.g. N, E, NE, N) to
		// either our (x1, y1) in the case of a full route, or
		// the best compromise.
		x = x0;
		y = y0;
		dir = directions;
		for (i = 0; i < num_steps; i++, dir++) {

			// Convince ourselves that A* never made us go through walls.
			assert (get_map (x, y) != MAP_WALL);
			
			// Mark the route on the map.
			set_map (x, y, MAP_ROUTE);

			// Get the next (x, y) co-ordinates of the map.
			x += astar_get_dx (as, *dir);
			y += astar_get_dy (as, *dir);
		}

		// Mark the starting and ending squares on the map. Do this last.
		set_map (x0, y0, MAP_START);
		set_map (x1, y1, MAP_END);

		// VERY IMPORTANT: MEMORY LEAKS OTHERWISE!
		astar_free_directions (directions);
	}

	// Print the map.
	map_print();

	// Dellocate the A* algorithm context.
	astar_destroy (as);
}
