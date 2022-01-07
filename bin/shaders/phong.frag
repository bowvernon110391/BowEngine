#ifdef GL_ES
precision mediump float;
#endif

varying vec3 vNormal;	// normal in view space
varying vec3 vSunDirection;	// sun direction in view space
varying vec3 vEye;

varying vec2 vTexcoord;

// some scene data
uniform vec4 scene_ambient_color;
uniform vec4 sun_color;
uniform float sun_intensity;

// material data?
uniform float material_glossiness;
uniform float material_fresnel0;
uniform vec4 material_specular;
uniform vec4 material_diffuse;

uniform sampler2D texture0;
uniform sampler2D texture1;

uniform mat3 m_normal;

const float MAX_SHININESS = 255.0;
const float GAMMA = 2.2;
const float PI = 3.141592653;

vec4 gammaEncode(vec4 color) {
	return vec4( pow(color.rgb, vec3(1.0/GAMMA)), color.a);
}

vec4 gammaDecode(vec4 color) {
	return vec4( pow(color.rgb, vec3(GAMMA)), color.a);
}

float schlickFresnel(float NDotH, float F0) {
	return F0 + (1.0 - F0) * pow(1.0-NDotH, 5.0);
}

vec3 blinnPhongConservative(vec3 albedo, vec3 ambient, vec3 sunColor, vec3 N, vec3 L, vec3 H, float gloss, float F0) {
	// for now, assume dielectric
	vec3 specColor = sunColor;
	
	// some terms
	float NDotL = max(dot(N, L), 0.0);
	float NDotH = max(dot(N, H), 0.0);
	
	// m in range [0..255]
	float m = max(0.0001, gloss * MAX_SHININESS);
	
	// blend to total diffuse when glossiness falls below F0
	float modifier = smoothstep(0.0, max(0.0001, F0), gloss);
	
	// to conserve energy better, acommodate fresnel factor
	float Rfh = schlickFresnel(NDotH, F0) * modifier;
	// and blinn phong normalization factor
	float normFactor = (m + 8.0) / 8.0;
	
	vec3 ambientTerm = ambient * albedo;
	vec3 diffuseTerm = albedo;
	vec3 specularTerm = specColor * (Rfh * pow(NDotH, m) * normFactor);
	
	// ambient term is mixed by (1.0-NDotL) amount (exist when no lighting occurs)
	vec3 finalColor = sunColor * ( ambientTerm + ((diffuseTerm + specularTerm) * NDotL));
	
	return finalColor;
}

float grayscale(vec3 color) {
	return color.r * 0.2 + color.g * 0.7 + color.b * 0.1;
}

vec4 getDualParaboloid(vec3 e, vec3 n, sampler2D tex, float glossiness) {
	float roughness = 1.0 - glossiness;
	const float MAX_MIP_LEVEL = 7.0;
	// const float BORDER = 2.0;
	// const float OFFSET_Y = BORDER/128.0;	// shift by 1 pixel
	// const float OFFSET_X = BORDER/256.0;	// shift by 1 pixel
	
	float mip_target = roughness * MAX_MIP_LEVEL;
	// reflect it first (make sure e, n is normalized)
	vec3 r = reflect(e, n);
	
	// r is in view space, convert to world space
	vec3 m0 = m_normal[0];
	vec3 m1 = m_normal[1];
	vec3 m2 = m_normal[2];
	//r = m_to_world * r;
	vec3 rt = vec3(
		dot(m0, r),
		dot(m1, r),
		dot(m2, r)
	);
	r = rt;
	
	// use the xz, y is just selector
	// xz is [-1..1], map it to the offset
	//r.x += r.x * (mip_target) * OFFSET_X;
	//r.z += r.z * (mip_target) * OFFSET_Y;
	
	vec2 uv = r.xz * 0.5 + 0.5;
	uv.x *= 0.5;	// shrink the x, cause two texture side by side
	
	// sample 2?
	float up_fraction = step(0.0, r.y);
	float down_fraction = 1.0 - up_fraction;
	
	// sample 2 times
	vec4 col_down = texture2D(tex, uv, mip_target);
	
	// shift and flip too
	uv.x = -uv.x + 1.0;
	vec4 col_up = texture2D(tex, uv, mip_target);
	
	vec4 normal_mix = mix(col_up, col_down, down_fraction);
	
	return normal_mix;
}

vec2 sphereMapCoord(vec3 E, vec3 N) {
	vec3 r = reflect(E, N);
	float m = 2. * sqrt(
		pow(r.x, 2.) + 
		pow(r.y, 2.) +
		pow(r.z+1., 2.)
	);
	
	return r.xy / m + 0.5;
}

vec4 getSphereMap(vec3 E, vec3 N, sampler2D spec_map, sampler2D diff_map, float glossiness) {
	const float MAX_MIP_LEVEL = 4.0;
	float roughness = (1.0 - glossiness);
	//roughness *= roughness;
	
	float mip_target = roughness * MAX_MIP_LEVEL;
	
	vec2 uv = sphereMapCoord(E, N);
	vec4 spec = gammaDecode(texture2D(spec_map, uv, mip_target));
	vec4 diff = gammaDecode(texture2D(diff_map, uv));
	
	return mix(spec, diff, roughness);
}

void main() {
	vec3 N = normalize(vNormal);
	vec3 V = normalize(vEye);
	vec3 L = vSunDirection;
	vec3 H = normalize(V + L);

	vec4 finalColor = vec4(1.0);
	
	float NDotV = max(0.0, dot(V, N));
	// scale down based on roughness value?	
	float fresnel = schlickFresnel(NDotV, material_fresnel0);
	float fresnel_factor = pow(material_glossiness, 1.0 - material_fresnel0);
	
	vec4 reflection = getSphereMap(-V, N, texture0, texture1, material_glossiness);//getDualParaboloid(-V, N, texture0, material_glossiness);
	// vec4 diff_ibl = gammaDecode(texture2D(texture1, N.xy * 0.5 + 0.5));
	
	vec3 lightColor = blinnPhongConservative(material_diffuse.rgb, scene_ambient_color.rgb, sun_color.rgb, N, L, H, material_glossiness, material_fresnel0);
	
	finalColor.rgb = mix(lightColor, reflection.rgb, fresnel * fresnel_factor);
	// finalColor.rgb = vec3(fresnel * fresnel_factor);
	// finalColor = reflection;
	// finalColor.rgb = finalColor.rgb / (finalColor.rgb + vec3(1.0));
	// gamma encode
	gl_FragColor = gammaEncode(finalColor);
}