#version 120

uniform mat4 MV;
uniform mat4 final_color;

uniform vec4 c1;
uniform vec4 c2;
uniform vec4 c3;
uniform vec4 c4;
 
uniform vec3 thresh;

varying vec4 normal;
varying vec4 fragPos_camera;

void main()
{
	vec4 n = normalize(normal); // normal in camera space 
	vec3 color2 = 0.5 * vec3(n.x + 1.0, n.y + 1.0, n.z + 1.0);
	gl_FragColor = vec4(color2.r, color2.g, color2.b, 1.0);

  vec4 camera_camera = vec4(0.0, 0.0, 0.0, 0.0);
  vec4 e = camera_camera - fragPos_camera;
  e = normalize(e);

  float intensity;
  vec4 color;
  intensity = dot(e, n);

  if (intensity > thresh[0]) {
		// color = vec4(1.0,0.5,0.5,1.0);
    color = c1;
  }
	else if (intensity > thresh[1]) {
		// color = vec4(0.6,0.3,0.3,1.0);
    color = c2;

  }
	else if (intensity > thresh[2]) {
		// color = vec4(0.4,0.2,0.2,1.0);
    color = c3;
  } 
	else {
		// color = vec4(0.2,0.1,0.1,1.0);
    color = c4;
  }
	gl_FragColor = color;
  // gl_FragColor = vec4(0.4, 0.2, 0.2, 1.0);

}
