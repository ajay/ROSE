Authors:
Ming Tai Ha
(Except he was too lazy to put his own name in here so I put it in here instead)
Timothy Yong
Jonathan Risinger

NOTE: This is a minimalistic A* from a bird's eye view.
Only to do local map resolution

Running the Simulation

1) Type "make"
2) run "./sim random=30 forward_max"
   this will run the A* algorithm on a grid 101x101 with a 30% chance of generating
   a wall

Options for ./sim:
arg1:
random=<prob>, where prob is any value between 0 and 100
file=<filename>, where filename is any image
arg2:
forward_max   Repeated forward A* with max g tie breaking
forward_min   Repeated forward A* with min g tie breaking
backward      Repeated backward A* with max g tie breaking
adaptive      Adaptive forward A* with max g tie breaking

A* files:
astar.cpp       This is the actual algorithm, which goes back and forth to the search tree
draw.cpp        This is just a drawing utility file for drawing the maze onto a screen
heuristic.cpp   This is where the heuristics are stored
mapcheck.cpp    This is a checker that to check whether a map has a path or not (obsolete)
maze_gen.cpp    This is used to generate random mazed
maze_imgio.cpp  This is used to load and store mazes from/to images
robot.cpp       This is the robot that uses the search algorithm to go along a path
searchtree.cpp  This is the search tree, which is looks at next states via a heap
sim.cpp         This is the main program; it runs the simulation and calls the robot
sim_window.cpp  This is the window to display the screen to the user
state.cpp       This is a search state of the algorithm
svec.cpp        This is a vector class that can store a series of states

TODO:
Need to implement D* as dictated by the paper (look at search history)
