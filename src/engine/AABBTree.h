#pragma once
#include "AABB.h"
#include "Traits.h"
#include "Camera.h"
#include <stdio.h>
#include <queue>
#include <unordered_map>
#include <stack>

class Renderer;

class AABBNode {
public:
	AABBNode(IBoundable* obj, float fatten=0.2f):
		obj(obj), parent(nullptr), left(nullptr), right(nullptr)
	{
		if (obj) {
			bbox = obj->getBoundingBox();
			bbox.expand(fatten);
		}
	}

	bool isLeaf() const {
		return !(left || right);
	}

	bool isRoot() const {
		return !parent;
	}

	AABBNode* sibling() const {
		// only valid if we have parent
		if (!parent) 
			return nullptr;
		return parent->left == this ? parent->right : parent->left;
	}

	// can we rotate?
	bool canRotate() const {
		if (isLeaf()) return false;
		if (left->isLeaf() && right->isLeaf()) return false;
		return true;
	}

	// cost of combination with other node
	float computeCost(const AABBNode* other) const {
		// return computed cost
		return AABB::combined(bbox, other->bbox).area();
	}

	// reduction in cost if we're swapping child with sibling
	float potentialReduction(AABBNode** candidate=nullptr) {
		if (isLeaf()) {
			return 0.f;
		}

		AABBNode* s = sibling();
		// if we have no sibling, cannot reduce (root)
		if (!s) {
			return 0.f;
		}

		// compute the greatest reduction
		float initSA = bbox.area();
		float reducedLeft = initSA - AABB::combined(left->bbox, s->bbox).area();
		reducedLeft = reducedLeft < 0 ? 0 : reducedLeft;
		float reducedRight = initSA - AABB::combined(right->bbox, s->bbox).area();
		reducedRight = reducedRight < 0 ? 0 : reducedRight;

		// which is greater?
		if (reducedLeft > reducedRight) {
			if (candidate)
				*candidate = left;
			return reducedLeft;
		}

		if (candidate)
			*candidate = right;
		return reducedRight;
	}

	// returns true if bbox changed
	bool refit() {
		// refit bounding box, only makes sense for branch node
		if (isLeaf()) {
			return false;
		}

		// okay, we have left and right, so compute new bbox
		float SA = bbox.area();
		bbox = AABB::combined(left->bbox, right->bbox);

		return bbox.area() != SA;
	}

	// this is what the aabb is like when rotation occurs
	AABB potentialRotatedBox() const {
		if (!canRotate()) {
			return AABB();
		}

		// compute for real?
		float c1, c2;
		AABBNode* cd1, * cd2;
		c1 = left->potentialReduction(&cd1);
		c2 = right->potentialReduction(&cd2);

		if (c1 > c2 && c1 > 0.f) {
			return AABB::combined(cd1->bbox, right->bbox);
		}
		else if (c2 > 0.f) {
			return AABB::combined(cd2->bbox, left->bbox);
		}

		return AABB();
	}

	// do some kind of rotation? return true if we do rotation
	bool rotate() {
		// if only two rotation possible, sibling vs left, sibling vs right
		// and only possible if we have parent and we have children (not leaf)
		if (isLeaf()) {
#ifdef _DEBUG
			printf("TREE_ROTATE: Node[%lX] is leaf!\n", (unsigned long)this);
#endif // _DEBUG
			return false;
		}
		// if both children are leaves, don't do shit
		if (left->isLeaf() && right->isLeaf()) {
#ifdef _DEBUG
			printf("TREE_ROTATE: Node[%lX] children are all leaves!\n", (unsigned long)this);
#endif // _DEBUG
			return false;
		}
		// at least one of the child is a branch!, so build possibilities
		AABBNode* p1 = nullptr, * p2 = nullptr;
		AABBNode* c1, * c2;
		float redLeft = left->potentialReduction(&c1);
		float redRight = right->potentialReduction(&c2);

		if (redLeft > redRight && redLeft > 0.0f) {
			// do left rotation?
#ifdef _DEBUG
			printf("TREE_ROTATE: Node[%lX] Left Rotate with (%lX)\n", (unsigned long)this, (unsigned long)c1);
#endif // _DEBUG
			p1 = right;
			p2 = c1;
		} else if (redRight > 0.0f) {
			// do right rotation?
#ifdef _DEBUG
			printf("TREE_ROTATE: Node[%lX] Right Rotate with (%lX)\n", (unsigned long)this, (unsigned long)c2);
#endif // _DEBUG
			p1 = left;
			p2 = c2;
		}

		// do the rotation? NOPE! p1 and p2 must be grouped together!!
		if (p1 && p2) {
			// p1 always higher than p2
			// p2.grandParent = p1.parent

			// grab the old parent, and whos gon git thrown away
			AABBNode* oldParent = p2->parent;
			AABBNode* orphan = oldParent->left == p2 ? oldParent->right : oldParent->left;

			// change p1
			if (p1 == left) {
				left = orphan;
			}
			else {
				right = orphan;
			}
			orphan->parent = this;

			// let p1 take the orphan's place
			if (oldParent->left == orphan) {
				oldParent->left = p1;
			}
			else {
				oldParent->right = p1;
			}
			p1->parent = oldParent;
			oldParent->refit();
			return true;
		}
#ifdef _DEBUG
		else {
			printf("TREE_ROTATE: Node[%lX] (NO POSSIBLE ROTATION)!\n", (unsigned long)this);
		}
#endif // _DEBUG

		return false;
	}

	void debugPrint(int lvl = 0) {
		// print our self, and our child?
		char tabs[32] = { 0 };
		for (int i = 0; i < lvl; i++) {
			tabs[i] = '=';
		}
		tabs[lvl] = 0;

		printf("%s> (%X), root(%s), leaf(%s), bbox(%.2f %.2f %.2f | %.2f %.2f %.2f) parent(%X)\n",
			tabs, (size_t) this, isRoot()?"Y":"N", isLeaf()?"Y":"N", 
			bbox.min.x, bbox.min.y, bbox.min.z, bbox.max.x, bbox.max.y, bbox.max.z,
			(size_t) parent);
		if (!isLeaf()) {
			// got child, recurse
			printf("%s> (left):\n", tabs);
			left->debugPrint(lvl+1);
			
			printf("%s> (right):\n", tabs);
			right->debugPrint(lvl+1);
		}
	}

	AABB bbox;
	IBoundable* obj;
	// relations
	AABBNode* parent, * left, * right;
};

class AABBTree {
public:
	AABBTree() {
		objs.clear();
		root = nullptr;
		selected = nullptr;
	}
	~AABBTree() {
		// do we own the render objects? nope. so just clear containers
		objs.clear();
		
		if (root) {
			// delete recursively?
			std::stack<AABBNode*> s;
			s.push(root);

			while (!s.empty()) {
				AABBNode* n = s.top();
				s.pop();

				if (!n->isLeaf()) {
					s.push(n->left);
					s.push(n->right);
				}

				// delete here
				delete n;
			}
		}
	}

	AABBNode* findNode(IBoundable* obj) {
		if (objs.find(obj) == objs.end())
			return nullptr;
		return objs.at(obj);
	}

	void debugDraw(Renderer* r);
	void debugPrint() {
		if (root) {
			root->debugPrint();
		}
		else {
			printf("NO_ROOT_ERROR!!\n");
		}
	}

	void setDebugBox(const AABB& b) {
		dbgBest = b;
	}

	void resetDebugBox() {
		dbgBest = AABB();
	}

	// operations
	AABBNode* insert(AABBNode* n);
	void remove(AABBNode* n);
	void refresh();	// maybe the boundable moved?
	void clip_leaves(const Frustum *f, std::vector<const IBoundable*>& clipped);

	// members?
	// first, vector of aabb tree?
	std::unordered_map<IBoundable*, AABBNode*> objs;
	AABBNode* root;
	AABBNode* selected;

	AABB dbgBest;

protected:
	// return best node
	AABBNode* findBestNode(const AABBNode* contender) const;
};

