#version 120
uniform float Time; // updated each frame by the application
uniform float Lifetime;
uniform vec3 Acceleration;

attribute vec3 Velocity; // initial velocity
attribute float StartTime; // time at which particle is activated

varying vec4 Color;

void main() {    
	vec4 vert;
    	float t = Time - StartTime;
    	if (t >= 0.0) {
		vec3 newV = Velocity + t * Acceleration;
        	vert = gl_Vertex + vec4(newV * t, 0.0);

        	Color = gl_Color;
            Color.a = t / Lifetime;
    	} else {
        	Color = vec4(0.0, 0.0, 0.0, 0.0);   // "pre-birth" color
        	vert = gl_Vertex;
		
        }
	gl_Position = gl_ModelViewProjectionMatrix * vert; 
}