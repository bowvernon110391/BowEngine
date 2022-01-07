#ifndef __MESH_H__
#define __MESH_H__

#include <cstdint>
#include <vector>
#include <stdio.h>
#include "Helper.h"
#include "AABB.h"
#include "Resource.h"

#define DEBUG_BCF   1

#define VF_XYZ      (1<<0)  // 12 bytes xyz
#define VF_NORMAL   (1<<1)  // 12 bytes xyz
#define VF_UV       (1<<2)  // 8 bytes xy
#define VF_TANGENT  (1<<3)  // 24 bytes xyz xyz
#define VF_UV2      (1<<4)  // 8 bytes xy
#define VF_COLOR    (1<<5)  // 16 bytes xyzw

typedef struct SubMesh
{
    // this contains the indices begin-end only
    // idxBegin is already a memory offset (not index)
    // elemCount also already total count of elements
    uint16_t idxBegin, elemCount;
    char materialName[32];
} SubMesh_t;

class Mesh : public Resource
{
public:    
    // raw buffer
    size_t vertexBufferSize;
    void* vertexBuffer;
    size_t indexBufferSize;
    unsigned short* indexBuffer;

    //std::vector<float> vertBuffer; // float buffer containing our mesh vertices data
    //std::vector<uint16_t> idxBuffer;
    std::vector<SubMesh_t> subMeshes;

    uint32_t vertexFormat;
    uint16_t strideLength;  // vertex size per bytes

	uint32_t vbo;
	uint32_t ibo;

    char name[32];

    AABB boundingBox;

    Mesh():
	strideLength(12), vertexFormat(VF_XYZ), vbo(0), ibo(0), vertexBuffer(0), indexBuffer(0)
	{
    }

    ~Mesh();

    virtual const char* type() { return "MESH"; }

	// instantiate buffer
	Mesh* createBufferObjects();

	// bind buffers
	void bind() const;

    // helper shiet
    static Mesh* createTexturedQuad(float w) {
        if (w <= 0.f)
            w = 1.f;

        const float half = .5f * w;

        Mesh* m = new Mesh();

        m->vertexFormat = VF_XYZ | VF_COLOR | VF_UV;
        m->strideLength = sizeof(float) * 8;

        // create 4 vertices
        float verts[] = {
            -half, -half, 0,    1, 0, 0,    0, 0,
            half, -half, 0,     0, 1, 0,    1, 0,
            half, half, 0,      0, 0, 1,    1, 1,
            -half, half, 0,     1, 1, 0,   0, 1
        };

        /*std::vector<float>& v = m->vertBuffer;
        v.insert(v.end(), verts, verts + 32);*/
        m->vertexBuffer = new char[sizeof(verts)];
        m->vertexBufferSize = sizeof(verts);
        memcpy(m->vertexBuffer, verts, sizeof(verts));

        // the indices is simple
        uint16_t indices[] = {
            0, 1, 2,
            0, 2, 3
        };
        
        /*std::vector<uint16_t>& idx = m->idxBuffer;
        idx.insert(idx.end(), indices, indices + 6);*/
        int idxCount = sizeof(indices) / sizeof(indices[0]);
        m->indexBuffer = new unsigned short[idxCount];
        m->indexBufferSize = sizeof(indices);

        SubMesh s;
        strcpy(s.materialName, "QUAD");
        s.idxBegin = 0;
        s.elemCount = idxCount;

        m->subMeshes.push_back(s);

        return m;
    }

    static Mesh* createUnitBox() {
        float w = 1.f;

        // now we create
        float half = .5f * w;

        // create cube with colors
        // 8 vertices
        Mesh *m = new Mesh();

        m->vertexFormat = VF_XYZ | VF_NORMAL | VF_UV;
        m->strideLength = 32;

        // create 24 vertices, front set first, then back
        // 
        float verts[] = {
            // xyz uv
            // front
            -half, -half, half, 0, 0, 1,  0, 0,
             half, -half, half, 0, 0, 1,  1, 0,
             half,  half, half, 0, 0, 1,  1, 1,
            -half,  half, half, 0, 0, 1,  0, 1,

            // back
             half, -half,-half, 0, 0,-1,  0, 0,
            -half, -half,-half, 0, 0,-1,  1, 0,
            -half,  half,-half, 0, 0,-1,  1, 1,
             half,  half,-half, 0, 0,-1,  0, 1,

             // right side
             half, -half, half, 1, 0, 0,  0, 0,
             half, -half, -half, 1, 0, 0,  1, 0,
             half, half, -half, 1, 0, 0,  1, 1,
             half, half, half, 1, 0, 0,  0, 1,

             // left side
             -half, -half, -half, -1, 0, 0,  0, 0,
             -half, -half,  half, -1, 0, 0,  1, 0,
             -half,  half,  half, -1, 0, 0,  1, 1,
             -half,  half, -half, -1, 0, 0,  0, 1,
             
             // top side
             -half,  half,  half, 0, 1, 0,  0, 0,
              half,  half,  half, 0, 1, 0,  1, 0,
              half,  half, -half, 0, 1, 0,  1, 1,
             -half,  half, -half, 0, 1, 0,  0, 1,
             
             // bottom side
             -half, -half, -half, 0, -1, 0,  0, 0,
              half, -half, -half, 0, -1, 0,  1, 0,
              half, -half,  half, 0, -1, 0,  1, 1,
             -half, -half,  half, 0, -1, 0,  0, 1,
        };

        m->boundingBox.min = glm::vec3(-half, -half, -half);
        m->boundingBox.max = glm::vec3(half, half, half);
        
        /*std::vector<float> &v = m->vertBuffer;
        v.insert(v.end(), verts, verts+(sizeof(verts)/sizeof(float)));*/
        m->vertexBuffer = new char[sizeof(verts)];
        m->vertexBufferSize = sizeof(verts);
        memcpy(m->vertexBuffer, verts, sizeof(verts));

        // index em
        uint16_t indices[36] = {
            0, 1, 2, 0, 2, 3,   // front
            4, 5, 6, 4, 6, 7,   // back
            8, 9, 10, 8, 10, 11,   // right
            12, 13, 14, 12, 14, 15,   // left
            16, 17, 18, 16, 18, 19,   // top            
            20, 21, 22, 20, 22, 23   // bottom
        };

        /*std::vector<uint16_t> &idx = m->idxBuffer;
        idx.insert(idx.end(), indices, indices+36);*/
        int idxCount = sizeof(indices) / sizeof(indices[0]);
        m->indexBufferSize = sizeof(indices);
        m->indexBuffer = new unsigned short[idxCount];
        memcpy(m->indexBuffer, indices, m->indexBufferSize);

        // append submeshes info
        SubMesh_t s;

        s.idxBegin = 0;
        s.elemCount = idxCount;
        strcpy(s.materialName, "BOX");

        m->subMeshes.push_back(s);

        return m;
    }

    /*
    * 
    # write_to_binary, write binary file
    # FORMAT IS AS FOLLOWS:
    # 1b : vtx_format
    # 1b : bytes_per_vertex
    # 2b : vertex_count (max 65535 vertex)
    # 4b : vertex_buffer_size_in_bytes
    # 2b : sub_mesh_count
    # 32b: objname
    # 2b : total_tris
    # 24b: bounding_box (6 float)
    # --SUB_MESH_DATA_---
    # { 32b: material_name, 2b: begin_at, 2b: total_tri }
    # { vertex_buffer }
    # { index_buffer }
    */
    static Mesh* loadBCFfromMemory(const void* mem, int bufSize) {
        char* ptr = (char*) mem;
        // first, read vertex format
        Mesh* m = new Mesh();

        // 1b: vtx_format
        m->vertexFormat = *(unsigned char*)ptr;
        ++ptr;

#ifdef _DEBUG
        printf("BCF_VTX_FORMAT: %d\n", m->vertexFormat);
#endif

        // 1b: bytes_per_vertex
        m->strideLength = *(unsigned char*)ptr;
        ++ptr;

#ifdef _DEBUG
        printf("BCF_BYTES_PER_VERTEX: %d\n", m->strideLength);
#endif

        // 2b: vertex count
        unsigned short vert_count = *(unsigned short*)ptr;
        ptr += 2;

#ifdef _DEBUG
        printf("BCF_VERTEX_COUNT: %d\n", vert_count);
#endif

        // 4b: vbuffer size
        m->vertexBufferSize = *(unsigned int*)ptr;
        ptr += 4;
        
#ifdef _DEBUG
        printf("BCF_VBUFFER_SIZE: %d\n", m->vertexBufferSize);
#endif

        // 2b: submesh_count
        int submeshCount = *(unsigned short*)ptr;
        ptr += 2;

#ifdef _DEBUG
        printf("BCF_SUBMESHES_COUNT: %d\n", submeshCount);
#endif

        // 32b: objname
        strcpy(m->name, ptr);
        ptr += 32;

#ifdef _DEBUG
        printf("BCF_OBJECT_NAME: %s\n", m->name);
#endif

        // 2b: total_tris
        int numTris = *(unsigned short*)ptr;
        ptr += 2;

        // 24b: bounding box
        float* bbox = (float*)ptr;
        ptr += 24;
        m->boundingBox.min = glm::vec3(bbox[0], bbox[1], bbox[2]);
        m->boundingBox.max = glm::vec3(bbox[3], bbox[4], bbox[5]);

#ifdef _DEBUG
        printf("BCF_TOTAL_TRIS: %d\n", numTris);
#endif

        // read all submesh data
        for (int i = 0; i < submeshCount; i++) {
            SubMesh s;

            // 32b: material_name
            strcpy(s.materialName, ptr);
            ptr += 32;

            // 2b: begin_at (index), convert to bytes by mult * 6
            s.idxBegin = *(unsigned short*)ptr;
            s.idxBegin *= 6;
            ptr += 2;

            // 2b: total_tri, convert to elem_count by mult * 3
            s.elemCount = *(unsigned short*)ptr;
            s.elemCount *= 3;
            ptr += 2;

#ifdef _DEBUG
            printf("BCF_SUBMESHES[%d]: matcap(%s), idxbegin(%d), elemcount(%d)\n", 
                i, s.materialName, s.idxBegin, s.elemCount);
#endif

            // add to mesh
            m->subMeshes.push_back(s);
        }

        // read vertex buffer
        m->vertexBuffer = new char[m->vertexBufferSize];
        memcpy(m->vertexBuffer, ptr, m->vertexBufferSize);
        ptr += m->vertexBufferSize;

#ifdef _DEBUG
        printf("BCF_VBUFFER_PTR: 0x%X\n", (unsigned int)m->vertexBuffer);
#endif

        // read index buffer
        m->indexBufferSize = numTris * 6;
        m->indexBuffer = new unsigned short[numTris * 3];
        memcpy(m->indexBuffer, ptr, m->indexBufferSize);

#ifdef _DEBUG
        printf("BCF_IBUFFER_PTR: 0x%X\n", (unsigned int)m->indexBuffer);
#endif

        return m;
    }

    static Mesh* loadBCFFromFile(const char* filename) {
        size_t contentSize;

        using namespace Helper;

        char* content = readFileContent(filename, &contentSize);

        if (content) {
            Mesh* m = loadBCFfromMemory(content, contentSize);
            delete[] content;

            return m;
        }

        return 0;
    }
};

#endif