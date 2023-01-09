#include "precomp.h"

namespace Tmpl8
{
	Grid::Grid(int height, int width)
	{
		numberCells = height * width;
		conversionFactorX = static_cast<float>(width) / static_cast<float>(length);
		conversionFactorY = static_cast<float>(height) / static_cast<float>(SCRHEIGHT);
		bool firstCell = false;
		setWidth = width;// = (max-min)/cell size 
		cellSize = vec2(roundf(1 / conversionFactorX), round(1 / conversionFactorY));
		//convert real coordinates to grid coordinates
		//hash coordinates

		//max = 1280 720
		//min = 0    0


		for (int h = 0; h < width; h++)
		{
			for (int v = 0; v < height; v++)
			{
				GridCell* cell = new GridCell(vec2((v / conversionFactorX), (h / conversionFactorY)), this);
				gridMap.emplace(cell->hash, cell);
			}
		}
	}
	
	int Grid::Size()
	{
		return numberCells;
	}
	
	void Grid::InsertObject(Tank* tank) noexcept
	{
		int h = hashObject(tank->position.x, tank->position.y);
		vector<Tank*>* v = gridMap[h]->tankList;
		v->push_back(tank);
		if (tank->allignment == 0)
		{
			gridMap[h]->blue->push_back(tank);
		}
		else if (tank->allignment == 1)
		{
			gridMap[h]->red->push_back(tank);
		}

		tank->hash = h;
	}
	

	bool Grid::RemoveObject(Tank* tank)
	{
		try
		{
			int h = tank->hash;
			vector<Tank*>* v = gridMap[h]->tankList;
			v->erase(std::remove(v->begin(), v->end(), tank));
			if (tank->allignment == 0)
			{
				vector<Tank*>* b = gridMap[h]->blue;
				b->erase(std::remove(b->begin(), b->end(), tank));
			}
			else if (tank->allignment == 1)
			{
				vector<Tank*>* r = gridMap[h]->red;
				r->erase(std::remove(r->begin(), r->end(), tank));
			}
			return 1;
		}
		catch (exception e)
		{
			printf("%s\n", e.what());
			return 0;
		}
	}

	
	void Grid::MoveObject(Tank* tank, int newHash)
	{
		int h = tank->hash;
		vector<Tank*>* v = gridMap[h]->tankList;
		vector<Tank*>* k = gridMap[newHash]->tankList;
		k->push_back(tank);
		v->erase(std::remove(v->begin(), v->end(), tank));
		tank->hash = h;

		if (tank->allignment == 0)
		{
			vector<Tank*>* b = gridMap[h]->blue;
			b->erase(std::remove(b->begin(), b->end(), tank));
			gridMap[newHash]->blue->push_back(tank);
		}
		else if (tank->allignment == 1)
		{
			vector<Tank*>* r = gridMap[h]->red;
			r->erase(std::remove(r->begin(), r->end(), tank));
			gridMap[newHash]->red->push_back(tank);
		}
		tank->hash = newHash;
	}
	Tmpl8::GridCell* Grid::GetGridLocation(int hash) { return (gridMap[hash]); }

	int Grid::GetHash(Tank* tank) { return hashObject(tank->position.x, tank->position.y); }
	
}// namespace Tmpl8