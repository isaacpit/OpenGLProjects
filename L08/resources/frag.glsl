#version 120

uniform mat4 MV;

// uniform vec3 lightPos;
uniform vec3 lightPos1;
uniform vec3 lightPos2; 
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
	// vec3 color2 = color * vec3(n.x + 1.0, n.y + 1.0, n.z + 1.0);
	// gl_FragColor = vec4(color2.r, color2.g, color2.b, 1.0);


	vec4 l = normalize(lightPos_camera - fragPos_camera);

	vec4 camera_c = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 e = normalize(camera_c - fragPos_camera);

	vec4 h = normalize((l + e) /2);

	float dot_h_n = max(0, dot(h, n));

	float dot_l_n = max(0, dot(l, n));

	vec3 cd = vec3(kd.r * dot_l_n, kd.g * dot_l_n, kd.b * dot_l_n);

	vec3 cs = vec3(
		ks.r * pow(dot_h_n, s),
		ks.g * pow(dot_h_n, s),
		ks.b * pow(dot_h_n, s) );

	vec3 c = vec3(ka.r + cd.r + cs.r, ka.g + cd.g + cs.g, ka.b + cd.b + cs.b);


	gl_FragColor = vec4(c, 1.0);
}
