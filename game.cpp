#include "precomp.h" // include (only) this in every .cpp file

constexpr auto num_tanks_blue = 2048;
constexpr auto num_tanks_red = 2048;

constexpr auto tank_max_health = 1000;
constexpr auto rocket_hit_value = 60;
constexpr auto particle_beam_hit_value = 50;

constexpr auto tank_max_speed = 1.0;

constexpr auto health_bar_width = 70;

constexpr auto max_frames = 2000;

//Global performance timer
constexpr auto REF_PERFORMANCE = 108894; //UPDATE THIS WITH YOUR REFERENCE PERFORMANCE (see console after 2k frames)
static timer perf_timer;
static float duration;

//Load sprite files and initialize sprites
static Surface* tank_red_img = new Surface("assets/Tank_Proj2.png");
static Surface* tank_blue_img = new Surface("assets/Tank_Blue_Proj2.png");
static Surface* rocket_red_img = new Surface("assets/Rocket_Proj2.png");
static Surface* rocket_blue_img = new Surface("assets/Rocket_Blue_Proj2.png");
static Surface* particle_beam_img = new Surface("assets/Particle_Beam.png");
static Surface* smoke_img = new Surface("assets/Smoke.png");
static Surface* explosion_img = new Surface("assets/Explosion.png");

static Sprite tank_red(tank_red_img, 12);
static Sprite tank_blue(tank_blue_img, 12);
static Sprite rocket_red(rocket_red_img, 12);
static Sprite rocket_blue(rocket_blue_img, 12);
static Sprite smoke(smoke_img, 4);
static Sprite explosion(explosion_img, 9);
static Sprite particle_beam_sprite(particle_beam_img, 3);

const static vec2 tank_size(7, 9);
const static vec2 rocket_size(6, 6);

const static float tank_radius = 3.f;
const float col_squared_len = (tank_radius + tank_radius) * (tank_radius + tank_radius);
const static float rocket_radius = 5.f;

//threading
static const auto processor_count = std::thread::hardware_concurrency();
ThreadPool pool{processor_count * 2};

//tank directional check bools
bool N = false, E = false, S = false, W = false;

// -----------------------------------------------------------
// Initialize the simulation state
// This function does not count for the performance multiplier
// (Feel free to optimize anyway though ;) )
// -----------------------------------------------------------
void Game::init()
{
	frame_count_font = new Font("assets/digital_small.png", "ABCDEFGHIJKLMNOPQRSTUVWXYZ:?!=-0123456789.");

	
	tanks.reserve(num_tanks_blue + num_tanks_red);

	uint max_rows = 24;

	float start_blue_x = tank_size.x + 40.0f; //47
	float start_blue_y = tank_size.y + 30.0f;

	float start_red_x = 1088.0f;			  //1088
	float start_red_y = tank_size.y + 30.0f;

	float spacing = 7.5f;
	grid = new Grid(10, 10);

	//Spawn blue tanks
	for (int i = 0; i < num_tanks_blue; i++) //N
	{
		vec2 position{ start_blue_x + ((i % max_rows) * spacing), start_blue_y + ((i / max_rows) * spacing) };
		tanks.push_back(Tank(position.x, position.y, BLUE, &tank_blue, &smoke, 1100.f, position.y + 16, tank_radius, tank_max_health, tank_max_speed));
		grid->InsertObject(&tanks.back());
	}
	//Spawn red tanks
	for (int i = 0; i < num_tanks_red; i++) //N
	{
		vec2 position{ start_red_x + ((i % max_rows) * spacing), start_red_y + ((i / max_rows) * spacing) };
		tanks.push_back(Tank(position.x, position.y, RED, &tank_red, &smoke, 100.f, position.y + 16, tank_radius, tank_max_health, tank_max_speed));
		grid->InsertObject(&tanks.back());
	}

	particle_beams.push_back(Particle_beam(vec2(590, 327), vec2(100, 50), &particle_beam_sprite, particle_beam_hit_value));
	particle_beams.push_back(Particle_beam(vec2(64, 64), vec2(100, 50), &particle_beam_sprite, particle_beam_hit_value));
	particle_beams.push_back(Particle_beam(vec2(1200, 600), vec2(100, 50), &particle_beam_sprite, particle_beam_hit_value));
}														//dont care

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::shutdown()
{
}

// -----------------------------------------------------------
// Iterates through all tanks and returns the closest enemy tank for the given tank
// -----------------------------------------------------------
Tank& Game::find_closest_enemy(Tank& current_tank)
{
	//very inefficient but better than nothing
	
	Tank* closestTank{};
	float closest_distance = numeric_limits<float>::infinity();
	float closest_cell_distance = numeric_limits<float>::infinity();
	int closest_index = 0;
	vec2 furthestPossible{ 0,0 };
	GridCell* tankCell = grid->GetGridLocation(current_tank.hash);
	float mininumDistance = fabsf((vec2{ 0,0 } - vec2{grid->cellSize.x*3, grid->cellSize.y*3}).sqr_length());


	vec2 closestHere{ numeric_limits<float>::infinity(), numeric_limits<float>::infinity() };


	for (int i = 0; i < grid->Size(); i++)
	{
		GridCell* cell = grid->GetGridLocation(i);
		if (current_tank.allignment == BLUE && cell->red->empty())
				continue;
		else if (current_tank.allignment == RED && cell->blue->empty())
				continue;
		
		
		//gets the furthest position in the closest cell
		closestHere = cell->startPosition;
		closestHere += current_tank.position.x < cell->startPosition.x ? grid->cellSize.x : current_tank.position.x > cell->startPosition.x ? 0 : (grid->cellSize.x / 2);
		closestHere += current_tank.position.y < cell->startPosition.y ? 0 : current_tank.position.y > cell->startPosition.y ? grid->cellSize.y : (grid->cellSize.y / 2);

		float sqr_dist = fabsf((closestHere - current_tank.position).sqr_length());

		if (closest_cell_distance < mininumDistance)
		{
			closest_cell_distance = mininumDistance;
			break;
		}
		else
		{

			if (sqr_dist > closest_cell_distance)
				continue;
			closest_cell_distance = sqr_dist;
		}


		//technically the closest cell with enemies but the furthest possible position within that cell (close enough)
		
	}


	for (int i = 0; i < grid->Size(); i++)																											//N
	{

		GridCell* cell = grid->GetGridLocation(i);

		if (current_tank.allignment == BLUE && cell->red->empty())
			continue;
		else if (current_tank.allignment == RED && cell->blue->empty())
			continue;

		//previous statement but instead getting the closest position possible
		
		furthestPossible = cell->startPosition;
		furthestPossible += current_tank.position.x > cell->startPosition.x ? grid->cellSize.x : current_tank.position.x < cell->startPosition.x ? 0 : (grid->cellSize.x / 2);
		furthestPossible += current_tank.position.y > cell->startPosition.y ? 0 : current_tank.position.y < cell->startPosition.y ? grid->cellSize.y : (grid->cellSize.y / 2);



		float sqr_dist = fabsf((furthestPossible - current_tank.position).sqr_length());
		if (sqr_dist >= closest_cell_distance)
			continue;


		vector<Tank*>* checkList = current_tank.allignment == BLUE ? cell->red : cell->blue;

		for (Tank* disTank : *checkList)
		{
			if (!disTank->active)
				continue;


			float sqr_dist = fabsf((disTank->position - current_tank.position).sqr_length());


			if (sqr_dist < closest_distance)
			{
				closest_distance = sqr_dist;
				closestTank = disTank;
			}
		
		
		}
	}


	return *closestTank ;
}
//Checks if a point lies on the left of an arbitrary angled line
bool Tmpl8::Game::left_of_line(vec2 line_start, vec2 line_end, vec2 point)
{
	return ((line_end.x - line_start.x) * (point.y - line_start.y) - (line_end.y - line_start.y) * (point.x - line_start.x)) < 0;					//1
}

// -----------------------------------------------------------
// Update the game state:
// Move all objects
// Update sprite frames
// Collision detection
// Targeting etc..
// -----------------------------------------------------------

void Game::route_tank(Tank& tank)
{
	tank.set_route(background_terrain.get_route(tank, tank.target));
}



void Game::update(float deltaTime)
{
	//Calculate the route to the destination for each tank using BFS
	//Initializing routes here so it gets counted for performance..
	if (frame_count == 0)																															//loops once on startup
	{
		for (Tank& t : tanks)																														//N
		{
			t.set_route(background_terrain.get_route(t, t.target));
		}
		//Check tank collision and nudge tanks away from each other											
	}

	Rectangle2D red = Rectangle2D();
	Rectangle2D blue = Rectangle2D();
	bool rSet = false;
	bool bSet = false;
	//TODO:
	//THREADING

	

		for (Tank& tank : tanks) // collision																											//N2
		{
			if (tank.active)
			{
				int tankHash = grid->GetHash(&tank);
				auto cell = grid->GetGridLocation(tankHash); //gets the list of current cell
				float col_Squared = tank.collision_radius + tank.collision_radius;

				bool N = false, S = false, E = false, W = false;

				N = abs(tank.position.y - cell->startPosition.y) < tank.collision_radius;
				S = N ? false : abs(tank.position.y - (cell->startPosition.y + grid->cellSize.y)) < tank.collision_radius;
				W = abs(tank.position.x - cell->startPosition.x) < tank.collision_radius;
				E = W ? false : abs(tank.position.y - (cell->startPosition.x + grid->cellSize.x)) < tank.collision_radius;
				//checks the sides the tank is close enough to collide with

				for (Tank* other_tank : (*cell->tankList))																											//N
				{
					if (&tank == other_tank || !other_tank->active) continue;
					vec2 dir = vec2(tank.position.x - other_tank->position.x, tank.position.y - other_tank->position.y);

					if (dir.sqr_length() < col_squared_len)
					{
						tank.push(dir.normalized(), 1.f);
					}
				}
#pragma region calculate sides
				if (N)
				{
					if (tankHash + grid->setWidth < grid->numberCells) {
						GridCell* NCell = grid->GetGridLocation(tankHash + grid->setWidth);

						for (Tank* other_tank : (*NCell->tankList))																											//N
						{
							if (&tank == other_tank || !other_tank->active) continue;
							vec2 dir = vec2(tank.position.x - other_tank->position.x, tank.position.y - other_tank->position.y);

							if (dir.sqr_length() < col_squared_len)
							{
								tank.push(dir.normalized(), 1.f);
							}
						}

					}
				}
				if (S)
				{
					if (tankHash - grid->setWidth >= 0)
					{
						GridCell* NCell = grid->GetGridLocation(tankHash - grid->setWidth);
						for (Tank* other_tank : (*NCell->tankList))																											//N
						{
							if (&tank == other_tank || !other_tank->active) continue;
							vec2 dir = vec2(tank.position.x - other_tank->position.x, tank.position.y - other_tank->position.y);

							if (dir.sqr_length() < col_squared_len)
							{
								tank.push(dir.normalized(), 1.f);
							}
						}
					}
				}
				if (W)
				{
					if (tankHash - 1 > 0 && (tankHash - 1) % grid->setWidth != 0) {

						GridCell* NCell = grid->GetGridLocation(tankHash - 1);
						for (Tank* other_tank : (*NCell->tankList))																											//N
						{
							if (&tank == other_tank || !other_tank->active) continue;
							vec2 dir = vec2(tank.position.x - other_tank->position.x, tank.position.y - other_tank->position.y);

							if (dir.sqr_length() < col_squared_len)
							{
								tank.push(dir.normalized(), 1.f);
							}
						}
					}
				}
				if (E)
				{
					if (tankHash + 1 < grid->numberCells && (tankHash + 1) % grid->setWidth != 0)
					{
						GridCell* NCell = grid->GetGridLocation(tankHash + 1);
						for (Tank* other_tank : (*NCell->tankList))																											//N
						{
							if (&tank == other_tank || !other_tank->active) continue;
							vec2 dir = vec2(tank.position.x - other_tank->position.x, tank.position.y - other_tank->position.y);

							if (dir.sqr_length() < col_squared_len)
							{
								tank.push(dir.normalized(), 1.f);
							}
						}
					}
				}
				if (N && E)
				{
					if (tankHash - grid->cellSize.x - 1 >= 0) {
						GridCell* NCell = grid->GetGridLocation(tankHash - (grid->setWidth - 1));
						for (Tank* other_tank : (*NCell->tankList))																											//N
						{
							if (&tank == other_tank || !other_tank->active) continue;
							vec2 dir = vec2(tank.position.x - other_tank->position.x, tank.position.y - other_tank->position.y);

							if (dir.sqr_length() < col_squared_len)
							{
								tank.push(dir.normalized(), 1.f);
							}
						}
					}
				}
				if (N && W)
				{
					if (tankHash - (grid->setWidth + 1) >= 0) {
						GridCell* NCell = grid->GetGridLocation(tankHash - (grid->setWidth + 1));
						for (Tank* other_tank : (*NCell->tankList))																											//N
						{
							if (&tank == other_tank || !other_tank->active) continue;
							vec2 dir = vec2(tank.position.x - other_tank->position.x, tank.position.y - other_tank->position.y);

							if (dir.sqr_length() < col_squared_len)
							{
								tank.push(dir.normalized(), 1.f);
							}
						}
					}
				}
				if (S && E)
				{
					if (tankHash + (grid->setWidth + 1) < grid->numberCells) {
						GridCell* NCell = grid->GetGridLocation(tankHash + (grid->setWidth + 1));
						for (Tank* other_tank : (*NCell->tankList))																											//N
						{
							if (&tank == other_tank || !other_tank->active) continue;
							vec2 dir = vec2(tank.position.x - other_tank->position.x, tank.position.y - other_tank->position.y);

							if (dir.sqr_length() < col_squared_len)
							{
								tank.push(dir.normalized(), 1.f);
							}
						}
					}
				}
				if (S && W)
				{
					if (tankHash + (grid->setWidth - 1) < grid->numberCells)
					{
						GridCell* NCell = grid->GetGridLocation(tankHash + (grid->setWidth - 1));
						for (Tank* other_tank : (*NCell->tankList))																											//N
						{
							if (&tank == other_tank || !other_tank->active) continue;
							vec2 dir = vec2(tank.position.x - other_tank->position.x, tank.position.y - other_tank->position.y);

							if (dir.sqr_length() < col_squared_len)
							{
								tank.push(dir.normalized(), 1.f);
							}
						}
					}
				}
#pragma endregions


			}
		
		

	}
	
	for (Smoke& smoke : smokes)																														//N
	{
		smoke.tick();
	}
	
	
	
	//Update tanks
	for (Tank& tank : tanks)																														//N
	{
		if (tank.active)
		{
			//Move tanks according to speed and nudges (see above) also reload
			tank.tick(background_terrain);
			int checkHash = grid->GetHash(&tank);
			if (tank.hash != checkHash)
			{
				grid->MoveObject(&tank, checkHash);
			}
			//Shoot at closest target if reloaded
			if (tank.rocket_reloaded())
			{
				Tank& target = find_closest_enemy(tank);

				rockets.push_back(Rocket(tank.position, (target.get_position() - tank.position).normalized() * 3, rocket_radius, tank.allignment, ((tank.allignment == RED) ? &rocket_red : &rocket_blue)));

				tank.reload_rocket();
			}
		}
	}


	//Calculate "forcefield" around active tanks
	forcefield_hull.clear();

	for (Rocket& rocket : rockets)																												//N*M
	{
		if (rocket.active)
		{
			for (size_t i = 0; i < forcefield_hull.size(); i++)
			{
				if (circle_segment_intersect(forcefield_hull.at(i), forcefield_hull.at((i + 1) % forcefield_hull.size()), rocket.position, rocket.collision_radius))
				{
					explosions.push_back(Explosion(&explosion, rocket.position));
					rocket.active = false;
				}
			}
		}
	}

	auto f2 = pool.enqueue([=] {
		for (Explosion& explosion : explosions)																										//N
		{
			rockets.erase(std::remove_if(rockets.begin(), rockets.end(), [](const Rocket& rocket) { return !rocket.active; }), rockets.end());
			explosion.tick();
		}

		explosions.erase(std::remove_if(explosions.begin(), explosions.end(), [](const Explosion& explosion) { return explosion.done(); }), explosions.end());
	});

	
	//Update particle beams
	for (Particle_beam& particle_beam : particle_beams)																							//N*M
	{
		particle_beam.tick(tanks);

		//Damage all tanks within the damage window of the beam (the window is an axis-aligned bounding box)
		for (Tank& tank : tanks)
		{
			if (tank.active && particle_beam.rectangle.intersects_circle(tank.get_position(), tank.get_collision_radius()))
			{
				if (tank.hit(particle_beam.damage))
				{
					smokes.push_back(Smoke(smoke, tank.position - vec2(0, 48)));
				}
			}
		}
	}
	f2.get();

	for (Rocket& rocket : rockets)																												//N*M*N
	{
		rocket.tick();
		if (!rocket.active)
			continue;
		//Check if rocket collides with enemy tank, spawn explosion, and if tank is destroyed spawn a smoke plume


		//this code has to do with the algorithm i employed to split up the tanks in buckets. this makes it very hard to multithread due to read access violations
		for (Tank* tank : (*grid->GetGridLocation(grid->hashObject(rocket.position.x, rocket.position.y))->tankList))
		{
			if (tank->active && (tank->allignment != rocket.allignment) && rocket.intersects(tank->position, tank->collision_radius))
			{
				explosions.push_back(Explosion(&explosion, tank->position));

				if (tank->hit(rocket_hit_value))
				{
					smokes.push_back(Smoke(smoke, tank->position - vec2(7, 24)));
				}

				rocket.active = false;
				break;
			}
		}
	}


}

// -----------------------------------------------------------
// Draw all sprites to the screen
// (It is not recommended to multi-thread this function)
// -----------------------------------------------------------
void Game::draw()
{
	// clear the graphics window
	screen->clear(0);

	//Draw background
	background_terrain.draw(screen);

	//Draw sprites
	for (int i = 0; i < num_tanks_blue + num_tanks_red; i++)																					//N
	{
		tanks.at(i).draw(screen);

		vec2 tank_pos = tanks.at(i).get_position();
	}

	for (Rocket& rocket : rockets)																												//N
	{
		rocket.draw(screen);
	}

	for (Smoke& smoke : smokes)																													//N
	{
		smoke.draw(screen);
	}

	for (Particle_beam& particle_beam : particle_beams)																							//N
	{
		particle_beam.draw(screen);
	}

	for (Explosion& explosion : explosions)																										//N
	{
		explosion.draw(screen);
	}

	//Draw forcefield (mostly for debugging, its kinda ugly..)
	//for (size_t i = 0; i < forcefield_hull.size(); i++) //N
	//{
	//	vec2 line_start = forcefield_hull.at(i);
	//	vec2 line_end = forcefield_hull.at((i + 1) % forcefield_hull.size());
	//	line_start.x += HEALTHBAR_OFFSET;
	//	line_end.x += HEALTHBAR_OFFSET;
	//	screen->line(line_start, line_end, 0x0000ff);
	//}

	//Draw sorted health bars
	for (int t = 0; t < 2; t++)																													//2
	{
		const int NUM_TANKS = ((t < 1) ? num_tanks_blue : num_tanks_red);

		const int begin = ((t < 1) ? 0 : num_tanks_blue);
		std::vector<Tank*> sorted_tanks(tanks.size());
		transform(tanks.begin(), tanks.end(), sorted_tanks.begin(), [](Tank& t) { return &t; }); //puts all the tanks in original in sorted tanks as pointers to make merge sorting easier

		merge_sort_tanks_health(sorted_tanks, 0, sorted_tanks.size() - 1, 0);

		sorted_tanks.erase(std::remove_if(sorted_tanks.begin(), sorted_tanks.end(), [](Tank* tank) { return !tank->active; }), sorted_tanks.end());

		draw_health_bars(sorted_tanks, t);
	}
}

// -----------------------------------------------------------
// Sort tanks by health value using insertion sort							ORIGINAL
// -----------------------------------------------------------
void Tmpl8::Game::insertion_sort_tanks_health(const std::vector<Tank>& original, std::vector<const Tank*>& sorted_tanks, int begin, int end)
{
	const int NUM_TANKS = end - begin;
	sorted_tanks.reserve(NUM_TANKS);
	sorted_tanks.emplace_back(&original.at(begin));

	for (int i = begin + 1; i < (begin + NUM_TANKS); i++)																					//N*M
	{
		const Tank& current_tank = original.at(i);

		for (int s = (int)sorted_tanks.size() - 1; s >= 0; s--)
		{
			const Tank* current_checking_tank = sorted_tanks.at(s);

			if ((current_checking_tank->compare_health(current_tank) <= 0))
			{
				sorted_tanks.insert(1 + sorted_tanks.begin() + s, &current_tank);
				break;
			}

			if (s == 0)
			{
				sorted_tanks.insert(sorted_tanks.begin(), &current_tank);
				break;
			}
		}
	}
}

//merge sort tanks health											THIS VERSION

void Tmpl8::Game::merge_sort_tanks_health(std::vector<Tank*>& sorted_tanks, int begin, int end, int depth)
{
	
	if (begin < end)
	{

		int middle = floor(begin + (end - begin) / 2);
		if (pow(2, depth) <= std::thread::hardware_concurrency())
		{
			//this one creates a read access violation when threaded

			merge_sort_tanks_health(sorted_tanks, begin, middle, depth + 1);

			merge_sort_tanks_health(sorted_tanks, middle + 1, end, depth + 1);
		}
		else
		{
			merge_sort_tanks_health(sorted_tanks, begin, middle, depth + 1);
			merge_sort_tanks_health(sorted_tanks, middle + 1, end, depth + 1);
		}

		merge(sorted_tanks, begin, middle, end);

	}

}

void Game::merge(vector<Tank*>& sorted_tanks, int start, int middle, int end) {

	// Create L ← A[p..q] and M ← A[q+1..r]
	
	int n1 = middle - start + 1;
	int n2 = end - middle;

	std::size_t const half_size = middle;
	std::size_t const end_size = end;
	vector<Tank*> L(n1);
	for (int i = 0; i < n1; i++)
	{
		L.at(i) = sorted_tanks.at(i + start);
	}

	vector<Tank*> R(n2);
	for (int i = 0; i < n2; i++)
	{
		R.at(i) = sorted_tanks.at(i + middle + 1);
	}

	// Maintain current index of sub-arrays and main array
	int i = 0, j = 0, k = start;

	// Until we reach either end of either L or M, pick larger among
	// elements L and M and place them in the correct position at A[p..r]
	while (i < n1 && j < n2) {
		if (L.at(i)->health <= R.at(j)->health)
		{
			sorted_tanks.at(k) = L.at(i);
			i++;
		}
		else
		{
			sorted_tanks.at(k) = R.at(j);
			j++;
		}
		k++;
	}

	// When we run out of elements in either L or M,
	// pick up the remaining elements and put in A[p..r]
	while (i < n1) {
		sorted_tanks.at(k) = L.at(i);
		i++;
		k++;
	}

	while (j < n2) {
		sorted_tanks.at(k) = R.at(j);
		j++;
		k++;
	}
}

// -----------------------------------------------------------
// Draw the health bars based on the given tanks health values
// -----------------------------------------------------------
void Tmpl8::Game::draw_health_bars(const std::vector<Tank*>& sorted_tanks, const int team)
{
	int health_bar_start_x = (team < 1) ? 0 : (SCRWIDTH - HEALTHBAR_OFFSET) - 1;
	int health_bar_end_x = (team < 1) ? health_bar_width : health_bar_start_x + health_bar_width - 1;

	for (int i = 0; i < SCRHEIGHT - 1; i++) //N
	{
		//Health bars are 1 pixel each
		int health_bar_start_y = i * 1;
		int health_bar_end_y = health_bar_start_y + 1;

		screen->bar(health_bar_start_x, health_bar_start_y, health_bar_end_x, health_bar_end_y, REDMASK);
	}

	//Draw the <SCRHEIGHT> least healthy tank health bars
	int draw_count = std::min(SCRHEIGHT, (int)sorted_tanks.size());
	for (int i = 0; i < draw_count - 1; i++) //N
	{
		//Health bars are 1 pixel each
		int health_bar_start_y = i * 1;
		int health_bar_end_y = health_bar_start_y + 1;

		float health_fraction = (1 - ((double)sorted_tanks.at(i)->health / (double)tank_max_health));

		if (team == 0) { screen->bar(health_bar_start_x + (int)((double)health_bar_width * health_fraction), health_bar_start_y, health_bar_end_x, health_bar_end_y, GREENMASK); }
		else { screen->bar(health_bar_start_x, health_bar_start_y, health_bar_end_x - (int)((double)health_bar_width * health_fraction), health_bar_end_y, GREENMASK); }
	}
}

// -----------------------------------------------------------
// When we reach max_frames print the duration and speedup multiplier
// Updating REF_PERFORMANCE at the top of this file with the value
// on your machine gives you an idea of the speedup your optimizations give
// -----------------------------------------------------------
void Tmpl8::Game::measure_performance()
{
	char buffer[128];
	if (frame_count >= max_frames)
	{
		if (!lock_update)
		{
			duration = perf_timer.elapsed();
			cout << "Duration was: " << duration << " (Replace REF_PERFORMANCE with this value)" << endl;
			lock_update = true;
		}

		frame_count--;
	}

	if (lock_update)
	{
		screen->bar(420 + HEALTHBAR_OFFSET, 170, 870 + HEALTHBAR_OFFSET, 430, 0x030000);
		int ms = (int)duration % 1000, sec = ((int)duration / 1000) % 60, min = ((int)duration / 60000);
		sprintf(buffer, "%02i:%02i:%03i", min, sec, ms);
		frame_count_font->centre(screen, buffer, 200);
		sprintf(buffer, "SPEEDUP: %4.1f", REF_PERFORMANCE / duration);
		frame_count_font->centre(screen, buffer, 340);
	}
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::tick(float deltaTime)
{
	if (!lock_update)
	{
		update(deltaTime);
	}
	draw();
	measure_performance();

	// print something in the graphics window
	//screen->Print("hello world", 2, 2, 0xffffff);

	// print something to the text window
	//cout << "This goes to the console window." << std::endl;

	//Print frame count
	frame_count++;
	string frame_count_string = "FRAME: " + std::to_string(frame_count);
	frame_count_font->print(screen, frame_count_string.c_str(), 350, 580);
}
