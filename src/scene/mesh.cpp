#include <scene/mesh.hpp>
#include "scene/objmodel.hpp"

struct TriIndex
{
	int vertex;
	int normal;
	int tcoord;
};

Mesh::Mesh()
{
	has_normals = false;
	has_tcoords = false;
}

Mesh::~Mesh() { }

bool Mesh::load(const ObjModel* model)
{
	for (unsigned int i = 0; i < model->num_vertices(); i++)
	{
		glm::vec3 vertex = model->get_vertices()[i];
		MeshVertex m_vertex;
		m_vertex.position = vertex;
		vertices.push_back(m_vertex);
	}

	for (unsigned int i = 0; i < model->num_groups(); i++)
	{
		ObjModel::TriangleGroup group = model->get_groups()[i];

		for (unsigned int j = 0; j < group.triangles.size(); j++)
		{
			ObjModel::Triangle triangle = group.triangles[j];
			MeshTriangle m_triangle;
			int a = triangle.vertices[0];
			int b = triangle.vertices[1];
			int c = triangle.vertices[2];

			if (triangle.vertexType == ObjModel::Triangle::POSITION_NORMAL || triangle.vertexType == ObjModel::Triangle::POSITION_TEXCOORD_NORMAL)
			{
				has_normals = true;
				vertices[a].normal = model->get_normals()[triangle.normals[0]];
				vertices[b].normal = model->get_normals()[triangle.normals[1]];
				vertices[c].normal = model->get_normals()[triangle.normals[2]];
			}
			if (triangle.vertexType == ObjModel::Triangle::POSITION_TEXCOORD || triangle.vertexType == ObjModel::Triangle::POSITION_TEXCOORD_NORMAL)
			{
				has_tcoords = true;
				vertices[a].tex_coord = model->get_texcoords()[triangle.texcoords[0]];
				vertices[b].tex_coord = model->get_texcoords()[triangle.texcoords[1]];
				vertices[c].tex_coord = model->get_texcoords()[triangle.texcoords[2]];
			}

			m_triangle.vertices[0] = a;
			m_triangle.vertices[1] = b;
			m_triangle.vertices[2] = c;
            
            ObjModel::ObjMtl material = model->get_materials()[triangle.materialID];
            diffuse = material.Kd;
            ambient = material.Ka;
            specular = material.Ks;
            
            
            if (material.map_Kd >= 0)
            {
                diffuseImg = &(model->get_textures()[material.map_Kd]);
            }
            if (material.map_Ka >= 0)
            {
                ambientImg = &(model->get_textures()[material.map_Ka]);
            }
            
			triangles.push_back(m_triangle);
		}
	}

	initialize();

	return true;

}

const MeshTriangle* Mesh::get_triangles() const
{
	return triangles.empty() ? nullptr : &triangles[0];
}

size_t Mesh::num_triangles() const
{
	return triangles.size();
}

const MeshVertex* Mesh::get_vertices() const
{
	return vertices.empty() ? nullptr : &vertices[0];
}

size_t Mesh::num_vertices() const
{
	return vertices.size();
}

bool Mesh::are_normals_valid() const
{
	return has_normals;
}

bool Mesh::are_tex_coords_valid() const
{
	return has_tcoords;
}

bool Mesh::initialize()
{
	compute_normals();
	return true;
}

void Mesh::compute_normals()
{
	return;
}

