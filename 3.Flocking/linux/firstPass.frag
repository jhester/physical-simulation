#version 120
uniform sampler2D positionTex;
uniform sampler2D velocityTex;
uniform float time_step;

uniform sampler2D colorTex;
uniform int texSixe;

uniform float Cohesion;
uniform float Alignment;
uniform float NeighborRadius;
uniform float CollisionRadius;

void main(void) {
    float starttime = texture2D( velocityTex, gl_TexCoord[0].st ).w;
    // retrieve data from textures
    vec3 position = texture2D( positionTex, gl_TexCoord[0].st ).xyz;
    vec3 velocity = texture2D( velocityTex, gl_TexCoord[0].st ).xyz;
    vec4 color = texture2D( colorTex, gl_TexCoord[0].st ).xyzw;
    
    // update particle if visible
    // Rule 1 : collision avoidance
    vec3 c1 = vec3(0.0, 0.0, 0.0);
    vec3 c2 = vec3(0.0, 0.0, 0.0);
    vec3 c3 = vec3(0.0, 0.0, 0.0);
    int N = 0;//texSixe * texSixe;
    for (int i=0; i<texSixe; i++) {
        for (int j=0; j<texSixe; j++) {
            vec2 coord = vec2(i, j);
            
            if(coord != gl_TexCoord[0].st) {
                vec3 position_n = texture2D( positionTex, coord ).xyz;
                vec3 velocity_n = texture2D( velocityTex, coord ).xyz;
                
                // Rule 1 : Collision avoidance
                if(length(position_n - position) < CollisionRadius) { // Possible collision, avoid
                    c1 = c1 - (position_n - position);
                }
                if(length(position_n - position) < NeighborRadius) {
                // Rule 2 : Cohesion, add up all position
                c2 = c2 + position_n;
                // Rule 3 : Alignment, velocity
                c3 = c3 + velocity_n;
                N++;
                }
           }
        }
    }
    
    c2 = c2 / (N- 1);
    c2 = (c2 - position) / Cohesion;
    
    c3 = c3 / (N- 1);
    c3 = (c3 - velocity) / Alignment;
    
    // Constrain to sphere
    vec3 c4 = vec3(0.0, 0.0, 0.0);
    if(length(position) > 1.5)
        c4 = -1*position / 5.0;
    
    velocity.xyz = velocity.xyz + c1 + c2 + c3 + c4;
    // Limit velocity
    if(length(velocity) > 3.0) {
        velocity = (velocity / length(velocity)) * 3.0;
    }
    
    position.xyz = position.xyz + velocity.xyz*time_step;
    color = vec4(color.x, color.yz, 1.0);
    
    // update position
    gl_FragData[0] = vec4( position.xyz, 1.0);
    // update velocity and lifetime
    gl_FragData[1] = vec4( velocity.xyz, starttime);
    // update color
    gl_FragData[2] = color;
}