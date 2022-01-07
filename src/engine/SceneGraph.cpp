#include "SceneGraph.h"
#include "Helper.h"
#include <stack>

//#define DEBUG_SCENE_NODE

SceneObject::SceneObject(ObjectType t, ObjectGroup g): type(t), group(g)
{
	node = nullptr;
}

AABB SceneObject::getBoundingBox()
{
	return AABB();
}

bool SceneObject::getObjectLocalTransform(glm::mat4* m) const
{
	return false;
}

//=========================================================================================

NullObject::NullObject():
	SceneObject(ObjectType::OT_RENDERABLE, ObjectGroup::OG_STATIC)
{
}

void NullObject::update(float dt)
{
	// do nothing?
}

void NullObject::preRender(float dt)
{
}

//==========================================================================================
SceneNode::SceneNode(SceneObject* o)
{
	this->parent = nullptr;
	// set relation
	obj = o;
	o->node = this;
	transform = glm::mat4(1.0);	// identity, by default
}

SceneNode::~SceneNode()
{
	// also delete children?
	for (SceneNode* c : children) {
		delete c;
	}
}

void SceneNode::setParent(SceneNode* p)
{
	if (parent)
		parent->removeChild(this);
	p->addChild(this);
}

void SceneNode::addChild(SceneNode* c)
{
	c->parent = this;
	children.push_back(c);
}

void SceneNode::removeChild(SceneNode* c)
{
	// find the child, move to back, and pop?
	for (int i = 0; i < (int)children.size(); i++) {
		if (children[i] == c) {
			SceneNode* tmp = children[(int)children.size() - 1];
			children[(int)children.size() - 1] = c;
			children[i] = tmp;
			children.pop_back();
			return;
		}
	}
}

void SceneNode::update(float dt)
{
	updateTransform();
}

void SceneNode::updateTransform()
{
	// update transform from child?
	obj->getObjectLocalTransform(&transform);
#ifdef DEBUG_SCENE_NODE
	printf("SCENE_NODE_UPDATE_TRANS: local obj @ %X\n", obj);
	printf("OBJ_LOCAL:\n");
	Helper::printMatrix(glm::value_ptr(transform));
#endif // DEBUG_SCENE_NODE


	// multiply by parent matrix
	if (parent) {
#ifdef DEBUG_SCENE_NODE
		printf("SCENE_NODE_UPDATE_TRANS: parent matrix\n");
		Helper::printMatrix(glm::value_ptr(parent->transform));
#endif // DEBUG_SCENE_NODE

		transform = parent->transform * transform;

#ifdef DEBUG_SCENE_NODE
		printf("SCENE_NODE_UPDATE_TRANS: parent * local\n");
		Helper::printMatrix(glm::value_ptr(transform));
#endif // DEBUG_SCENE_NODE
	}
}

//=============================================================================================
SceneGraph::SceneGraph()
{
	// initialize root
	root = new SceneNode(new NullObject);
	// initialize tree
	dynamic_tree = new AABBTree();
	static_tree = new AABBTree();
}

SceneGraph::~SceneGraph()
{
	if (root)
		delete root;
}

/// <summary>
/// add object, with parent n
/// </summary>
/// <param name="o">scene object</param>
/// <param name="n">parent node, null by default</param>
/// <returns>newly created node</returns>
SceneNode* SceneGraph::addObject(SceneObject* o, SceneNode* n)
{
	SceneNode* node = new SceneNode(o);
	if (n) {
		// attach to parent?
		node->setParent(n);
	}
	else {
		// attach to root instead
		node->setParent(root);
	}

	// now decide where to put the object in the list
	switch (o->type) {
	case ObjectType::OT_RENDERABLE:
		renderables.push_back(node);
		break;
	case ObjectType::OT_LIGHT:
		lights.push_back(node);
		break;
	case ObjectType::OT_CAMERA:
		cameras.push_back(node);
		break;
	}

	// maybe update its transform first? (MUST! BEFORE INSERTION!!!)
	node->updateTransform();

	// and the trees
	switch (o->group) {
	case ObjectGroup::OG_STATIC:
		static_tree->insert(new AABBNode(o));
		break;
	case ObjectGroup::OG_DYNAMIC:
		dynamic_tree->insert(new AABBNode(o));
		break;
	}

	return node;
}

void SceneGraph::update(float dt)
{
	// use iteration
	std::stack<SceneNode*> s;

	s.push(root);

	while (!s.empty()) {
		SceneNode* n = s.top();
		s.pop();

		// do something, and push children
		n->update(dt); // update is non recursive, but take into account parent node's state

		// add children
		for (SceneNode* c : n->children) {
			s.push(c);
		}
	}

	// gotta update the dynamic tree!
	dynamic_tree->refresh();
}

void SceneGraph::clip(const Frustum* f, int clip_flags, std::vector<const SceneObject*>& clipped) const
{
	std::vector<const IBoundable*> objs;

	if (clip_flags & CLIP_STATIC) {
		static_tree->clip_leaves(f, objs);
	}

	if (clip_flags & CLIP_DYNAMIC) {
		dynamic_tree->clip_leaves(f, objs);
	}

	//printf("CLIPPED: %d\n", objs.size());

	// now, filter only that is necessary
	for (const IBoundable* obj : objs) {
		const SceneObject* o = static_cast<const SceneObject*>(obj);

		//printf("IBoundable(%X), CAST(%X)\n", obj, o);

		// okay, check if it fills requirement
		if (clip_flags & CLIP_RENDERABLES && o->type == ObjectType::OT_RENDERABLE)
			clipped.push_back(o);
		if (clip_flags & CLIP_LIGHTS && o->type == ObjectType::OT_LIGHT)
			clipped.push_back(o);
		if (clip_flags & CLIP_CAMERAS && o->type == ObjectType::OT_CAMERA)
			clipped.push_back(o);
	}
}

void SceneGraph::printDebug() const
{
	// dump static, dynamic too?
	printf("STATIC_TREE:\n");
	static_tree->debugPrint();

	printf("DYNAMIC TREE:\n");
	dynamic_tree->debugPrint();
}
