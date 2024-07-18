#include <core/navmesh.hpp>
#include <core/fetch.hpp>

namespace detail {

static std::unordered_map<std::string, std::promise<std::shared_ptr<navmesh_ref>>> promises;

}

navmesh_ref::navmesh_ref(navmesh_ref&& other)
{
    *this = std::move(other);
}

navmesh_ref& navmesh_ref::operator=(navmesh_ref&& other)
{
    _shape = other._shape;
#if LUCARIA_GUIZMO
    _guizmo = std::move(other._guizmo);
#endif
    _is_instanced = true;
    other._is_instanced = false;
    return *this;
}

navmesh_ref::~navmesh_ref()
{
    if (_is_instanced) {
        delete _shape;
    }
}

void extractMeshData(btBvhTriangleMeshShape* meshShape, std::vector<glm::vec3>& positions, std::vector<glm::uvec3>& triangleIndices) {
    // Get the internal triangle mesh
    btTriangleMesh* triangleMesh = dynamic_cast<btTriangleMesh*>(meshShape->getMeshInterface());

    const unsigned char* vertexBase;
    const unsigned char* indexBase;
    int numVerts;
    int vertexStride;
    PHY_ScalarType vertexType;
    int numFaces;
    int indexStride;
    PHY_ScalarType indexType;

    // Get mesh buffers
    triangleMesh->getLockedReadOnlyVertexIndexBase(
        &vertexBase, numVerts, vertexType, vertexStride,
        &indexBase, indexStride, numFaces, indexType);

    // Ensure the vertex and index types are as expected
    if (vertexType != PHY_FLOAT || indexType != PHY_INTEGER) {
        std::cerr << "Unsupported vertex or index type" << std::endl;
        return;
    }

    // Read vertices
    const btScalar* vertices = reinterpret_cast<const btScalar*>(vertexBase);
    for (int i = 0; i < numVerts; ++i) {
        btVector3 vertex(vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2]);
        positions.emplace_back(vertex.getX(), vertex.getY(), vertex.getZ());
    }

    // Read indices and create triangles
    const unsigned int* indices = reinterpret_cast<const unsigned int*>(indexBase);
    for (int i = 0; i < numFaces; ++i) {
        int index0 = indices[i * 3];
        int index1 = indices[i * 3 + 1];
        int index2 = indices[i * 3 + 2];

        // Create triangle indices
        triangleIndices.emplace_back(index0, index1, index2);
    }

    // Unlock the vertex and index base
    triangleMesh->unLockReadOnlyVertexBase(0);
}

void extractMeshData(btBvhTriangleMeshShape* meshShape, std::vector<glm::vec3>& positions, std::vector<glm::uvec2>& lineIndices) {
    // Get the internal triangle mesh
    const btStridingMeshInterface* meshInterface = meshShape->getMeshInterface();

    const unsigned char* vertexBase;
    const unsigned char* indexBase;
    int numVerts;
    int vertexStride;
    PHY_ScalarType vertexType;
    int numFaces;
    int indexStride;
    PHY_ScalarType indexType;

    // Get mesh buffers
    meshInterface->getLockedReadOnlyVertexIndexBase(
        &vertexBase, numVerts, vertexType, vertexStride,
        &indexBase, indexStride, numFaces, indexType);

    // Ensure the vertex and index types are as expected
    if (vertexType != PHY_FLOAT || indexType != PHY_INTEGER) {
        std::cerr << "Unsupported vertex or index type" << std::endl;
        return;
    }

    // Read vertices
    const btScalar* vertices = reinterpret_cast<const btScalar*>(vertexBase);
    for (int i = 0; i < numVerts; ++i) {
        btVector3 vertex(vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2]);
        positions.emplace_back(vertex.getX(), vertex.getY(), vertex.getZ());
    }

    // Read indices and create line indices
    const unsigned int* indices = reinterpret_cast<const unsigned int*>(indexBase);
    for (int i = 0; i < numFaces; ++i) {
        int index0 = indices[i * 3];
        int index1 = indices[i * 3 + 1];
        int index2 = indices[i * 3 + 2];

        // Create lines from triangle indices
        lineIndices.emplace_back(index0, index1);
        lineIndices.emplace_back(index1, index2);
        lineIndices.emplace_back(index2, index0);
    }

    // Unlock the vertex and index base
    meshInterface->unLockReadOnlyVertexBase(0);
}

struct uvec2Hash {
    std::size_t operator()(const glm::uvec2& vec) const {
        // Combine hashes of x and y components
        std::size_t h1 = std::hash<unsigned int>{}(vec.x);
        std::size_t h2 = std::hash<unsigned int>{}(vec.y);
        return h1 ^ (h2 << 1); // Example hash combination
    }
};

navmesh_ref::navmesh_ref(const mesh_data& data)
{
    // btTriangleMesh* _triangle_mesh = new btTriangleMesh();
    // for (const glm::uvec3& _index : data.indices) {
    //     btVector3 _vertex_0(data.positions[_index.x].x, data.positions[_index.x].y, data.positions[_index.x].z);
    //     btVector3 _vertex_1(data.positions[_index.y].x, data.positions[_index.y].y, data.positions[_index.y].z);
    //     btVector3 _vertex_2(data.positions[_index.z].x, data.positions[_index.z].y, data.positions[_index.z].z);

    //     _triangle_mesh->addTriangle(_vertex_0, _vertex_1, _vertex_2);
    // }
    // _shape = new btConvexHullShape(_triangle_mesh, true, false);
    btConvexHullShape* hullShape = new btConvexHullShape();

    // Add vertices to the convex hull shape
    for (const glm::vec3& vertex : data.positions) {
        hullShape->addPoint(btVector3(vertex.x, vertex.y, vertex.z));
    }

    // Recalculate the internal data structures of the convex hull
    hullShape->recalcLocalAabb();
    
    _shape = hullShape;

    // btConvexTriangleMeshShape
#if LUCARIA_GUIZMO
    // std::vector<glm::vec3> positions;
    // std::vector<glm::uvec2> lineIndices;
    // extractMeshData((btBvhTriangleMeshShape*)_shape, positions, lineIndices);
    std::vector<glm::vec3> _positions = {};  
    std::vector<glm::uvec2> _edges = {};  

    // Get number of vertices in the convex hull shape
    int numVertices = hullShape->getNumPoints();

    // Reserve space in positions vector
    _positions.reserve(numVertices);

    // Iterate over the vertices and store them in positions vector
    for (int i = 0; i < numVertices; ++i) {
        btVector3 vertex = hullShape->getUnscaledPoints()[i];
        _positions.emplace_back(vertex.x(), vertex.y(), vertex.z());
    }

    // Compute edges (lines) from the convex hull shape
    std::unordered_set<glm::uvec2, uvec2Hash> edgeSet;

    // Iterate over vertex pairs to find unique edges
    for (int i = 0; i < numVertices; ++i) {
        for (int j = i + 1; j < numVertices; ++j) {
            btVector3 vertex1 = hullShape->getUnscaledPoints()[i];
            btVector3 vertex2 = hullShape->getUnscaledPoints()[j];
            glm::uvec2 edgeIndices(i, j);
            glm::uvec2 reverseEdgeIndices(j, i);

            // Check if edge already exists (avoid duplicates)
            if (edgeSet.find(reverseEdgeIndices) == edgeSet.end()) {
                edgeSet.insert(edgeIndices);
            }
        }
    }

    // Convert edge set to vector for mesh_data representation
    _edges.assign(edgeSet.begin(), edgeSet.end());

    _guizmo = std::make_unique<guizmo_mesh_ref>(_positions, _edges);
#endif
    _is_instanced = true;
}

btCollisionShape* navmesh_ref::get_shape() const
{
    return _shape;
}

std::shared_future<std::shared_ptr<navmesh_ref>> fetch_navmesh(const std::filesystem::path& mesh_path)
{
    std::promise<std::shared_ptr<navmesh_ref>>& _promise = detail::promises[mesh_path.string()];
    fetch_file(mesh_path, [&_promise](std::istringstream& stream) {
        _promise.set_value(std::move(std::make_shared<navmesh_ref>(load_mesh_data(stream))));
    });
    return _promise.get_future();
}
