#version 120

uniform sampler2D texture0; // kD
uniform sampler2D texture1; // kS
uniform sampler2D texture2; // clouds
uniform vec3 lightPosCam;

varying vec2 vTex0;
varying vec2 vTex1;
varying vec4 fragPos_cam;
varying vec3 normal_cam;

// attribute vec4 aPos;
// attribute vec3 aNor;

void main()
{
	
	vec3 kd = texture2D(texture0, vTex0).rgb;
	vec3 ks = texture2D(texture1, vTex0).rgb;
	vec3 color0 = texture2D(texture2, vTex1).rgb;


	vec4 l = normalize(vec4(lightPosCam, 1.0) - fragPos_cam);

	vec4 camPos_cam = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	vec4 e = normalize(camPos_cam - fragPos_cam);

	vec4 h = normalize(e + l);
	float dot_l_n = max(dot(vec4(normal_cam, 0.0), l), 0.0);
	float dot_h_n = max(dot(vec4(normal_cam, 0.0), h), 0.0);

	float s = 50.0;

	vec3 cd = kd * dot_l_n;
	vec3 cs = ks * pow(dot_h_n, s);
	vec3 c = cs + cd;

	float tol = 0.3;
	// if (color0[0] <= tol && color0[1] <= tol && color0[2] <= tol) {
	// 	color0 = c ;
	// }
	color0 = color0 + c;
	// gl_FragColor = vec4(kd, 1.0f);
	// gl_FragColor = vec4(l, 1.0);
	gl_FragColor = vec4(color0, 1.0);


}
