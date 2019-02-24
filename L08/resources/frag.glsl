#version 120

uniform mat4 MV;

uniform vec3 lightPos;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

varying vec3 color; // passed from the vertex shader
varying vec4 normal;

varying vec4 lightPos_camera;
varying vec4 fragPos_camera;

void main()
{
	vec4 n = normalize(normal); // normal in camera space 
	// vec3 n_3 = normalize(vec3(normal.x, normal.y, normal.z));
	vec4 colorNew = 0.5 * (n + 1.0); // maps normal to a valid color

	vec4 coord = vec4(gl_FragCoord[0], gl_FragCoord[1], gl_FragCoord[2], 1.0);
	vec4 l_2 = vec4(lightPos.x - gl_FragCoord[0], 
	lightPos.y -gl_FragCoord[1],
	lightPos.z - gl_FragCoord[2],
	0.0);
	l_2 = MV * l_2;

	l_2 = normalize(l_2);

	vec4 coord_camera = MV * coord;
	// gl_FragColor = vec4(colorNew.r, colorNew.g, colorNew.b, 1.0);
	// vec4 l = vec4(lightPos_camera[0] - coord_camera[0],
	// 	lightPos_camera[1] - coord_camera[1],
	// 	lightPos_camera[2] - coord_camera[2],
	// 	lightPos_camera[3] - coord_camera[3]); // light pos vector in camera space

	vec4 l = vec4(lightPos_camera[0] - fragPos_camera[0],
		lightPos_camera[1] - fragPos_camera[1],
		lightPos_camera[2] - fragPos_camera[2],
		lightPos_camera[3] - fragPos_camera[3]);

	// vec4 n_camera = vec4(n[0] - fragPos_camera[0],
	// n[1] - fragPos_camera[1],
	// n[2] - fragPos_camera[2],
	// n[3] - fragPos_camera[3]);

	l = normalize(l);
	// n_camera = normalize(n_camera);
	
	// float x = lightPos_camera[0] - gl_FragCoord[0];

	float dot_l_n = dot(l, n);
	vec3 cd = vec3(
		kd.r * max(0, dot_l_n),
		kd.g * max(0, dot_l_n),
		kd.b * max(0, dot_l_n)
	);

	vec3 c = vec3(cd.r + ka.r, cd.g + ka.g, cd.b + ka.b);

	gl_FragColor = vec4(c.r, c.g, c.b, 1.0);
	// gl_FragColor = vec4(cd.r, cd.g, cd.b, 1.0);
	// gl_FragColor = vec4(colorNew.r, colorNew.g, colorNew.b, 1.0);
}
