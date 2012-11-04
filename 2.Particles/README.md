Million Particle System
===================

This work takes parts from a SIGGRAPH 2004 paper entitled "Hardware-based simulation and collision detection for large particle systems", the citation can be found here: 

["Hardware-based simulation and collision detection for large particle systems"](http://dl.acm.org/citation.cfm?id=1058147.)

This was the first "million particle system" to my knowledge. It is a shame that MacBook Pro's are not equipped with better graphics cards, or I would have been able to make a real GPU particle system without all these dirty hacks. As it is, SIGGRAPH tech from 2006 and before is the latest the hardware lets me implement. 

I did not implement their GPU sorting algorithm (I opted for additive particles, way easier), but modified their approach by using a PBO->VBO mapping for better performance, as opposed to glBegin and glEnd calls.