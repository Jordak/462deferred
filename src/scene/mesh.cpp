#include <scene/mesh.hpp>
#include "scene/objmodel.hpp"
//#include <stdio.h>
#include <iostream>

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

glm::vec3 copy3(const glm::vec3 orig)
{
	glm::vec3 result;
	result.x = orig.x;
	result.y = orig.y;
	result.z = orig.z;
	return result;
}

glm::vec2 copy2(const glm::vec2 orig)
{
	glm::vec2 result;
	result.x = orig.x;
	result.y = orig.y;
	return result;
}

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

			int a;
			glm::vec2 at;
			glm::vec3 an;

			int b;
			glm::vec2 bt;
			glm::vec3 bn;

			int c;
			glm::vec2 ct;
			glm::vec3 cn;

			a = triangle.vertices[0];
			b = triangle.vertices[1];
			c = triangle.vertices[2];

			if (triangle.vertexType == ObjModel::Triangle::POSITION_NORMAL || triangle.vertexType == ObjModel::Triangle::POSITION_TEXCOORD_NORMAL)
			{
				has_normals = true;
				an = copy3(model->get_normals()[triangle.normals[0]]);
				bn = copy3(model->get_normals()[triangle.normals[1]]);
				cn = copy3(model->get_normals()[triangle.normals[2]]);
			}
			if (triangle.vertexType == ObjModel::Triangle::POSITION_TEXCOORD || triangle.vertexType == ObjModel::Triangle::POSITION_TEXCOORD_NORMAL)
			{
				has_tcoords = true;
				at = copy2(model->get_texcoords()[triangle.texcoords[0]]);
				bt = copy2(model->get_texcoords()[triangle.texcoords[1]]);
				ct = copy2(model->get_texcoords()[triangle.texcoords[2]]);
			}

                  int idxa = -1;
                  if ((!has_tcoords || glm::all(glm::equal(vertices[a].tex_coord, glm::vec2(0.0))) || glm::all(glm::equal(vertices[a].tex_coord, at)) ) && 
                      (!has_normals || glm::all(glm::equal(vertices[a].normal, glm::vec3(0.0))) || glm::all(glm::equal(vertices[a].normal, an)) ))
                  {
                  	idxa = a;
                  }
                  else
                  {
                  	for (unsigned int k = model->num_vertices(); k < vertices.size(); k++)
                  	{
                  		if (glm::all(glm::equal(vertices[k].position, vertices[a].position)) && 
                  		    (!has_tcoords || glm::all(glm::equal(vertices[k].tex_coord, at)) ) &&
                  		    (!has_normals || glm::all(glm::equal(vertices[k].normal, an)) ) )
                  		{
                  			idxa = k;
                  			break;
                  		}
                  	}
                  }
                  if (idxa == -1)
                  {
                  	MeshVertex mv;
                  	mv.position = vertices[a].position;
                  	idxa = vertices.size();
                  	vertices.push_back(mv);
                  }
                  if (has_normals)
                  {
                  	vertices[idxa].normal = an;
                  }
                  if (has_tcoords)
                  {
                  	vertices[idxa].tex_coord = at;
                  }
                  m_triangle.vertices[0] = idxa;
                  
                  int idxb = -1;
                  if ((!has_tcoords || glm::all(glm::equal(vertices[b].tex_coord, glm::vec2(0.0))) || glm::all(glm::equal(vertices[b].tex_coord, bt)) ) && 
                      (!has_normals || glm::all(glm::equal(vertices[b].normal, glm::vec3(0.0))) || glm::all(glm::equal(vertices[b].normal, bn)) ))
                  {
                  	idxb = b;
                  }
                  else
                  {
                  	for (unsigned int k = model->num_vertices(); k < vertices.size(); k++)
                  	{
                  		if (glm::all(glm::equal(vertices[k].position, vertices[b].position)) && 
                  		    (!has_tcoords || glm::all(glm::equal(vertices[k].tex_coord, bt)) ) &&
                  		    (!has_normals || glm::all(glm::equal(vertices[k].normal, bn)) ) )
                  		{
                  			idxb = k;
                  			break;
                  		}
                  	}
                  }
                  if (idxb == -1)
                  {
                  	MeshVertex mv;
                  	mv.position = vertices[b].position;
                  	idxb = vertices.size();
                  	vertices.push_back(mv);
                  }
                  if (has_normals)
                  {
                  	vertices[idxb].normal = bn;
                  }
                  if (has_tcoords)
                  {
                  	vertices[idxb].tex_coord = bt;
                  }
                  m_triangle.vertices[1] = idxb;
                  
                  int idxc = -1;
                  if ((!has_tcoords || glm::all(glm::equal(vertices[c].tex_coord, glm::vec2(0.0))) || glm::all(glm::equal(vertices[c].tex_coord, ct)) ) && 
                      (!has_normals || glm::all(glm::equal(vertices[c].normal, glm::vec3(0.0))) || glm::all(glm::equal(vertices[c].normal, cn)) ))
                  {
                  	idxc = c;
                  }
                  else
                  {
                  	for (unsigned int k = model->num_vertices(); k < vertices.size(); k++)
                  	{
                  		if (glm::all(glm::equal(vertices[k].position, vertices[c].position)) && 
                  		    (!has_tcoords || glm::all(glm::equal(vertices[k].tex_coord, ct)) ) &&
                  		    (!has_normals || glm::all(glm::equal(vertices[k].normal, cn)) ) )
                  		{
                  			idxc = k;
                  			break;
                  		}
                  	}
                  }
                  if (idxc == -1)
                  {
                  	MeshVertex mv;
                  	mv.position = vertices[c].position;
                  	idxc = vertices.size();
                  	vertices.push_back(mv);
                  }
                  if (has_normals)
                  {
                  	vertices[idxc].normal = cn;
                  }
                  if (has_tcoords)
                  {
                  	vertices[idxc].tex_coord = ct;
                  }
                  m_triangle.vertices[2] = idxc;
                  
                  if (triangle.materialID >= 0)
                  {
                        ObjModel::ObjMtl material = model->get_materials()[triangle.materialID];
                        diffuse = material.Kd;
                        ambient = material.Ka;
                        specular = material.Ks;
                        if (material.map_Kd >= 0)
                        {
                              diffuseImg = &(model->get_textures()[material.map_Kd-1]);
                        }
                        if (material.map_Ka >= 0)
                        {
                            ambientImg = &(model->get_textures()[material.map_Ka-1]);
                        }
                  }
                  else
                  {
                        diffuse = glm::vec3(1.0);
                        ambient = glm::vec3(1.0);
                        specular = glm::vec3(1.0);

                        sf::Image* diffuseImage = new sf::Image();
                        diffuseImage->create(1,1,sf::Color::Blue);
                        diffuseImg = diffuseImage;

                        sf::Image* ambientImage = new sf::Image();
                        ambientImage->create(1,1,sf::Color::Blue);
                        ambientImg = ambientImage;
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

