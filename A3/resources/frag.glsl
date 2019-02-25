#version 120

uniform mat4 MV;

// uniform vec3 lightPos;
uniform vec3 lightPos1;
uniform vec3 lightPos2; 
uniform float intensity1;
uniform float intensity2;

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

varying vec3 color; // passed from the vertex shader
varying vec4 normal;

varying vec4 lightPos_camera;
varying vec4 lightPos_camera1;
varying vec4 lightPos_camera2;

varying vec4 fragPos_camera;

void main()
{
	vec4 n = normalize(normal); // normal in camera space 
	// vec3 color2 = color * vec3(n.x + 1.0, n.y + 1.0, n.z + 1.0);
	// gl_FragColor = vec4(color2.r, color2.g, color2.b, 1.0);

	vec4 l = normalize(lightPos_camera - fragPos_camera);

	vec4 l1 = normalize(lightPos_camera1 - fragPos_camera);
	vec4 l2 = normalize(lightPos_camera2 - fragPos_camera);

	vec4 camera_c = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 e = normalize(camera_c - fragPos_camera);

	vec4 h = normalize((l + e) /2);
	vec4 h1 = normalize((l1 + e) /2);
	vec4 h2 = normalize((l2 + e) /2);

	float dot_h_n = max(0, dot(h, n));
	float dot_h1_n = max(0, dot(h1, n));
	float dot_h2_n = max(0, dot(h2, n));

	float dot_l_n = max(0, dot(l, n));
	float dot_l1_n = max(0, dot(l1, n));
	float dot_l2_n = max(0, dot(l2, n));

	vec3 cd = vec3(kd.r * dot_l_n, kd.g * dot_l_n, kd.b * dot_l_n);
	vec3 cd1 = vec3(kd.r * dot_l1_n, kd.g * dot_l1_n, kd.b * dot_l1_n);
	vec3 cd2 = vec3(kd.r * dot_l2_n, kd.g * dot_l2_n, kd.b * dot_l2_n);

	vec3 cs = vec3(
		ks.r * pow(dot_h_n, s),
		ks.g * pow(dot_h_n, s),
		ks.b * pow(dot_h_n, s) );
	vec3 cs1 = vec3(
		ks.r * pow(dot_h1_n, s),
		ks.g * pow(dot_h1_n, s),
		ks.b * pow(dot_h1_n, s) );
	vec3 cs2 = vec3(
		ks.r * pow(dot_h2_n, s),
		ks.g * pow(dot_h2_n, s),
		ks.b * pow(dot_h2_n, s) );


	// vec3 c = vec3(ka.r + cd.r + cs.r, ka.g + cd.g + cs.g, ka.b + cd.b + cs.b);
	vec3 c_1 = vec3((ka.r + cd1.r + cs1.r) * intensity1, 
	(ka.g + cd1.g + cs1.g) * intensity1,
	(ka.b + cd1.b + cs1.b) * intensity1);

	vec3 c_2 = vec3((ka.r + cd2.r + cs2.r) * intensity2, 
	(ka.g + cd2.g + cs2.g) * intensity2,
	(ka.b + cd2.b + cs2.b) * intensity2);

	vec3 c = vec3(c_1.r + c_2.r, c_1.g + c_2.g, c_1.b + c_2.b);

	gl_FragColor = vec4(c, 1.0);
}
