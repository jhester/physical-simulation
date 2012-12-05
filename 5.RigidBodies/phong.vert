varying vec3 normal, lightDir, eyeVec;
varying float att;

void main()
{
	normal = gl_NormalMatrix * gl_Normal;
    
	vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);
	lightDir = vec3(gl_LightSource[0].position.xyz - vVertex);
	eyeVec = -vVertex;
	
	float d = length(lightDir);
	
	att = 1.0 / ( gl_LightSource[0].constantAttenuation +
                 (gl_LightSource[0].linearAttenuation*d) +
                 (gl_LightSource[0].quadraticAttenuation*d*d) );
	
    
	gl_Position = ftransform();		
}