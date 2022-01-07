#include "LargeMesh.h"
#include "Helper.h"

LargeMesh::LargeMesh()
{
    mesh_count = node_count = submesh_per_mesh = 0;
    vertex_size = 0;
    vertex_format = 0;
    meshes.clear();
    nodes.clear();
}

LargeMesh::~LargeMesh()
{
    // clean up
    for (auto m : meshes)
        delete m;
    meshes.clear();
    nodes.clear();
}

LargeMesh* LargeMesh::loadLMFFromMemory(const char* buf, size_t buflen)
{
    if (!buf) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LMF_PARSE: Invalid buffer!");
        return nullptr;
    }
    // instantiate new object
    LargeMesh* l = new LargeMesh();

    // first, make it parse header data
    char* ptr = l->parseHeader(buf);

    // are we done?
    if (!ptr) {
#ifdef _DEBUG
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LMF_PARSE: Error when parsing header @ %X!\n", buf);
#endif // _DEBUG

        // something goes wrong
        delete l;
        return nullptr;
    }

    // header parsed. now read node data
    l->nodes.reserve(l->node_count);

    // for every node, read em
    for (int i = 0; i < l->node_count; i++) {
        // make new node
        KDTreeNode node;
        // read node data, and store it?
        ptr = l->parseNode(ptr, node);

        if (!ptr) {
            // we failed
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LMF_PARSE: Error parsing node[%d]\n", i);
            delete l;
            return nullptr;
        }

        // we parsed the data, store it
        l->nodes.push_back(node);
    }

    // node parsed, now read mesh data
    l->meshes.reserve(l->mesh_count);
    // for every mesh, read em
    for (int i = 0; i < l->mesh_count; i++) {
        Mesh* m = new Mesh();
        // set some unfilled data (name)
        sprintf(m->name, "mesh_%03d", i);
        m->id = 0xFFFF - i;
        // parse it
        ptr = l->parseMesh(ptr, m);
        if (!ptr) {
            // we failed
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LMF_PARSE: Error parsing mesh[%d]\n", i);
            delete m;
            delete l;
            return nullptr;
        }

        // mesh parsed. store it
        l->meshes.push_back(m);
    }

    // setup pointer and shiet
    l->setupReferences();

    // it works then
    return l;
}

LargeMesh* LargeMesh::loadLMFFromFile(const char* filename)
{
    char* buf;
    size_t buflen;

    buf = Helper::readFileContent(filename, &buflen);

    LargeMesh* lm = loadLMFFromMemory(buf, buflen);

    delete[] buf;

    return lm;
}

char* LargeMesh::parseHeader(const char* buf)
{
    // set ptr to buf
    char* ptr = (char*)buf;
    
    // 1b: vertex_format
    vertex_format = *(unsigned char*)ptr;
    ++ptr;

    // 1b: vertex_size
    vertex_size = *(unsigned char*)ptr;
    ++ptr;

    // 2b: node_count
    node_count = *(unsigned short*)ptr;
    ptr += 2;

    // 2b: mesh_count
    mesh_count = *(unsigned short*)ptr;
    ptr += 2;

    // 2b: submesh_per_mesh
    submesh_per_mesh = *(unsigned short*)ptr;
    ptr += 2;

    // 32b: name
    memcpy(name, ptr, 32);
    ptr += 32;

#ifdef _DEBUG
    SDL_Log("LMF_PARSE_HEADER: name(%s), vtx_format(%d), vtx_size(%d), nodes(%d), meshes(%d), submesh(%d)\n",
        name, vertex_format, vertex_size, node_count, mesh_count, submesh_per_mesh);
#endif // _DEBUG


    return ptr;
}

char* LargeMesh::parseNode(const char* buf, KDTreeNode& n)
{
    char* ptr = (char*)buf;
    // 4b: id
    n.id = *(int*)ptr;
    ptr += 4;
    //SDL_Log("LMF_PARSE_NODE: id = %d\n", n.id);


    // 4b: parent_id
    n.parent_id = *(int*)ptr;
    ptr += 4;
    //SDL_Log("LMF_PARSE_NODE: parent_id = %d\n", n.parent_id);

    // 24b: 6 float for aabb data
    float* f = (float*)ptr;
    n.bbox.min = glm::vec3(f[0], f[1], f[2]);
    n.bbox.max = glm::vec3(f[3], f[4], f[5]);
    ptr += 24;

    // 4b: mesh_id
    n.mesh_id = *(int*)ptr;
    ptr += 4;

#ifdef _DEBUG
    SDL_Log("LMF_PARSE_NODE: Node[%d], parent(%d), aabb(%.2f %.2f %.2f | %.2f %.2f %.2f) mesh(%d)\n",
        n.id, n.parent_id, n.bbox.min.x, n.bbox.min.y, n.bbox.min.z, n.bbox.max.x, n.bbox.max.y, n.bbox.max.z,
        n.mesh_id
        );
#endif // _DEBUG

    return ptr;
}

char* LargeMesh::parseMesh(const char* buf, Mesh* m)
{
    char* ptr = (char*)buf;

    // copy data like vertex format, size
    m->vertexFormat = vertex_format;
    m->strideLength = vertex_size;

    // 4b: mesh_data_size
    int mesh_bytes = *(unsigned int*)ptr;
    ptr += 4;

    // 2b: vertex_count
    unsigned short vertex_count = *(unsigned short*)ptr;
    ptr += 2;
    // 2b: tri_count
    unsigned short tri_count = *(unsigned short*)ptr;
    ptr += 2;

    // submeshes data
    // ---------------------------------------------------
    for (int i = 0; i < submesh_per_mesh; i++) {
        // spawn new submesh data
        SubMesh sm;
        // 2b: offset_start
        sm.idxBegin = *(unsigned short*)ptr;
        ptr += 2;
        // 2b: elem_count
        sm.elemCount = *(unsigned short*)ptr;
        ptr += 2;
        // add submesh to mesh
        m->subMeshes.push_back(sm);
    }

    // vertex buffer
    // ----------------------------------------------------
    // compute size
    m->vertexBufferSize = vertex_count * vertex_size;
    // allocate buffer
    m->vertexBuffer = new char[m->vertexBufferSize];
    // copy them
    memcpy(m->vertexBuffer, ptr, m->vertexBufferSize);
    // offset
    ptr += m->vertexBufferSize;

    // index buffer (tri_count * 6)
    // ----------------------------------------------------
    m->indexBufferSize = tri_count * 6;
    // allocate buffer
    m->indexBuffer = (unsigned short*) new char[m->indexBufferSize];
    // copy them
    memcpy(m->indexBuffer, ptr, m->indexBufferSize);
    // offset
    ptr += m->indexBufferSize;

#ifdef _DEBUG
    SDL_Log("LMF_PARSE_MESH: name(%s) vtx_count(%d) tri_count(%d) vbsize(%d) ibsize(%d)\n",
        m->name, vertex_count, tri_count, m->vertexBufferSize, m->indexBufferSize
        );
#endif // _DEBUG
        
    return ptr;
}

void LargeMesh::setupReferences()
{
    SDL_Log("LMF_PARSE: Setting reference data...\n");
    // iterate over nodes, set parent pointer + mesh pointer
    for (KDTreeNode& n : nodes) {
        if (n.parent_id >= 0)
            n.parent = &nodes[n.parent_id];

        if (n.mesh_id >= 0) {
            n.mesh = meshes[n.mesh_id];
            n.mesh->boundingBox = n.bbox;   // copy node bbox as mesh bbox
        }
    }
    SDL_Log("LMF_PARSE: Done.\n");
}

const char* LargeMesh::type()
{
    return "LARGE_MESH";
}

LargeMesh* LargeMesh::createBufferObjects() {
    // loop all over mesh, create the buffer objects
    for (Mesh* m : meshes) {
        m->createBufferObjects();
    }

    return this;
}

KDTreeNode* LargeMesh::findNodeByMeshId(int id)
{
    if (id < 0 || id >= (int)meshes.size())
        return nullptr;
    // do linear search?
    // yep, from the back though
    for (auto it = nodes.rbegin(); it != nodes.rend(); ++it) {
        if (it->mesh_id == id)
            return &(*it);
    }
    return nullptr;
}
