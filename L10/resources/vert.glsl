#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat3 T;

attribute vec4 aPos;
attribute vec3 aNor;
attribute vec2 aTex;

varying vec2 vTex0;
varying vec2 vTex1;
varying vec4 fragPos_cam;
varying vec3 normal_cam;

void main()
{
	gl_Position = P * MV * aPos;

	fragPos_cam = MV * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	normal_cam = normalize(vec3(MV * vec4(aNor, 0.0)));


	vTex0 = aTex;
	vTex1 = vec2(vec3(aTex, 1.0) * T);
}
