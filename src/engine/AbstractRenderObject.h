#pragma once
#include <vector>
#include <assert.h>
#include "AABB.h"
#include "render_enums.h"
#include "Traits.h"
#include "Shader.h"
#include "SceneGraph.h"

class Mesh;
class Renderable;
class RenderPass;
class ShaderFX;
class ShaderInstanceManager;

/// <summary>
/// This is a pure abstract render object, with only couple of responsibility, which are:
/// - SETUP SHADER DATA
/// - UPDATE
/// - PRE-RENDER UPDATE
/// - FILL RENDERABLE
/// </summary>
class AbstractRenderObject : public SceneObject, public ShaderInput, public ITransformable {
public:
	AbstractRenderObject(ObjectGroup grp) : 
		SceneObject(ObjectType::OT_RENDERABLE, grp)
	{

	}
	virtual ~AbstractRenderObject() {}

	// pure virtual
	// what kind of geometry?
	virtual GeomType getGeometryType() const = 0;
	// get buffer layout?
	virtual const Mesh* getMeshLayout() const = 0;

	AbstractRenderObject* addEffect(ShaderFX* fx) {
		effects.push_back(fx);
		return this;
	}
	// all renderable must have shader effect
	std::vector<ShaderFX*> effects;
};