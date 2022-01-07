#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D texture0;

varying vec2 vTexcoord;
varying vec3 vNormal;	// normal in view space
varying vec3 vSunDirection;	// sun direction in view space
varying vec3 vEye;

// some scene data
uniform vec4 scene_ambient_color;
uniform vec4 sun_color;
uniform float sun_intensity;

// material data?
uniform float material_glossiness;
uniform float material_fresnel0;
uniform vec4 material_specular;

const float MAX_SHININESS = 255.0;
const float GAMMA = 2.2;

vec4 gammaDecode(vec4 color) {
	return vec4(pow(color.xyz, vec3(GAMMA)), color.a);
}

vec4 gammaEncode(vec4 color) {
	return vec4(pow(color.xyz, vec3(1.0/GAMMA)), color.a);
}

// compute our final color, baseDiffuse and baseSpecular assumed to be in linear space
vec4 computeFinalColor(vec4 baseDiffuse, vec4 baseSpecular, float glossiness, float F0) {
	// INPUTS
	//----------------------------------------------------------------------
	// N = normal
	vec3 n_normal = normalize(vNormal);
	// V = eye vector
	vec3 eyeDir = normalize(vEye);
	// H = half vector between eye and sun dir
	vec3 halfVec = normalize((vSunDirection + eyeDir));
	
	// SOME TERMS
	//----------------------------------------------------------------------
	// N.L
	float NDotL = max(0.0, dot(n_normal, vSunDirection));
	// H.N
	float HDotN = max(0.0, dot(halfVec, n_normal));
	// N.V
	float NDotV = max(0.0, dot(n_normal, eyeDir));
	
	// alpha
	float a = glossiness * glossiness;
	
	// fresnel?
	float fresnel = pow(1.0-NDotV, (a * (1.0-F0)) * MAX_SHININESS) * (1.0-(a+F0)) * (a + F0);
	
	// diffuse
	float diffuseTerm = NDotL * sun_intensity;
	
	// specular
	float specularTerm = sun_intensity * pow(HDotN, a * MAX_SHININESS) * NDotL * a;
	
	// so, we got what we need, accumulate them
	vec4 finalColor = baseDiffuse * (scene_ambient_color + sun_color * vec4(vec3(diffuseTerm), 1.0)) + (sun_color * baseSpecular * (specularTerm + fresnel));
	
	finalColor = clamp(finalColor, 0.0, 1.0);
	return finalColor;
}


void main() {
	vec4 baseColor = gammaDecode(texture2D(texture0, vTexcoord));
	vec4 finalColor = computeFinalColor(baseColor, material_specular, material_glossiness, material_fresnel0);
	gl_FragColor = gammaEncode(finalColor);
}