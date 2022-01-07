#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D texture0;

varying vec3 vNormal;
varying vec3 vEye;

void main() {
	vec3 r = (reflect(vEye, (vNormal)));
	float m = 2. * sqrt(
		pow(r.x, 2.) + 
		pow(r.y, 2.) +
		pow(r.z+1., 2.)
	);
	
	vec2 vN = r.xy / m + 0.5;

	gl_FragColor = texture2D(texture0, vN); //vec4(vN.x, vN.y, 0.0, 1.0);
}