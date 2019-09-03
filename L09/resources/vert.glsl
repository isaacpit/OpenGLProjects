#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat3 T1;

attribute vec4 aPos;
attribute vec2 aTex;

varying vec2 vTex0;
varying vec2 vTex1;

void main()
{
	gl_Position = P * MV * aPos;
	vTex0 = aTex;
	vTex1 = vec2(vec3(aTex, 1.0) * T1);
}
