#pragma once
#include "precomp.h"

namespace Tmpl8
{
	class GridCell;

	class Grid
	{
	public:
		Grid(int height, int width);
		int Size();
		void InsertObject(Tank* tank) noexcept;
		bool RemoveObject(Tank* tank);
		void MoveObject(Tank* tank, int newHash);
		GridCell* GetGridLocation(int hash);
		int GetHash(Tank* tank);
		Tank* GetClosestEnemy(Tank* checkTank);
		int hash(float x, float y) { return static_cast<int>(floorf((x * conversionFactorX)) + static_cast<int>(floorf((y * conversionFactorY)) * setWidth));}
		int hashObject(float x, float y) { return static_cast<int>(floorf(x * (1 / cellSize.x))) + static_cast<int>(floorf(y * (1 / cellSize.y))) * setWidth; }
		int numberCells;
		int setWidth;
		vec2 cellSize;
	private:
		std::unordered_map<int, GridCell*> gridMap;
		int length = SCRWIDTH - HEALTHBAR_OFFSET * 2;
		double conversionFactorX;
		double conversionFactorY;

	};
	class GridCell
	{
	public:
		GridCell(vec2 startPosition, Grid* parent)
		{
			this->startPosition = startPosition;
			this->hash = parent->hashObject(startPosition.x, startPosition.y);
			this->tankList = new vector<Tank*>();
			this->red = new vector<Tank*>();
			this->blue = new vector<Tank*>();
		}

		vector<Tank*>* red;
		vector<Tank*>* blue;
		int hash;
		vec2 startPosition;
		vector<Tank*>* tankList;
	private:
	};
}