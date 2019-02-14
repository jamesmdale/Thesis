#pragma once
#include "Engine\Utility\Grid.hpp"
#include "Engine\Math\MathUtils.hpp"
#include "Engine\Core\ErrorWarningAssert.hpp"
#include "Engine\Core\EngineCommon.hpp"
#include "Engine\Core\DevConsole.hpp"
#include "Game\Agents\Agent.hpp"
#include <set>

typedef std::pair<int, IntVector2> valCoordPair;

//  =========================================================================================
struct ThesisSearchCell
{

public:

	ThesisSearchCell(){};

	ThesisSearchCell(const ThesisSearchCell& copyCell)
	{
		parentCoordinateX = copyCell.parentCoordinateX;
		parentCoordinateY = copyCell.parentCoordinateY;

		totalCost = copyCell.totalCost;
		movementCost = copyCell.movementCost;
		heuristicCost = copyCell.heuristicCost;
	}

public:
	int parentCoordinateX = -1;
	int parentCoordinateY = -1;

	int totalCost = INT_MAX;
	int movementCost = INT_MAX;
	int heuristicCost = INT_MAX;
};

//  =========================================================================================
static int ThesisCalculateHeuristicValue(const IntVector2& cellPosition, const IntVector2& destinationPosition)
{
	return GetManhattanDistance(cellPosition, destinationPosition);
}

//  =========================================================================================
static void ThesisGetPath(PathData& pathData, Grid<ThesisSearchCell>& cellList, const IntVector2& cellListDimensions, const IntVector2& startPosition, const IntVector2& destinationPosition)
{
	//we have a path from the start to the destination. Now we just need to extract it.
	int currentRow = destinationPosition.x;
	int currentColumn = destinationPosition.y;

	int tempX = cellList.GetValueAtCoordinate(currentRow, currentColumn).parentCoordinateX;
	int tempY = cellList.GetValueAtCoordinate(currentRow, currentColumn).parentCoordinateY;

	//only while we are not the same row and not the same column
	while (tempX != startPosition.x || tempY != startPosition.y)
	{
		IntVector2 coordinate = IntVector2((float)currentRow, (float)currentColumn);

		//add position to path
		pathData.m_currentPath[pathData.m_pathCount] = Vector2(0.5f, 0.5f) + Vector2(coordinate);
		++pathData.m_pathCount;

		int tempRow = cellList.GetValueAtCoordinate(currentRow, currentColumn).parentCoordinateX;
		int tempColumn = cellList.GetValueAtCoordinate(currentRow, currentColumn).parentCoordinateY;

		currentRow = tempRow;
		currentColumn = tempColumn;

		tempX = cellList.GetValueAtCoordinate(currentRow, currentColumn).parentCoordinateX;
		tempY = cellList.GetValueAtCoordinate(currentRow, currentColumn).parentCoordinateY;
	}

	//complete
}

//  =========================================================================================
static bool ThesisAStarSearchOnGrid(PathData& pathData, const IntVector2& startCoordinate, const IntVector2& destinationCoordinate, Grid<int>* grid)
{
	ASSERT_OR_DIE(grid->IsCellValid(startCoordinate) && grid->IsCellValid(destinationCoordinate), "Search cells out of range");
	int value = grid->GetValueAtCoordinate(destinationCoordinate);
	//ASSERT_OR_DIE(grid.GetValueAtCoordinate(startPosition) == 0 && grid.GetValueAtCoordinate(destinationPosition) == 0, "Search cell is blocked (inaccessible)");

	if (startCoordinate == destinationCoordinate)
		return true;

	//as we loop over the grid, we need to mark cells as searched
	bool defaultCheckVal = false;
	Grid<bool> checkedList = Grid<bool>();
	checkedList.InitializeGrid(false, grid->m_dimensions.x, grid->m_dimensions.y);

	//each cell will continue data about it's coordinate and relative costs
	ThesisSearchCell defaultSearchCell;
	Grid<ThesisSearchCell> cellList = Grid<ThesisSearchCell>();	
	cellList.InitializeGrid(defaultSearchCell, grid->m_dimensions.x, grid->m_dimensions.y);

	//set starting indices to the startposition
	IntVector2 currentCoordinate = startCoordinate;

	//set starting cell to lowest values
	ThesisSearchCell* startingCell = cellList.GetReferenceAtCoordinate(currentCoordinate);
	startingCell->parentCoordinateX = startCoordinate.x;
	startingCell->parentCoordinateY = startCoordinate.y;
	startingCell->heuristicCost = 0;
	startingCell->movementCost = 0;
	startingCell->totalCost = 0;
	startingCell = nullptr;

	//add starting position to the first entry in the list
	std::set<valCoordPair> openList;
	openList.insert(std::make_pair(0, currentCoordinate));

	bool isDestinationFound = false;

	while (!openList.empty())
	{
		//temporarily store of entry at start
		valCoordPair beginningPair = *openList.begin();

		//wipe first entry
		openList.erase(openList.begin());

		currentCoordinate.x = beginningPair.second.x;
		currentCoordinate.y = beginningPair.second.y;

		checkedList.SetValueAtCoordinate(true, currentCoordinate);

		//get all surrounding cells
		/*
		UP
		DOWN
		RIGHT
		LEFT
		UPPER_RIGHT
		UPPER_LEFT
		DOWN_LEFT
		DOWN_RIGHT
		*/

		int tempTotalCost = INT_MAX;
		int tempMovementCost = INT_MAX;
		int tempHeuristicCost = INT_MAX;

		ThesisSearchCell originCell = cellList.GetValueAtCoordinate(currentCoordinate.x, currentCoordinate.y);

		// GET UP =========================================================================================
		{
			IntVector2 upCoordinates = IntVector2(currentCoordinate.x , currentCoordinate.y + 1);

			//confirm cell is even on grid
			if (grid->IsCellValid(upCoordinates))
			{
				if (upCoordinates == destinationCoordinate)
				{
					ThesisSearchCell* foundCell = cellList.GetReferenceAtCoordinate(upCoordinates);
					foundCell->parentCoordinateX = currentCoordinate.x;
					foundCell->parentCoordinateY = currentCoordinate.y;
					isDestinationFound = true;

					ThesisGetPath(pathData, cellList, cellList.m_dimensions, startCoordinate, destinationCoordinate);

					foundCell = nullptr;
					return isDestinationFound;
				}

				//if we haven't checked the coordinate yet and the coordinate isn't blocked...
				else if (checkedList.GetValueAtCoordinate(upCoordinates) == false && grid->GetValueAtCoordinate(upCoordinates) == 0)
				{
					tempMovementCost = originCell.movementCost + 1;
					tempHeuristicCost = ThesisCalculateHeuristicValue(upCoordinates, destinationCoordinate);
					tempTotalCost = tempMovementCost + tempHeuristicCost;

					//if the total cost is max (we haven't changed it yet) OR if the value we just calculated is less than the current total cost
					if (cellList.GetValueAtCoordinate(upCoordinates).totalCost == INT_MAX || cellList.GetValueAtCoordinate(upCoordinates).totalCost > tempTotalCost)
					{
						openList.insert(std::make_pair(tempTotalCost, upCoordinates));

						ThesisSearchCell* cell = cellList.GetReferenceAtCoordinate(upCoordinates);
						cell->movementCost = tempMovementCost;
						cell->heuristicCost = tempHeuristicCost;
						cell->totalCost = tempTotalCost;
						cell->parentCoordinateX = currentCoordinate.x;
						cell->parentCoordinateY = currentCoordinate.y;
					}
				}
			}
		}

		// GET DOWN =========================================================================================
		{			
			IntVector2 downCoordinates = IntVector2(currentCoordinate.x, currentCoordinate.y - 1);
			if (grid->IsCellValid(downCoordinates))
			{
				if (downCoordinates == destinationCoordinate)
				{
					ThesisSearchCell* foundCell = cellList.GetReferenceAtCoordinate(downCoordinates);
					foundCell->parentCoordinateX = currentCoordinate.x;
					foundCell->parentCoordinateY = currentCoordinate.y;
					isDestinationFound = true;					

					ThesisGetPath(pathData, cellList, cellList.m_dimensions, startCoordinate, destinationCoordinate);

					foundCell = nullptr;
					return isDestinationFound;
				}

				else if (checkedList.GetValueAtCoordinate(downCoordinates) == false && grid->GetValueAtCoordinate(downCoordinates) == 0)
				{		
					tempMovementCost = originCell.movementCost + 1;
					tempHeuristicCost = ThesisCalculateHeuristicValue(downCoordinates, destinationCoordinate);
					tempTotalCost = tempMovementCost + tempHeuristicCost;

					if (cellList.GetValueAtCoordinate(downCoordinates).totalCost == INT_MAX || cellList.GetValueAtCoordinate(downCoordinates).totalCost > tempTotalCost)
					{
						openList.insert(std::make_pair(tempTotalCost, downCoordinates));

						ThesisSearchCell* cell = cellList.GetReferenceAtCoordinate(downCoordinates);
						cell->movementCost = tempMovementCost;
						cell->heuristicCost = tempHeuristicCost;
						cell->totalCost = tempTotalCost;
						cell->parentCoordinateX = currentCoordinate.x;
						cell->parentCoordinateY = currentCoordinate.y;
					}
				}
			}
		}

		// GET RIGHT =========================================================================================
		{
			IntVector2 rightCoordinates = IntVector2(currentCoordinate.x + 1, currentCoordinate.y);
			if (grid->IsCellValid(rightCoordinates))
			{
				if (rightCoordinates == destinationCoordinate)
				{
					ThesisSearchCell* foundCell = cellList.GetReferenceAtCoordinate(rightCoordinates);
					foundCell->parentCoordinateX = currentCoordinate.x;
					foundCell->parentCoordinateY = currentCoordinate.y;
					isDestinationFound = true;					

					ThesisGetPath(pathData, cellList, cellList.m_dimensions, startCoordinate, destinationCoordinate);

					foundCell = nullptr;
					return isDestinationFound;
				}

				else if (checkedList.GetValueAtCoordinate(rightCoordinates) == false && grid->GetValueAtCoordinate(rightCoordinates) == 0)
				{
					tempMovementCost = originCell.movementCost + 1;
					tempHeuristicCost = ThesisCalculateHeuristicValue(rightCoordinates, destinationCoordinate);
					tempTotalCost = tempMovementCost + tempHeuristicCost;

					if (cellList.GetValueAtCoordinate(rightCoordinates).totalCost == INT_MAX || cellList.GetValueAtCoordinate(rightCoordinates).totalCost > tempTotalCost)
					{
						openList.insert(std::make_pair(tempTotalCost, rightCoordinates));

						ThesisSearchCell* cell = cellList.GetReferenceAtCoordinate(rightCoordinates);
						cell->movementCost = tempMovementCost;
						cell->heuristicCost = tempHeuristicCost;
						cell->totalCost = tempTotalCost;
						cell->parentCoordinateX = currentCoordinate.x;
						cell->parentCoordinateY = currentCoordinate.y;
					}
				}
			}
		}		

		// GET LEFT =========================================================================================
		{
			IntVector2 leftCoordinates = IntVector2(currentCoordinate.x - 1, currentCoordinate.y);
			if (grid->IsCellValid(leftCoordinates))
			{
				if (leftCoordinates == destinationCoordinate)
				{
					ThesisSearchCell* foundCell = cellList.GetReferenceAtCoordinate(leftCoordinates);
					foundCell->parentCoordinateX = currentCoordinate.x;
					foundCell->parentCoordinateY = currentCoordinate.y;
					isDestinationFound = true;				

					ThesisGetPath(pathData, cellList, cellList.m_dimensions, startCoordinate, destinationCoordinate);

					foundCell = nullptr;
					return isDestinationFound;
				}

				else if (checkedList.GetValueAtCoordinate(leftCoordinates) == false && grid->GetValueAtCoordinate(leftCoordinates) == 0)
				{
					tempMovementCost = originCell.movementCost + 1;
					tempHeuristicCost = ThesisCalculateHeuristicValue(leftCoordinates, destinationCoordinate);
					tempTotalCost = tempMovementCost + tempHeuristicCost;

					if (cellList.GetValueAtCoordinate(leftCoordinates).totalCost == INT_MAX || cellList.GetValueAtCoordinate(leftCoordinates).totalCost > tempTotalCost)
					{
						openList.insert(std::make_pair(tempTotalCost, leftCoordinates));

						ThesisSearchCell* cell = cellList.GetReferenceAtCoordinate(leftCoordinates);
						cell->movementCost = tempMovementCost;
						cell->heuristicCost = tempHeuristicCost;
						cell->totalCost = tempTotalCost;
						cell->parentCoordinateX = currentCoordinate.x;
						cell->parentCoordinateY = currentCoordinate.y;
					}
				}
			}
		}		

		//// GET UP_RIGHT =========================================================================================
		//{
		//	IntVector2 upRightCoordinates = IntVector2(currentCoordinate.x + 1, currentCoordinate.y + 1);
		//	if (grid->IsCellValid(upRightCoordinates))
		//	{
		//		if (upRightCoordinates == destinationPosition)
		//		{
		//			ThesisSearchCell* foundCell = cellList.GetReferenceAtCoordinate(upRightCoordinates);
		//			foundCell->parentCoordinateX = currentCoordinate.x;
		//			foundCell->parentCoordinateY = currentCoordinate.y;
		//			isDestinationFound = true;					

		//			GetPath(outPositions, cellList, cellList.m_dimensions, startPosition, destinationPosition, currentMap);

		//			foundCell = nullptr;
		//			return isDestinationFound;
		//		}

		//		else if (checkedList.GetValueAtCoordinate(upRightCoordinates) == false && grid->GetValueAtCoordinate(upRightCoordinates) == 0)
		//		{
		//			tempMovementCost = originCell.movementCost + 1;
		//			tempHeuristicCost = CalculateHeuristicValue(upRightCoordinates, destinationPosition);
		//			tempTotalCost = tempMovementCost + tempHeuristicCost;

		//			if (cellList.GetValueAtCoordinate(upRightCoordinates).totalCost == INT_MAX || cellList.GetValueAtCoordinate(upRightCoordinates).totalCost > tempTotalCost)
		//			{
		//				openList.insert(std::make_pair(tempTotalCost, upRightCoordinates));

		//				ThesisSearchCell* cell = cellList.GetReferenceAtCoordinate(upRightCoordinates);
		//				cell->movementCost = tempMovementCost;
		//				cell->heuristicCost = tempHeuristicCost;
		//				cell->totalCost = tempTotalCost;
		//				cell->parentCoordinateX = currentCoordinate.x;
		//				cell->parentCoordinateY = currentCoordinate.y;
		//			}
		//		}
		//	}
		//}		

		//// GET UP_LEFT =========================================================================================
		//{
		//	IntVector2 upLeftCoordinates = IntVector2(currentCoordinate.x - 1, currentCoordinate.y + 1);
		//	if (grid->IsCellValid(upLeftCoordinates))
		//	{
		//		if (upLeftCoordinates == destinationPosition)
		//		{
		//			ThesisSearchCell* foundCell = cellList.GetReferenceAtCoordinate(upLeftCoordinates);
		//			foundCell->parentCoordinateX = currentCoordinate.x;
		//			foundCell->parentCoordinateY = currentCoordinate.y;
		//			isDestinationFound = true;					

		//			GetPath(outPositions, cellList, cellList.m_dimensions, startPosition, destinationPosition, currentMap);

		//			foundCell = nullptr;
		//			return isDestinationFound;
		//		}

		//		else if (checkedList.GetValueAtCoordinate(upLeftCoordinates) == false && grid->GetValueAtCoordinate(upLeftCoordinates) == 0)
		//		{
		//			tempMovementCost = originCell.movementCost + 1;
		//			tempHeuristicCost = CalculateHeuristicValue(upLeftCoordinates, destinationPosition);
		//			tempTotalCost = tempMovementCost + tempHeuristicCost;

		//			if (cellList.GetValueAtCoordinate(upLeftCoordinates).totalCost == INT_MAX || cellList.GetValueAtCoordinate(upLeftCoordinates).totalCost > tempTotalCost)
		//			{
		//				openList.insert(std::make_pair(tempTotalCost, upLeftCoordinates));

		//				ThesisSearchCell* cell = cellList.GetReferenceAtCoordinate(upLeftCoordinates);
		//				cell->movementCost = tempMovementCost;
		//				cell->heuristicCost = tempHeuristicCost;
		//				cell->totalCost = tempTotalCost;
		//				cell->parentCoordinateX = currentCoordinate.x;
		//				cell->parentCoordinateY = currentCoordinate.y;
		//			}
		//		}
		//	}
		//}		

		//// GET DOWN_RIGHT =========================================================================================
		//{			
		//	IntVector2 downRightCoordinates = IntVector2(currentCoordinate.x + 1, currentCoordinate.y - 1);
		//	if (grid->IsCellValid(downRightCoordinates))
		//	{
		//		if (downRightCoordinates == destinationPosition)
		//		{
		//			ThesisSearchCell* foundCell = cellList.GetReferenceAtCoordinate(downRightCoordinates);
		//			foundCell->parentCoordinateX = currentCoordinate.x;
		//			foundCell->parentCoordinateY = currentCoordinate.y;
		//			isDestinationFound = true;					

		//			GetPath(outPositions, cellList, cellList.m_dimensions, startPosition, destinationPosition, currentMap);

		//			foundCell = nullptr;
		//			return isDestinationFound;
		//		}

		//		else if (checkedList.GetValueAtCoordinate(downRightCoordinates) == false && grid->GetValueAtCoordinate(downRightCoordinates) == 0)
		//		{
		//			tempMovementCost = originCell.movementCost + 1;
		//			tempHeuristicCost = CalculateHeuristicValue(downRightCoordinates, destinationPosition);
		//			tempTotalCost = tempMovementCost + tempHeuristicCost;

		//			if (cellList.GetValueAtCoordinate(downRightCoordinates).totalCost == INT_MAX || cellList.GetValueAtCoordinate(downRightCoordinates).totalCost > tempTotalCost)
		//			{
		//				openList.insert(std::make_pair(tempTotalCost, downRightCoordinates));

		//				ThesisSearchCell* cell = cellList.GetReferenceAtCoordinate(downRightCoordinates);
		//				cell->movementCost = tempMovementCost;
		//				cell->heuristicCost = tempHeuristicCost;
		//				cell->totalCost = tempTotalCost;
		//				cell->parentCoordinateX = currentCoordinate.x;
		//				cell->parentCoordinateY = currentCoordinate.y;
		//			}
		//		}
		//	}
		//}

		//// GET DOWN_LEFT =========================================================================================
		//{
		//	IntVector2 downLeftCoordinates = IntVector2(currentCoordinate.x - 1, currentCoordinate.y - 1);
		//	if (grid->IsCellValid(downLeftCoordinates))
		//	{
		//		if (downLeftCoordinates == destinationPosition)
		//		{
		//			ThesisSearchCell* foundCell = cellList.GetReferenceAtCoordinate(downLeftCoordinates);
		//			foundCell->parentCoordinateX = currentCoordinate.x;
		//			foundCell->parentCoordinateY = currentCoordinate.y;
		//			isDestinationFound = true;					

		//			GetPath(outPositions, cellList, cellList.m_dimensions, startPosition, destinationPosition, currentMap);

		//			foundCell = nullptr;
		//			return isDestinationFound;
		//		}

		//		else if (checkedList.GetValueAtCoordinate(downLeftCoordinates) == false && grid->GetValueAtCoordinate(downLeftCoordinates) == 0)
		//		{
		//			tempMovementCost = originCell.movementCost + 1;
		//			tempHeuristicCost = CalculateHeuristicValue(downLeftCoordinates, destinationPosition);
		//			tempTotalCost = tempMovementCost + tempHeuristicCost;

		//			if (cellList.GetValueAtCoordinate(downLeftCoordinates).totalCost == INT_MAX || cellList.GetValueAtCoordinate(downLeftCoordinates).totalCost > tempTotalCost)
		//			{
		//				openList.insert(std::make_pair(tempTotalCost, downLeftCoordinates));

		//				ThesisSearchCell* cell = cellList.GetReferenceAtCoordinate(downLeftCoordinates);
		//				cell->movementCost = tempMovementCost;
		//				cell->heuristicCost = tempHeuristicCost;
		//				cell->totalCost = tempTotalCost;
		//				cell->parentCoordinateX = currentCoordinate.x;
		//				cell->parentCoordinateY = currentCoordinate.y;
		//			}
		//		}
		//	}
		//}		
	}

	if (!isDestinationFound)
		DebuggerPrintf("A* SEARCH INCOMPLETE!!");

	return isDestinationFound;
};

