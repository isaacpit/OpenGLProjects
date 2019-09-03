#version 120

uniform mat4 MV;

varying vec4 normal;
varying vec4 fragPos_camera;

void main()
{
	vec4 n = normalize(normal); // normal in camera space 
	vec3 color2 = 0.5 * vec3(n.x + 1.0, n.y + 1.0, n.z + 1.0);
	// gl_FragColor = vec4(color2.r, color2.g, color2.b, 1.0);

  vec4 camera_camera = vec4(0.0, 0.0, 0.0, 0.0);
  vec4 e = camera_camera - fragPos_camera;
  e = normalize(e);

  float dot_e_n = dot(e, n);
  float thresh = 0.25;
  
  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
  if (dot_e_n < thresh) {
    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
  }
  // float dot_n_e = dot(n, )/
}
