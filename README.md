THESIS - README

Controls:
- SPACE			 	= Select menu uption
- UP, DOWN, LEFT, RIGHT 	= Pan Camera
- PAGEUP, PAGEDOWN			= Zoom In/Out
- R                         = Reset Camera Position
- T                         = Show Tile Data
- I                         = Show Agent Id
- CTRL + F                  = Show Debug Data
- CTRL + P		    = Pause Sim
- CTRL+ UP or DOWN (While debug data shown) = Cycle through agent details


Optimizations:

1. Memoization
	- We store off utility calculations made into a map for quicker access for already computed utilities
	- Uses a rounded threshold

3. Copy Pathing
	- Agents will look for other agents heading in a similar direction (objective) that they are. If so, they copy the path of the agent and skip calculating their own path.

	- We maintain a list of agents sorted by their X coordinate and a list sorted by their Y.  When we conduct our path copy, we just find the nearest on X and on Y and find if any match our destination to copy.
4. Ammortization
	- A minimum 60 fps will be maintained when running a simulation with the 'IsBudgeted' flag set.
	- Debug information regarding how many agents were updated in the last frame will be displayed

4. Agent Budgetting
	- Keep queue of agents to update based on time since last update
	- Agents can skip this list if they are idling and have nothing to do
	- A budget is calculated based on frame time and adjusted as agents update. (We save how long this agent took to update).


************************************************************************************************
