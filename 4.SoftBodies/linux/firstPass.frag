#version 120
uniform sampler2D positionTex;
uniform sampler2D velocityTex;
uniform sampler2D colorTex;
uniform sampler2D forcesTex;
uniform sampler2D neighbourhoodTex; // Hack this : also holds sum of orces on center of mass
uniform sampler2D neighboursTex;

uniform float time_step;
uniform vec3 gravity;
uniform vec3 centerOfMass;

uniform int KS;
uniform int KD;

uniform float step;
uniform float restitution;

uniform int numVerts; // Number of vertices / particles
uniform int texSixe; // Size of the texture (w, h)
uniform int neighborTexSize;

void main(void) {
    if((gl_TexCoord[0].s + gl_TexCoord[0].t*texSixe) < numVerts-1) {
        float starttime = texture2D( velocityTex, gl_TexCoord[0].st ).w;
        // retrieve data from textures
        vec3 position = texture2D( positionTex, gl_TexCoord[0].st ).xyz;
        vec3 velocity = texture2D( velocityTex, gl_TexCoord[0].st ).xyz;
        vec4 color = texture2D( colorTex, gl_TexCoord[0].st ).xyzw;
        vec4 neighbourhood = texture2D( neighbourhoodTex, gl_TexCoord[0].st ).xyzw;
        int numNeighbours = int(neighbourhood.b);
        
        float ks = KS;
        float kd = KD;
        
        vec3 force = gravity;
        for(int i=0;i < numNeighbours;i++) {
            int ndx = int(i+neighbourhood.r + neighbourhood.g*neighborTexSize);
            vec4 neighbouriInfo = texture2D(neighboursTex, vec2(mod(ndx, neighborTexSize), ndx / neighborTexSize));
            float restLength = neighbouriInfo.b;
            vec3 neighbourPosition = texture2D( positionTex, neighbouriInfo.rg ).xyz;
            vec3 neighbourVelocity = texture2D( velocityTex, neighbouriInfo.rg ).xyz;
            
            vec3 xij = neighbourPosition.xyz-position.xyz;
            
            if(neighbourPosition.xyz == centerOfMass.xyz) {
                ks = 1.0;
                kd = 0.1;
            } 
            force += ks * (length(xij) - restLength) * (xij / length(xij)); // Spring force
            force += -1 * kd * (velocity.xyz - neighbourVelocity.xyz); // Damping force
        }
        
        // update particle if not center of mass
        if(position.xyz != centerOfMass.xyz) {
            velocity.xyz = velocity.xyz + force*time_step;
            position.xyz = position.xyz + velocity.xyz*time_step;
            position.y = clamp (position.y, 0.0, 180.0); // this make collisions with floor easy
        }
        
        // Bounce particle if on floor
        if(position.y == 0.0) {
            velocity.xyz = reflect(velocity.xyz, vec3(0.0, 1.0, 0.0));
            velocity.y *= restitution;
        }
        
        // update position
        gl_FragData[0] = vec4( position.xyz, 1.0);
        // update velocity and lifetime
        gl_FragData[1] = vec4( velocity.xyz, starttime);
        // update color
        gl_FragData[2] = vec4(color.x-0.001, color.yzw);
    }
}