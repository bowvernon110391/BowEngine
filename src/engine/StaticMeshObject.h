#pragma once
#include "AbstractRenderObject.h"
#include "AABB.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Mesh;

class StaticMeshObject : public AbstractRenderObject {
public:
	StaticMeshObject() : 
		AbstractRenderObject(ObjectGroup::OG_STATIC)
	{
		bbox_dirty = true;

		setPosition(glm::vec3(0, 0, 0));
		setRotation(glm::quat(1, 0, 0, 0));
	}

	StaticMeshObject* setPosition(const glm::vec3& p) { bbox_dirty = true; pos = p; return this; }
	StaticMeshObject* setRotation(const glm::quat& r) { bbox_dirty = true; rot = r; return this; }
	StaticMeshObject* setMesh(Mesh* m) { meshRef = m; return this; }

	// got a position, and a rotation
	glm::vec3 pos;
	glm::quat rot;

	// and a mesh
	Mesh* meshRef;

	// the up to date aabb
	AABB bbox;	// the up to date
	bool bbox_dirty;

	// Inherited via AbstractRenderObject
	virtual void update(float dt) override;
	virtual void preRender(float dt) override;
	virtual void setupData(const Shader* shd) const override;
	virtual GeomType getGeometryType() const override;
	virtual const Mesh* getMeshLayout() const override;

	// override getbounding box
	virtual AABB getBoundingBox() override;

	// Inherited via AbstractRenderObject
	// override get matrix, and local transform
	virtual bool getObjectLocalTransform(glm::mat4* m) const override;
	virtual bool getTransform(glm::mat4* m) const override;

protected:
	void updateBBox();

};