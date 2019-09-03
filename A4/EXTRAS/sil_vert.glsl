#version 120

uniform mat4 P;
uniform mat4 MV;

attribute vec4 aPos; // in object space
attribute vec3 aNor; // in object space

varying vec4 normal;

varying vec4 fragPos_camera;



void main()
{
	gl_Position = P * MV * aPos;

	normal = vec4(aNor.x, aNor.y, aNor.z, 0.0);
	normal = MV * normal;
	
	// already in camera space
	fragPos_camera = vec4(aPos.x, aPos.y, aPos.z, 1.0);
	fragPos_camera = MV * fragPos_camera;

}
