THESIS - README

Controls:
- UP, DOWN, LEFT, RIGHT 	= Pan Camera
- PAGEUP, PAGEDOWN			= Zoom In/Out
- R                         = Reset Camera Position
- T                         = Show Tile Data
- I                         = Show Agent Id
- CTRL + F                  = Show Debug Data
- CTRL + P		    = Pause Sim
- CTRL+ UP or DOWN (While debug data shown) = Cycle through agent details


Optimizations:

1. Map Saving After Initialization. 
	- (Map doesn't change so no reconstruction is needed during A* calculations)

2. Memoization
	- We store off utility calculations made into a map for quicker access for already computed utilities
	- Uses a rounded threshold

3. Copy Pathing
	- Agents will look for other agents heading in a similar direction (objective) that they are. If so, they copy the path of the agent
	and skip calculating their own path.

	- We maintain a list of agents sorted by their X coordinate and a list sorted by their Y.  When we conduct our path copy, we just find the nearest
	on X and on Y and find if any match our destination to copy.

4. Agent Budgetting
	- Keep queue of agents to update based on time since last update
	- Agents can skip this list if they are idling and have nothing to do
	- A budget is calculated based on frame time and adjusted as agents update. (We save how long this agent took to update).


************************************************************************************************

	**Potential Other Categories**

	- JPS - *NOTE* - Evaluate jump point search (JPS) - GDC 2016(maybe)  - Steve Rabin
	- Cache Friendliness / Data Oriented / AoS (Array of Structs) vs SoA (Structs of Array)
	- Progressive Path Quality (A valid path vs best possible path)
	- Bidirecitonal A* - Nathan Sturtavant (going from opposite directions)
	- Check calling new in A* calculation (across your system you need to check calling new and remove as much memory allocation as possible)
