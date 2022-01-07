#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "Light.h"
#include <vector>

#define MAX_POINT_LIGHTS		4
#define MAX_SPOT_LIGHTS			2

class Shader;
class RenderPass;
/// <summary>
/// represent scene data
/// </summary>
class SceneData {
public:
	SceneData() {
		// set some default value perhaps?
		ambientColor = glm::vec4(0.2f, 0.2f, 0.2f, 0.0f);
		sunDirection = glm::normalize(glm::vec3(1, 3, 2));
		sunColor = glm::vec4(1, 1, 1, 1);
		sunIntensity = 1.0f;

		nullLight.setDiffuse(glm::vec4(0.0f));
		nullLight.setSpecular(glm::vec4(0.0f));
		nullLight.setRadius(0.0f);
	}

	SceneData* setAmbientColor(const glm::vec4& c) { ambientColor = c; return this; }
	SceneData* setSunDirection(const glm::vec3& d) { sunDirection = glm::normalize(d); return this; }
	SceneData* setSunColor(const glm::vec4& c) { sunColor = c; return this; }
	SceneData* setSunIntensity(float i) { sunIntensity = i; return this; }


	virtual void setupData(const Shader* s, const RenderPass* rp);
	virtual void updateLightsUniformData(const glm::vec3& queryPos);
	virtual int getActivePointLightsData(int MAX_LIGHT, std::vector<Light*>& result, const glm::vec3& pos);

	glm::vec4 ambientColor;
	glm::vec3 sunDirection;
	glm::vec4 sunColor;
	float sunIntensity;

	std::vector<Light> pointLights;
	Light nullLight;	// black, uninteresting light for padding

protected:
	int activePointLightCount;
	float lightDiffuseColor[MAX_POINT_LIGHTS * 4];
	float lightSpecularColor[MAX_POINT_LIGHTS * 4];
	float lightFalloff[MAX_POINT_LIGHTS * 4];
	float lightPosition[MAX_POINT_LIGHTS * 3];
};