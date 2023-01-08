#include "precomp.h"
#include "terrain.h"

namespace fs = std::filesystem;
namespace Tmpl8
{
	Terrain::Terrain()
	{
		//Load in terrain sprites
		grass_img = std::make_unique<Surface>("assets/tile_grass.png");
		forest_img = std::make_unique<Surface>("assets/tile_forest.png");
		rocks_img = std::make_unique<Surface>("assets/tile_rocks.png");
		mountains_img = std::make_unique<Surface>("assets/tile_mountains.png");
		water_img = std::make_unique<Surface>("assets/tile_water.png");


		tile_grass = std::make_unique<Sprite>(grass_img.get(), 1);
		tile_forest = std::make_unique<Sprite>(forest_img.get(), 1);
		tile_rocks = std::make_unique<Sprite>(rocks_img.get(), 1);
		tile_water = std::make_unique<Sprite>(water_img.get(), 1);
		tile_mountains = std::make_unique<Sprite>(mountains_img.get(), 1);


		//Load terrain layout file and fill grid based on tiletypes
		fs::path terrain_file_path{ "assets/terrain.txt" };
		std::ifstream terrain_file(terrain_file_path);

		if (terrain_file.is_open()) // 1
		{
			std::string terrain_line;

			std::getline(terrain_file, terrain_line);
			std::istringstream lineStream(terrain_line);

			int rows;

			lineStream >> rows;

			for (size_t row = 0; row < rows; row++) //N
			{
				std::getline(terrain_file, terrain_line); // 1

				for (size_t collumn = 0; collumn < terrain_line.size(); collumn++) //M
				{
					switch (std::toupper(terrain_line.at(collumn))) // K
					{
					case 'G':
						tiles.at(row).at(collumn).tile_type = TileType::GRASS;
						break;
					case 'F':
						tiles.at(row).at(collumn).tile_type = TileType::FORREST;
						break;
					case 'R':
						tiles.at(row).at(collumn).tile_type = TileType::ROCKS;
						break;
					case 'M':
						tiles.at(row).at(collumn).tile_type = TileType::MOUNTAINS;
						break;
					case 'W':
						tiles.at(row).at(collumn).tile_type = TileType::WATER;
						break;
					default:
						tiles.at(row).at(collumn).tile_type = TileType::GRASS;
						break;
					}
				}
			}
		}
		else
		{
			std::cout << "Could not open terrain file! Is the path correct? Defaulting to grass.." << std::endl;
			std::cout << "Path was: " << terrain_file_path << std::endl;
		}

		//Instantiate tiles for path planning
		for (size_t y = 0; y < tiles.size(); y++) //N
		{
			for (size_t x = 0; x < tiles.at(y).size(); x++) //N
			{
				tiles.at(y).at(x).position_x = x;
				tiles.at(y).at(x).position_y = y;

				if (is_accessible(y, x + 1)) { tiles.at(y).at(x).exits.push_back(&tiles.at(y).at(x + 1)); }//?  
				if (is_accessible(y, x - 1)) { tiles.at(y).at(x).exits.push_back(&tiles.at(y).at(x - 1)); }//?
				if (is_accessible(y + 1, x)) { tiles.at(y).at(x).exits.push_back(&tiles.at(y + 1).at(x)); }//?
				if (is_accessible(y - 1, x)) { tiles.at(y).at(x).exits.push_back(&tiles.at(y - 1).at(x)); }//?
			}
		}
	}

	void Terrain::update()
	{
		//Pretend there is animation code here.. next year :)
	}

	void Terrain::draw(Surface* target) const
	{

		for (size_t y = 0; y < tiles.size(); y++)//N
		{
			for (size_t x = 0; x < tiles.at(y).size(); x++) //M
			{
				int posX = (x * sprite_size) + HEALTHBAR_OFFSET;
				int posY = y * sprite_size;

				switch (tiles.at(y).at(x).tile_type) //K
				{
				case TileType::GRASS:
					tile_grass->draw(target, posX, posY);
					break;
				case TileType::FORREST:
					tile_forest->draw(target, posX, posY);
					break;
				case TileType::ROCKS:
					tile_rocks->draw(target, posX, posY);
					break;
				case TileType::MOUNTAINS:
					tile_mountains->draw(target, posX, posY);
					break;
				case TileType::WATER:
					tile_water->draw(target, posX, posY);
					break;
				default:
					tile_grass->draw(target, posX, posY);
					break;
				}
			}
		}
	}

	vector<vec2> Terrain::get_route_astar(const Tank& tank, const vec2& target)
	{
		TerrainTile* start = &tiles.at(tank.position.y / sprite_size).at(tank.position.x / sprite_size);
		TerrainTile* end = &tiles.at(target.y / sprite_size).at(target.x / sprite_size);

		// set up the open and closed lists
		vector<TerrainTile*> open_list;
		vector<TerrainTile*> closed_list;

		// add the start tile to the open list
		open_list.push_back(start);

		// set the start tile's f score (estimated distance to target) to the heuristic distance to the target
		start->f_score = 0;

		bool route_found = false;

		while (!open_list.empty() && !route_found)
		{
			// find the tile in the open list with the lowest f score
			int lowest_f_score = INT_MAX;
			TerrainTile* current_tile = nullptr;

			for (TerrainTile* tile : open_list)
			{
				if (tile->f_score < lowest_f_score)
				{
					current_tile = tile;
					lowest_f_score = tile->f_score;
				}
			}

			// move the current tile from the open list to the closed list
			open_list.erase(std::remove(open_list.begin(), open_list.end(), current_tile), open_list.end());
			current_tile->visited = true; // current tile is never nullpointer except if openlist is empty in which case it doesnt get here.
			closed_list.push_back(current_tile); // this is not necessary for the algorithm, but it's useful for resetting the tiles.

			// check all exits of the current tile
			for (TerrainTile* exit : current_tile->exits)
			{
				if (exit == end)
				{
					exit->parent = current_tile;
					route_found = true;
					break;
				}
				
				if (exit->visited)
					continue; // skip this exit if it's already in the closed list
				
				if (find(open_list.begin(), open_list.end(), exit) == open_list.end())
				{
					exit->update_scores(current_tile, end);
					open_list.push_back(exit); // add this exit to the open list if it's not already in it
				}
				else
				{							   // if it's already in the open list, check if the current path is better
					//calculate new gscore to check if it is better
					int g_score = current_tile->g_score + 1;
					if (exit->g_score < g_score)
						exit->update_scores(current_tile, end);	// if it is, update the scores
				}
			}
		}

		// if a route was found, construct the route from the target to the start
		if (route_found)
		{
			vector<vec2> route;
			TerrainTile* current_tile = end;
			while (current_tile != start)
			{
				route.insert(route.begin(), vec2((float)current_tile->position_x * sprite_size, (float)current_tile->position_y * sprite_size));
				current_tile = current_tile->parent;
			}
			route.insert(route.begin(), vec2((float)current_tile->position_x * sprite_size, (float)current_tile->position_y * sprite_size));

			// reset tiles
			for (TerrainTile* tile : open_list)
			{
				tile->visited = false;
				tile->parent = nullptr;
				tile->g_score = 0;
				tile->f_score = 0;
			}
			for (TerrainTile* tile : closed_list)
			{
				tile->visited = false;
				tile->parent = nullptr;
				tile->g_score = 0;
				tile->f_score = 0;
			}

			return route;
		}
		else
		{
			// reset tiles
			for (TerrainTile* tile : open_list)
			{
				tile->parent = nullptr;
				tile->g_score = 0;
				tile->f_score = 0;
			}
			for (TerrainTile* tile : closed_list)
			{
				tile->parent = nullptr;
				tile->g_score = 0;
				tile->f_score = 0;
			}

			return vector<vec2>();
		}
	}
 
	// Calculate the heuristic distance (estimated distance) from one tile to another using the Manhattan distance
	int Terrain::heuristic_distance(const TerrainTile& from, const TerrainTile& to)
	{
		int a = from.position_x - to.position_x;
		int b = from.position_y - to.position_y;

		return abs(a) + abs(b);
	}
	
	//Use Breadth-first search to find shortest route to the destination
	vector<vec2> Terrain::get_route(const Tank& tank, const vec2& target)
	{
		//start tile
		const size_t pos_x = tank.position.x / sprite_size;
		const size_t pos_y = tank.position.y / sprite_size;

		//target tile
		const size_t target_x = target.x / sprite_size;
		const size_t target_y = target.y / sprite_size;

		vector<vector<TerrainTile*>> queue;
		queue.push_back(vector<TerrainTile*>{});
		queue.back().push_back(&tiles.at(pos_y).at(pos_x));



		std::vector<TerrainTile*> visited;

		bool route_found = false;
		vector<TerrainTile*> current_route;

		while (!queue.empty() && !route_found)
		{
			current_route = queue.front();
			queue.erase(queue.begin());
			TerrainTile* current_tile = current_route.back();

			//Check all exits, if target then done, else if unvisited push a new partial route

			for (TerrainTile* exit : current_tile->exits)
			{
				exit->cost = current_tile->cost + 1;

				if (exit->position_x == target_x && exit->position_y == target_y)
				{
					visited.push_back(exit);
					route_found = true;
					break;
				}
				else if (!exit->visited)
				{
					exit->visited = true;
					visited.push_back(exit);
					queue.insert(queue.end(), current_route);
					queue.back().push_back(exit);
				}
			}
		}

		//Reset tiles
		for(TerrainTile* tile : visited)
		{
			tile->visited = false;
			tile->cost = 0;
			tile->distanceToEnd = 0;
		}

		if (route_found)
		{
			//Convert route to vec2 to prevent dangling pointers
			std::vector<vec2> route;
			for (TerrainTile* tile : current_route)
			{
				route.push_back(vec2((float)tile->position_x * sprite_size, (float)tile->position_y * sprite_size));
			}

			return route;
		}
		else
		{
			return  std::vector<vec2>();
		}

	}

	//TODO: Function not used, convert BFS to dijkstra and take speed into account next year :)
	float Terrain::get_speed_modifier(const vec2& position) const
	{
		const size_t pos_x = position.x / sprite_size;
		const size_t pos_y = position.y / sprite_size;

		switch (tiles.at(pos_y).at(pos_x).tile_type)
		{
		case TileType::GRASS:
			return 1.0f;
			break;
		case TileType::FORREST:
			return 0.5f;
			break;
		case TileType::ROCKS:
			return 0.75f;
			break;
		case TileType::MOUNTAINS:
			return 0.0f;
			break;
		case TileType::WATER:
			return 0.0f;
			break;
		default:
			return 1.0f;
			break;
		}
	}

	bool Terrain::is_accessible(int y, int x)
	{
		//Bounds check
		if ((x >= 0 && x < terrain_width) && (y >= 0 && y < terrain_height))
		{
			//Inaccessible terrain check
			if (tiles.at(y).at(x).tile_type != TileType::MOUNTAINS && tiles.at(y).at(x).tile_type != TileType::WATER)
			{
				return true;
			}
		}

		return false;
	}
}