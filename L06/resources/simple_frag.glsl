#version 120

// uniform int height;
varying vec4 vColor;
uniform int height;
uniform int width;
uniform vec2 movingCenter;

void main()
{
	vec2 center = vec2(width / 2, height / 2);
	// vec2 center = vec2(width / 2, height / 2);
	vec2 p = vec2(gl_FragCoord.x, gl_FragCoord.y);
	float dist = distance(p, movingCenter);
	float delta = 20;
	if (dist < delta) {
		discard;
	}
	else {
		float radius = 300.0;
		vec4 ratio = dist * vec4(.8, .8, .8, 1.00) / radius;
		// vec4 ratio = dist * vec4(1.0, 1.0, 1.0, 1.0) / 300.0;
		gl_FragColor = vColor + ratio ;
	}

}
