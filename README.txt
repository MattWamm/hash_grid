added grid.cpp
game.cpp
	ln 74 - 86		added insertion into grid on init;
	ln 93:			moved get route to init
	ln 110			changed get closest enemy to take advantage of hash grid
	ln 203 - 269	made quickhull algorithm to calculate hull
	ln 286 - 450	changed collision to take advantage of hash grid
	ln 486			calls the quickhull algorithm
	ln 527 - 555	changed the way the rockets checks for collisions to take advantage of hash grid
	ln 660 - 739	added merge sort for tanks health
terrain.cpp
	ln 131 - 253	added a* algorithm to pathfinding
template.cpp
	ln 21 - 86		changed functions to inline
TODO:
	remove inactive tanks from the list and draw them seperately so the checks dont have to go through them
	multithread the entirety of the project
	