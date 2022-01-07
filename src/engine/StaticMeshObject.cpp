#include "StaticMeshObject.h"
#include "ShaderFX.h"
#include "Mesh.h"

#define DEBUG_BBOX

void StaticMeshObject::update(float dt)
{
	if (bbox_dirty) {
		this->updateBBox();
	}
}

void StaticMeshObject::preRender(float dt)
{
}

void StaticMeshObject::setupData(const Shader* shd) const
{
	// nothing special, so do nothing
}

GeomType StaticMeshObject::getGeometryType() const
{
	return GeomType::STATIC;
}

const Mesh* StaticMeshObject::getMeshLayout() const
{
	return meshRef;
}
AABB StaticMeshObject::getBoundingBox()
{
	if (bbox_dirty)
		updateBBox();

	return bbox;
}
//
//bool StaticMeshObject::getTransform(glm::mat4* m) const
//{
//	SDL_assert(m != nullptr);
//	// use scene node's transform
//	*m = (glm::translate(glm::mat4(1.0f), pos) * glm::mat4_cast(rot));
//	
//#ifdef _STATIC_MESH_OBJECT_DEBUG
//	printf("STATIC_MESH_OBJECT: MODEL_MATRIX\n");
//	Helper::printMatrix(glm::value_ptr(*m));
//#endif
//	return true;
//}

void StaticMeshObject::updateBBox()
{
#ifdef DEBUG_BBOX
	printf("STATIC_MESH_BBOX_UPDATE! @ %x\n", this);
	printf("AABB TRANSFORM: \n");
	Helper::printMatrix(glm::value_ptr(node->transform));
#endif // DEBUG_BBOX

	// use transform data, not our rotation and sheit
	bbox = meshRef->boundingBox.transform(node->transform);
	bbox_dirty = false;
}

bool StaticMeshObject::getObjectLocalTransform(glm::mat4* m) const
{
	*m = glm::translate(glm::mat4(1.0f), pos) * glm::mat4_cast(rot);
	return true;
}

bool StaticMeshObject::getTransform(glm::mat4* m) const
{
	*m = node->transform;
	return true;
}

