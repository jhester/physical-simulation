Boids
===================
![](https://raw.github.com/jhester/physical-simulation/master/3.Flocking/screen.png)

Uses the Boids model shown in the [Reynolds 87 paper](http://www.cs.toronto.edu/~dt/siggraph97-course/cwr87/), uses Collision Avoidance (Separation), 
Cohesion, and Alignment on a local scale. Fully implemented on the GPU, can easily run about 
5,000 boids at once on a Mac Book Pro. Right now its very easy to make the system unstable and
do weird things. All the boids are in a 


####COMMANDS

'r' - reset simulation, reload parameters file

'g' - show / hide grid

'q' - quit


####PARAMETERS

__Cohesion__ : the number of time steps it takes for the boids to fully cohese locally

__Alignment__ : the number of time steps it takes for the boids to fully align locally

__NeighborRadius__ : the size of the radius around each boid wherein it considers 
				 surrounding boids "neighbors" in its calculations

__CollisionRadius__ : the size of the radius where a boid considers its getting to close to
				  a neighboring boid and adjusts its steering