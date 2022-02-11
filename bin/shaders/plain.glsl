//------------------------------------VERTEX SHADER---------------------------------------------------
#ifdef VERTEX_SHADER

uniform mat4 m_model_view_projection;
uniform mat3 m_normal;

attribute vec3 position;
attribute vec2 uv;
attribute vec3 normal;

varying vec2 v_texcoord;
varying vec3 v_normal_view;

void main() {
	v_texcoord = uv;
    v_normal_view = m_normal * normal;

	gl_Position = m_model_view_projection * vec4(position, 1.0);
}

#endif//VERTEX_SHADER

//------------------------------------FRAGMENT SHADER---------------------------------------------------
#ifdef FRAGMENT_SHADER
#ifdef GL_ES
precision mediump float;
#endif//GL_ES

uniform sampler2D texture0;

varying vec2 v_texcoord;
varying vec3 v_normal_view;

void main() {
	vec4 texColor = texture2D(texture0, v_texcoord);
	
	#ifdef HAS_ALPHA_CLIP
	if (texColor.a < 0.5) discard;
	#endif//HAS_ALPHA_CLIP

	float mult = max(0.2, v_normal_view.z);
	
	gl_FragColor = texColor * mult;
}

#endif//FRAGMENT_SHADER
