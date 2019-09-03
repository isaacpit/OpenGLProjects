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

varying vec4 fragPos_camera;

varying vec4 lightPos_camera;

void main()
{
	vec4 n = normalize(normal); // normal in camera space 

	// vec4 l1 = normalize(vec4(lightPos1, 1.0) - fragPos_camera);
	vec4 l1 = normalize(lightPos_camera - fragPos_camera);
	vec4 l2 = normalize(vec4(lightPos2, 1.0) - fragPos_camera);

	vec4 camera_c = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 e = normalize(camera_c - fragPos_camera);

	vec4 h1 = normalize((l1 + e));
	vec4 h2 = normalize((l2 + e) /2);

	float dot_h1_n = max(0, dot(h1, n));
	float dot_h2_n = max(0, dot(h2, n));

	float dot_l1_n = max(0, dot(l1, n));
	float dot_l2_n = max(0, dot(l2, n));

	vec3 cd1 = kd * dot_l1_n;
	vec3 cd2 = kd * dot_l2_n;

	vec3 cs1 = ks * pow(dot_h1_n, s);
	vec3 cs2 = ks * pow(dot_h2_n, s);

	vec3 c_1 = intensity1 * (ka + cd1 + cs1);
	vec3 c_2 = intensity2 * (ka + cd2 + cs2);

	// vec3 c = c_1 + c_2; 
	vec3 c = c_1;

	gl_FragColor = vec4(c, 1.0);
}
