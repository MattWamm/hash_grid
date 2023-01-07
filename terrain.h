#pragma once

namespace Tmpl8
{
    
    enum TileType
    {
        GRASS,
        FORREST,
        ROCKS,
        MOUNTAINS,
        WATER
    };

    class TerrainTile
    {
    public:
        //TerrainTile *up, *down, *left, *right;
        vector<TerrainTile*> exits;
        bool visited = false;
        int cost = 0;
        float distanceToEnd{};
        float sum;
        size_t position_x;
        size_t position_y;
        TileType tile_type;
        
        int g_score;
        int f_score;
        TerrainTile* parent;

		void update_scores(TerrainTile* current, TerrainTile* end)
		{

            parent = current;
            g_score = current->g_score + 1;
            f_score = g_score + heuristic_distance(*this, *end);;
		}
    private:

        int heuristic_distance(const TerrainTile& from, const TerrainTile& to)
        {
            int a = from.position_x - to.position_x;
            int b = from.position_y - to.position_y;

            return abs(a) + abs(b);
        }
    };


    class Terrain
    {
    public:

        Terrain();

        void update();
        void draw(Surface* target) const;

        int heuristic_distance(const TerrainTile& from, const TerrainTile& to);

        vector<vec2> get_route_astar(const Tank& tank, const vec2& target);
        //Use Breadth-first search to find shortest route to the destination
        vector<vec2> get_route(const Tank& tank, const vec2& target);

        float get_speed_modifier(const vec2& position) const;


    private:

        bool is_accessible(int y, int x);

        static constexpr int sprite_size = 16;
        static constexpr size_t terrain_width = 80;
        static constexpr size_t terrain_height = 45;

        std::unique_ptr<Surface> grass_img;
        std::unique_ptr<Surface> forest_img;
        std::unique_ptr<Surface> rocks_img;
        std::unique_ptr<Surface> mountains_img;
        std::unique_ptr<Surface> water_img;

        std::unique_ptr<Sprite> tile_grass;
        std::unique_ptr<Sprite> tile_forest;
        std::unique_ptr<Sprite> tile_rocks;
        std::unique_ptr<Sprite> tile_mountains;
        std::unique_ptr<Sprite> tile_water;

        std::array<std::array<TerrainTile, terrain_width>, terrain_height> tiles;
    };
}