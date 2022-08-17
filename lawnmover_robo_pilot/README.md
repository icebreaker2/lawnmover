# Robo Pilot
Multiple strategies of a "smart" robo pilot making decisions about the next movements of the lawnmover. 

## Installation
Given the arduino IDE and compilers (same for esp32), we cannot include subdirectories sources or even modules on the same directory layer of the module to compile.
Unfortunately, the Arduino IDE does not follow symlinks of any kind.
Thus, please copy this directory to your sketchbook library location. You can find it by:
1. Opening your Arduino IDE
2. Go to File > Preferences > Sketchbook location and copy the location
3. Copy the module to <your-sketchbook-location>/libraries/

It is now available for any project you open.

## Hints:
* https://en.wikibooks.org/wiki/Robotics/Navigation/Collision_Avoidance
* We do not measure speed, so, braking algorithms are difficult?!
* https://www.researchgate.net/figure/Intelligent-robotics-navigation-system-algorithms_fig1_320083259
* https://en.wikipedia.org/wiki/Motion_planning
* https://www.ri.cmu.edu/pub_files/pub3/urmson_christopher_2003_1/urmson_christopher_2003_1.pdf

## Rules based
// TODO outline rules
See motions in https://www.ri.cmu.edu/pub_files/pub3/urmson_christopher_2003_1/urmson_christopher_2003_1.pdf

## Tree based
// TODO outline states and connections
See motions in https://www.ri.cmu.edu/pub_files/pub3/urmson_christopher_2003_1/urmson_christopher_2003_1.pdf

## Self learning
// TODO outline strategies in reinforcement learning. Use Cost functions transform the terrain evaluation data into a
form that can be used for planning and define a cost function for the past, too.

## TODOs
* Create an internal map of known locations in an exploring mode