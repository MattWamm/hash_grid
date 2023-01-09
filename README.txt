added grid.cpp
game.cpp
	ln 76 - 88		added insertion into grid on init;
	ln 90			added cash of rockets to save on making new rockets;
	ln 99:			moved get route to init
	ln 115			changed get closest enemy to take advantage of hash grid
	ln 240 - 305	made quickhull algorithm to calculate hull
	ln 322			moved hull to different spot to maximize time saved when threaded
	ln 326 - 489	changed collision to take advantage of hash grid
	ln 527 - 555	changed the way the rockets checks for collisions to take advantage of hash grid
	ln 660 - 739	added merge sort for tanks health
terrain.cpp
	ln 131 - 253	added a* algorithm to pathfinding
template.cpp
	ln 21 - 86		changed functions to inline