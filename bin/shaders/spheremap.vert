
attribute vec3 position;
attribute vec3 normal;
attribute vec2 uv;

uniform mat4 m_model_view_projection;
uniform mat4 m_model_view;
uniform mat3 m_normal;

varying vec3 vNormal;
varying vec3 vEye;

void main() {
	vec4 p = vec4(position, 1.0);
	
	vNormal = normalize(m_normal * normal);
	vEye = normalize(vec3(m_model_view * p));
	
	gl_Position = m_model_view_projection * p;
}