#version 120

uniform mat4 P;
uniform int height;
uniform int width;
uniform vec2 movingCenter;
attribute vec3 vertPos;
varying vec4 vColor;

void main()
{
	gl_Position = P * vec4(vertPos, 1.0);
	if (vertPos[1] > 0 ) {
		vColor = vec4(0, 0, 1, 1);
	}
	else if (vertPos[0] < -1.0) {
		vColor = vec4(1, 0, 0, 1);
	}
	else if (vertPos[0] > 1.0) {
		vColor = vec4(1, 0, 0, 1);
	}
	else if (vertPos[0] < .9) {
		vColor = vec4(0, 1, 0, 1);
	}
	else if (vertPos[0] > -.9) {
		vColor = vec4(1, 1, 0, 1);
	}

}
