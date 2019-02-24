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
	// vec3 color2 = color * vec3(n.x + 1.0, n.y + 1.0, n.z + 1.0);
	// gl_FragColor = vec4(color2.r, color2.g, color2.b, 1.0);


	vec4 l = normalize(lightPos_camera - fragPos_camera);

	float dot_l_n = dot(l, n);

	vec3 cd = vec3(kd.r * dot_l_n, kd.g * dot_l_n, kd.b * dot_l_n);
	vec3 c = vec3(ka.r + cd.r, ka.g + cd.g, ka.b + cd.b);

	gl_FragColor = vec4(c, 1.0);
}
