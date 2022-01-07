#include "AABBTree.h"
#include "Renderer.h"
#include "Camera.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

void AABBTree::debugDraw(Renderer* r)
{
    if (!root)
        return;

    // some colors?
    const glm::vec4 colors[] = {
        glm::vec4(1.f, 0.f, 0.f, 1.f),
        glm::vec4(1.f, 1.f, 0.f, 1.f),
        glm::vec4(0.f, 1.f, 0.f, 1.f),
        glm::vec4(0.f, 1.f, 1.f, 1.f),
        glm::vec4(0.f, 0.f, 1.f, 1.f),
        glm::vec4(1.f, 0.f, 1.f, 1.f),
    };

    AABBNode* last = nullptr;
    int lastColor = 0;
    std::queue<AABBNode*> q;
    q.push(root);

    // bind shader, and setup uniforms?
    Shader* s = r->getDebugShader();
    s->bind();

    // get uniform
    glm::mat4 mvp = r->getCamera()->getProjectionMatrix() * r->getCamera()->getViewMatrix();
    int u_loc = s->getUniformLocation(Shader::UniformLoc::m_model_view_projection);
    glUniformMatrix4fv(u_loc, 1, false, glm::value_ptr(mvp));
    int a_loc = s->getAttribLocation(Shader::AttribLoc::position);

    // bind buffer and set vertex client state?
    int vbo = r->getDebugVBO();
    int ibo = r->getDebugIBO();

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    glEnableVertexAttribArray(a_loc);
    glVertexAttribPointer(a_loc, 3, GL_FLOAT, false, 12, (void*)0);

    u_loc = s->getUniformLocation(Shader::UniformLoc::material_diffuse);
    while (!q.empty()) {
        // pop, change color, generate draw data, and draw!
        AABBNode* n = q.front();
        q.pop();
        if (n->parent && n->parent != last) {
            // change color
            lastColor = (lastColor + 1) % 6;
        }
        // set shader color
        glm::vec4 col = colors[lastColor];

        // if it's selected, change to white and thicken!
        if (n == selected) {
            col = glm::vec4(1.f);
        }

        glUniform4fv(u_loc, 1, glm::value_ptr(col));

        // generate debug draw data
        r->generateDebugData(n->bbox);

        // draw call
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, (void*)0);

        // if has children, add to queue
        if (!n->isLeaf()) {
            q.push(n->left);
            q.push(n->right);
        }
    }

    // draw dbgBest
    glUniform4f(u_loc, 0, 0, 0, 1.f);
    r->generateDebugData(dbgBest);
    // draw call
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, (void*)0);
}

AABBNode* AABBTree::insert(AABBNode* n)
{
    // since it's a leaf node, add to map first?
    objs.insert(std::make_pair(n->obj, n));

    // 1st, find best node
    AABBNode* bestNode = findBestNode(n);
    // if no best node, then we have nothing. set to root
    if (!bestNode) {
        root = n;
#ifdef _DEBUG
        printf("TREE_INSERT: NEW_ROOT[%X]!\n", n);
#endif // _DEBUG

        return n;
    }

    // welp, we got a node, gotta make a new parent and enclose em both?
    AABBNode* newParent = new AABBNode(nullptr);
    AABBNode* oldParent = bestNode->parent;

    // set bbox
    newParent->bbox = AABB::combined(n->bbox, bestNode->bbox);
    newParent->parent = oldParent;

    // set newparent's children
    newParent->left = bestNode;
    newParent->right = n;

    bestNode->parent = newParent;
    n->parent = newParent;

    // if old parent is null, then we need to promote newparent to root
    if (!oldParent) {
        root = newParent;
#ifdef _DEBUG
        printf("TREE_INSERT: NEW_PARENT_TO_ROOT[%lX]!\n", (unsigned long)root);
#endif // _DEBUG
        return root;
    }

    // otherwise, we can update parent-child relationship
    // of oldParent
    if (oldParent->left == bestNode) {
        oldParent->left = newParent;
    }
    else {
        oldParent->right = newParent;
    }

    // here we need to refit old parent, rotating it if possible
    AABBNode* ptr = oldParent;

    // rotate just this one
    while (ptr) {
        // refit and propagate
        ptr->refit();
        ptr->rotate();

        // if at least we update aabb or rotated, then keep propagating
        // upwards. otherwise, stop (LATER MAYBE!!!)
        // maybe call the sibling to rotate too?
        ptr = ptr->parent;
    }

    return newParent;
}

void AABBTree::remove(AABBNode* n)
{
    // if we're root, delete ourself, and reset root
    if (!n->parent && n == root) {
        delete n;
        root = nullptr;
        return;
    }

    // else, promote sibling into parent, or root if necessary
    AABBNode* oldParent = n->parent;
    AABBNode* sibling = n->sibling();

    if (!oldParent->parent) {
        // promote sibling to root, and delete parent
        sibling->parent = nullptr;
        root = sibling;
        delete oldParent;
        delete n;
        return;
    }

    // or, we have grand parent. a lil complicated now
    AABBNode* grandparent = oldParent->parent;
    sibling->parent = grandparent;
    if (grandparent->left == oldParent) {
        grandparent->left = sibling;
    }
    else {
        grandparent->right = sibling;
    }
    // now safe to remove parent and ourself
    delete oldParent;
    delete n;

    // now propagate rotate and refit?
    AABBNode* ptr = sibling->parent;
    while (ptr) {
        ptr->rotate();
        ptr->refit();
        ptr = ptr->parent;
    }
}

void AABBTree::refresh()
{
    // we got pair of object and nodes
    std::vector<AABBNode*> changed;
    std::vector<IBoundable*> orphaned;

    // detect which ones need changing
    std::unordered_map<IBoundable*, AABBNode*>::iterator it = objs.begin();
    while (it != objs.end()) {
        // check
        const AABB& node_aabb = it->second->bbox;
        AABB obj_aabb = it->first->getBoundingBox();

        // might have changed
        if (!AABB::contain(node_aabb, obj_aabb)) {
            changed.push_back(it->second);
            orphaned.push_back(it->first);
        }

        ++it;
    }

    // remove changed
    for (AABBNode* n : changed) {
        remove(n);
    }

    // add again orphaned one
    for (IBoundable* b : orphaned) {
        // make new node
        AABBNode *newNode = new AABBNode(b);
        insert(newNode);

        // update in the hash map (we didn't delete it)
        auto it = objs.find(b);
        SDL_assert(it != objs.end());

        it->second = newNode;
    }
}

void AABBTree::clip_leaves(const Frustum* f, std::vector<const IBoundable*>& clipped)
{
    // if doesn't have root, skip
    if (!root)
        return;

    // return all contained in leaves
    std::stack<AABBNode*> s;
    s.push(root);

    while (!s.empty()) {
        AABBNode* n = s.top();
        s.pop();

        // check
        Frustum::TestResult res = f->testAABB(&n->bbox);

        // depending on result, do something
        // if it's fully in, or if it's just intersecting but it's leaf already
        if (res == Frustum::FULLY_IN || (res == Frustum::INTERSECT && n->isLeaf())) {
            //printf("NODE(%X) FRUSTUM_FULLY_IN/INTERSECT_ON_LEAF: collecting child nodes directly...\n", n);
            // no need to check, just directly return all leaves inside
            std::stack<AABBNode*> ss;
            ss.push(n);

            while (!ss.empty()) {
                AABBNode* nn = ss.top();
                ss.pop();

                if (!nn->isLeaf()) {
                    //printf("COLLECT_CHILDREN: NODE(%X) is not leaf. add children...\n", nn);
                    ss.push(nn->left);
                    ss.push(nn->right);
                    continue;
                }
                // well it's leaf, push it
                //printf("COLLECT_CHILDREN: NODE(%X) object(%X)\n", nn, nn->obj);
                clipped.push_back(nn->obj);
            }
        }
        else if (res == Frustum::INTERSECT && !n->isLeaf()) {
            //printf("NODE(%X) FRUSTUM_INTERSECT, test children...\n", n);
            // intersect, test children
            s.push(n->left);
            s.push(n->right);
        }
        
    }
}

AABBNode* AABBTree::findBestNode(const AABBNode* contender) const
{
    if (!root) {
        return nullptr;
    }

    // we do have root and all, so construct a queue
    std::stack<AABBNode*> s;
    AABBNode* bestNode = nullptr;
    float bestCost = 0;
    // initialize stack
    s.push(root);
    // keep going until empty
    while (!s.empty()) {
        AABBNode* n = s.top();
        s.pop();
        // compute cost, if it's better or if it's first, set to it
        float cost = n->computeCost(contender);
        if (cost <= bestCost || !bestNode) {
            bestCost = cost;
            bestNode = n;

            // well if n has children it must be worth to check, so go on...
            if (!n->isLeaf()) {
                s.push(n->left);
                s.push(n->right);
            }
        }
    }

    return bestNode;
}
