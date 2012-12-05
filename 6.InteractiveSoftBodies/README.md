Interactive Soft Bodies
===================
###With Three.js

This project simulates a minecraft style voxel world, dynamically generated, where you can drop in soft bodie voxels and watch them interact with the environment and each other.

Also bodies are affected by the wind forces (shown by a particle sim in the background).

The code includes a Three.Js enabled custom soft body mesh object with constrained time step Rung-Kutta 4 integration for good physical accuracy and stability. 

Try changing the "wind" parameters to see the particles change (and bodies change also), increase the speed to 23 to see the bodies fall faster.

Note that changing the Kd and Ks values with a high speed value will introduce some instablity.