#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D texture0;

varying vec2 vTexcoord;

void main() {
	vec4 color = texture2D(texture0, vTexcoord);
	if (color.a < 0.5) {
		discard;
	}
	gl_FragColor = color;
}