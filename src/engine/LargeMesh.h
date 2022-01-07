#pragma once
#include <vector>
#include "AABB.h"
#include "Mesh.h"
#include "Resource.h"

class KDTreeNode {
public:
	KDTreeNode() {
		id = 0;
		parent_id = -1;
		children.reserve(2);	// hold 2 children
		// reference data (set later after loading)
		parent = nullptr;
		mesh = nullptr;
	}
	~KDTreeNode() {}

	bool isRoot() {
		return !parent;
	}

	bool isLeaf() {
		return children.size() == 0;
	}

	AABB bbox;
	int id, parent_id;
	KDTreeNode* parent;
	std::vector<KDTreeNode*> children;
	int mesh_id;
	Mesh* mesh;	// could be null for non leaf. IT DOESNT OWN IT, JUST REFER TO IT
};

/// <summary>
/// This class represent a large mesh with some BVH (KDTree)
/// </summary>
class LargeMesh : public Resource {
public:
	LargeMesh();
	~LargeMesh();

	// loader
	static LargeMesh* loadLMFFromMemory(const char* buf, size_t buflen);
	static LargeMesh* loadLMFFromFile(const char* filename);

	// Inherited via Resource
	virtual const char* type() override;

	// helper
	LargeMesh* createBufferObjects();
	KDTreeNode* findNodeByMeshId(int id);

	// nodes
	std::vector<KDTreeNode> nodes;
	// meshes
	std::vector<Mesh*> meshes;

	// header data
	int node_count, mesh_count, submesh_per_mesh;
	int vertex_format;
	size_t vertex_size;
	char name[32];
protected:
	// helper
	char* parseHeader(const char* buf);
	char* parseNode(const char* buf, KDTreeNode& n);
	char* parseMesh(const char* buf, Mesh* m);
	void setupReferences();
};