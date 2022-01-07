#pragma once
#include "AABBTree.h"
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <vector>
#include "render_enums.h"

/// <summary>
/// pure virtual class
/// </summary>
/// 

class SceneNode;
class SceneObject : public IBoundable {
public:
	SceneObject(ObjectType t, ObjectGroup g);
	virtual ~SceneObject() {}

	virtual void update(float dt) = 0;
	virtual void preRender(float dt) = 0;	// for interpolation and sheeit?

	virtual void printDebug() {
		AABB bbox = getBoundingBox();
		printf("SceneObject: type(%d), group(%d), node(%lX), aabb(%.2f %.2f %.2f | %.2f %.2f %.2f)\n",
			type, group, (unsigned long)node, 
			bbox.min.x, bbox.min.y, bbox.min.z,
			bbox.max.z, bbox.max.y, bbox.max.z
			);
	}

	ObjectType type;
	ObjectGroup group;
	SceneNode* node;

	// Inherited via IBoundable
	virtual AABB getBoundingBox() override;

	// Inherited via ITransformable
	virtual bool getObjectLocalTransform(glm::mat4* m) const;
};

class NullObject : public SceneObject {
public:
	NullObject();
	// Inherited via SceneObject
	virtual void update(float dt) override;
	virtual void preRender(float dt) override;
};

/// <summary>
/// a representation of scene node
/// </summary>
class SceneNode {
	friend class SceneGraph;
public:
	SceneNode(SceneObject *o);
	virtual ~SceneNode();

	void setParent(SceneNode* p);
	void addChild(SceneNode* c);
	void removeChild(SceneNode* c);

	virtual void update(float dt);

	// the transformation matrix (nope, should be got from object, no?)
	glm::mat4 transform;

	// parent child
	SceneNode* parent;
	std::vector<SceneNode*> children;

	// the object (could be mesh, light, camera, etc)
	SceneObject* obj;

protected:
	void updateTransform();
};

/// <summary>
/// list of node
/// </summary>
class SceneGraph {
public:
	enum {
		CLIP_RENDERABLES = (1 << 0),
		CLIP_LIGHTS = (1 << 1),
		CLIP_CAMERAS = (1 << 2),

		CLIP_STATIC = (1 << 3),
		CLIP_DYNAMIC = (1 << 4),

		CLIP_ALL = CLIP_RENDERABLES | CLIP_LIGHTS | CLIP_CAMERAS | CLIP_STATIC | CLIP_DYNAMIC
	};

	SceneGraph();
	~SceneGraph();

	// add a scene object
	SceneNode* addObject(SceneObject* o, SceneNode* n = nullptr);
	// update teh scenegraph
	void update(float dt);
	// get all scene objects
	void clip(const Frustum* f, int clip_flags, std::vector<const SceneObject*>& clipped) const;

	// print debug
	void printDebug() const;

	// root scene node
	SceneNode* root;

	// the aabb tree?
	AABBTree* dynamic_tree;
	AABBTree* static_tree;

	// based on object type
	std::vector<SceneNode*> renderables;
	std::vector<SceneNode*> lights;
	std::vector<SceneNode*> cameras;
};