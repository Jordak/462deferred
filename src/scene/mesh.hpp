#include <glm/glm.hpp>
#include <scene/objmodel.hpp>

struct MeshVertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coord;
};

struct MeshTriangle
{
	unsigned int vertices[3];
};

class Mesh
{
public:
	Mesh();
	~Mesh();

	bool load(const ObjModel* model);

	const MeshTriangle* get_triangles() const;
	const MeshVertex* get_vertices() const;
	size_t num_triangles() const;
	size_t num_vertices() const;

	bool are_normals_valid() const;
	bool are_tex_coords_valid() const;


	bool initialize();
	void compute_normals();

private:
	typedef std::vector< MeshTriangle > MeshTriangleList;
	typedef std::vector< MeshVertex > MeshVertexList;

	MeshTriangleList triangles;
	MeshVertexList vertices;

	bool has_normals;
	bool has_tcoords;
    
    //glm::vec3 Ka;
    glm::vec3 Kd;
    //glm::vec3 Ks;
    
    //sf::Image* diffuseImage;
    //sf::Image* ambientImage;
};
