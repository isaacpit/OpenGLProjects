#version 120

uniform mat4 P;
uniform mat4 MV;

uniform vec3 lightPos; 
uniform vec3 lightPos1; 
uniform vec3 lightPos2; 
uniform vec3 intensity1; 
uniform vec3 intensity2; 

attribute vec4 aPos; // in object space
attribute vec3 aNor; // in object space

varying vec3 color; // Pass to fragment shader

varying vec4 normal;
varying vec4 lightPos_camera;
varying vec4 lightPos_camera1;
varying vec4 lightPos_camera2;

varying vec4 fragPos_camera;



void main()
{
	gl_Position = P * MV * aPos;

	normal = vec4(aNor.x, aNor.y, aNor.z, 0.0);
	normal = MV * normal;
	
	// already in camera space
	lightPos_camera = vec4(lightPos.x, lightPos.y, lightPos.z, 1.0);
	lightPos_camera1 = vec4(lightPos1.x, lightPos1.y, lightPos1.z, 1.0);
	lightPos_camera2 = vec4(lightPos2.x, lightPos2.y, lightPos2.z, 1.0);
	
	fragPos_camera = vec4(aPos.x, aPos.y, aPos.z, 1.0);
	fragPos_camera = MV * fragPos_camera;

	color = vec3(0.5, 0.5, 0.5);
}
