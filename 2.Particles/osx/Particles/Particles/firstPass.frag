#version 120
uniform sampler2D positionTex;
uniform sampler2D velocityTex;
uniform float time_step;
uniform vec3 gravity;

uniform sampler2D colorTex;
uniform float step;
uniform float restitution;
uniform int spectrum;

void main(void) {
        float starttime = texture2D( velocityTex, gl_TexCoord[0].st ).w;
        // retrieve data from textures
        vec3 position = texture2D( positionTex, gl_TexCoord[0].st ).xyz;
        vec3 velocity = texture2D( velocityTex, gl_TexCoord[0].st ).xyz;
        vec4 color = texture2D( colorTex, gl_TexCoord[0].st ).xyzw;
        
        // update particle if visible
        if(starttime < step*time_step) {
            velocity.xyz = velocity.xyz + gravity.xyz*time_step;
            position.xyz = position.xyz + velocity.xyz*time_step;
            position = clamp (position, -2.0, 2.0); // this make collisions with rectangle walls easy
            if(spectrum == 0) {
                color = vec4(color.x-time_step, color.yz, 1.0-time_step);
            }
            if(spectrum == 1) {
                color = vec4(color.x, color.y-time_step, color.z, 1.0-time_step);
            }
            if(spectrum == 2) {
                color = vec4(color.x, color.y, color.z-time_step, 1.0-time_step);
            }
            
            
        } else {
            color = vec4(color.xyz, 0.0);
        }
        
    
        // See if particle is on a wall, if so reflect the velocity on the wall
        // there is definitely a better way to do this but i have no time
        bool collision = false;
        if(position.x == -2) {
            velocity.xyz = reflect(velocity.xyz, vec3(1.0, 0.0, 0.0));
            collision = true;
        }
        if(position.x == 2.0) {
            velocity.xyz = reflect(velocity.xyz, vec3(-1.0, 0.0, 0.0));
            collision = true;
        }
        if(position.y == -2) {
            velocity.xyz = reflect(velocity.xyz, vec3(0.0, 1.0, 0.0));
            collision = true;
        }
        if(position.y == 2.0) {
            velocity.xyz = reflect(velocity.xyz, vec3(0.0, -1.0, 0.0));
            collision = true;
        }
        if(position.z == -2) {
            velocity.xyz = reflect(velocity.xyz, vec3(0.0, 0.0, 1.0));
            collision = true;
        }
        if(position.z == 2.0) {
            velocity.xyz = reflect(velocity.xyz, vec3(0.0, 0.0, -1.0));
            collision = true;
        }
        
        // Apply restitution and more force = more color
        if(collision) {
            velocity.y *= restitution;
            if (spectrum==0) {
                color = vec4(color.x+1.0, color.yz, 1.0);
            }
            if(spectrum==1) {
                color = vec4(color.x, color.y+1.0, color.z, 1.0);
            }
            if(spectrum==2) {
                color = vec4(color.x, color.y, color.z+1.0, 1.0);
            }
        }
    
        // update position
        gl_FragData[0] = vec4( position.xyz, 1.0);
        // update velocity and lifetime
        gl_FragData[1] = vec4( velocity.xyz, starttime);
        // update color
        gl_FragData[2] = color;
}