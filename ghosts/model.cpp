#include "model.hpp"
#include "logging.hpp"
#include "format.hpp"
#include "mesh.hpp"
#include "material.hpp"
#include "util.hpp"
#include "texture.hpp"

#include "tiny_obj_loader.h"

#include <algorithm>
#include <map>

namespace framework
{
	const char* s_default_texture_files[enum_to_t(graphics::material::sampler::MAX)] =
	{
		"data/textures/defaults/diffuse.dds",		// DIFFUSE
		"data/textures/defaults/metalness.dds",		// SPECULAR
		"data/textures/defaults/normal.dds",		// NORMAL
		"data/textures/defaults/roughness.dds",		// ROUGHNESS
		"data/textures/defaults/displacement.dds"	// DISPLACEMENT
	};

	static std::map<std::string, graphics::texture*> s_texture_names;
	graphics::texture* generateTexture(const std::string& tex_filename)
	{
		auto texture_pair = s_texture_names.find(tex_filename);
		if (texture_pair == s_texture_names.end())
		{
			graphics::texture* new_texture = new graphics::texture();
			s_texture_names.emplace(tex_filename, new_texture);
			return new_texture;
		}

		return texture_pair->second;
	}

	void computeTangents(
		const std::vector<uint32_t>& in_triangles,
		const std::vector<glm::vec4>& in_positions,
		const std::vector<glm::vec4>& in_normals,
		const std::vector<glm::vec2>& in_uvs,
		std::vector<glm::vec4>& out_tangents,
		std::vector<glm::vec4>& out_bitangents)
	{
		// initialise tangents and bitangents to zero vectors
		out_tangents.resize(in_positions.size());
		std::fill(out_tangents.begin(), out_tangents.end(), glm::vec4::ZERO);

		out_bitangents.resize(in_positions.size());
		std::fill(out_bitangents.begin(), out_bitangents.end(), glm::vec4::ZERO);

		// iterate through the triangles
		for (int t = 0; t < in_triangles.size(); t += 3)
		{
			const uint32_t i0 = in_triangles[t + 0];
			const uint32_t i1 = in_triangles[t + 1];
			const uint32_t i2 = in_triangles[t + 2];

			// shortcuts for vertices
			const glm::vec4 & v0 = in_positions[i0];
			const glm::vec4 & v1 = in_positions[i1];
			const glm::vec4 & v2 = in_positions[i2];

			// shortcuts for UVs
			const glm::vec2 & uv0 = in_uvs[i0];
			const glm::vec2 & uv1 = in_uvs[i1];
			const glm::vec2 & uv2 = in_uvs[i2];

			// edges of the triangle : position delta
			glm::vec4 deltaPos1 = v1 - v0;
			glm::vec4 deltaPos2 = v2 - v0;

			// uv delta
			glm::vec2 deltaUV1 = uv1 - uv0;
			glm::vec2 deltaUV2 = uv2 - uv0;

			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

			// accumulate tangent for all three vertices of the triangle.
			glm::vec4 new_tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;

			out_tangents[i0] = out_tangents[i0] + new_tangent;
			out_tangents[i1] = out_tangents[i1] + new_tangent;
			out_tangents[i2] = out_tangents[i2] + new_tangent;

			// same thing for bitangent
			glm::vec4 new_bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

			out_bitangents[i0] = out_bitangents[i0] + new_bitangent;
			out_bitangents[i1] = out_bitangents[i1] + new_bitangent;
			out_bitangents[i2] = out_bitangents[i2] + new_bitangent;
		}

		// normalize tangents and bitangent
		for (int i = 0; i < in_normals.size(); ++i)
		{
			const auto& normal = in_normals[i];

			// at this point tangents are not necessarily orthogonal 
			// to normals and do not create a space basis, we fix it
			// with help of Gram-Schmidt process.
			auto& tangent = out_tangents[i];
			tangent = glm::normalize(tangent);
			tangent = tangent - normal * glm::dot(tangent, normal);

			// apply R^3 stage Gram-Schmidt process.
			// would it be the same just to cross(tangent,normal)?
			auto& bitangent = out_bitangents[i];
			bitangent = glm::normalize(bitangent);
			bitangent = bitangent - normal * glm::dot(bitangent, normal) - tangent * glm::dot(bitangent, tangent);

			// calculate the handedness equal to sign (1 or -1) of determinant of world to tangent matrix
			glm::mat3 tbn = glm::transpose(glm::mat3(tangent.xyz(), bitangent.xyz(), normal.xyz()));
			const float handedness = glm::sign(glm::determinant(tbn));

			// write the handedness into element w of the tangent, to reconstruct
			// the bitangent from tangent and normal B = (N x T) * H
			tangent.w = handedness;
		}
	}

	model* model::loadObj(const std::string& in_file)
	{
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		LOG(INFO) << "Loading file: " << in_file;

		const auto file_basepath = path::left(in_file, '/');

		std::string error_string;
		bool valid_obj = tinyobj::LoadObj(shapes, materials, error_string, in_file.c_str(), file_basepath.c_str());
		if (!error_string.empty()) // `err` may contain warning message.
		{
			if (valid_obj) LOG(WARNING) << error_string;
			else LOG(ERROR) << error_string;
		}

		if (!valid_obj) {
			return nullptr;
		}

		model* loaded_model = new model();

		LOG(INFO) << "# of shapes    : " << shapes.size();
		LOG(INFO) << "# of materials : " << materials.size();

		for (size_t i = 0; i < shapes.size(); ++i)
		{
			graphics::mesh* mesh = new graphics::mesh();

			LOG(INFO) << fmt::format("shape[{}].name = {}", i, shapes[i].name.c_str());
			LOG(INFO) << fmt::format("Size of shape[{}].indices: {}", i, shapes[i].mesh.indices.size() / 3);
			LOG(INFO) << fmt::format("Size of shape[{}].material_ids: {}", i, shapes[i].mesh.material_ids.size());
			assert((shapes[i].mesh.indices.size() % 3) == 0);

			// copy triangles topology
			mesh->p_FaceIndices = shapes[i].mesh.indices;
			
			//for (size_t f = 0; f < shapes[i].mesh.indices.size() / 3; f++)
			//{
			//	LOG(INFO) << fmt::format("  idx[{}] = {}, {}, {}. mat_id = {}", f,
			//		shapes[i].mesh.indices[3 * f + 0],
			//		shapes[i].mesh.indices[3 * f + 1],
			//		shapes[i].mesh.indices[3 * f + 2],
			//		shapes[i].mesh.material_ids[f]);
			//}

			assert((shapes[i].mesh.positions.size() % 3) == 0);
			const size_t n_vertices = shapes[i].mesh.positions.size() / 3;
			assert(shapes[i].mesh.normals.size() == n_vertices * 3);
			assert(shapes[i].mesh.texcoords.size() == n_vertices * 2);

			LOG(INFO) << fmt::format("shape[{}].vertices: {}", i, n_vertices);

			// transform per mesh is no supported yet
			mesh->p_MeshToModel.p_Position = glm::vec4::ZERO;
			mesh->p_MeshToModel.p_Rotation = glm::quat::IDENTITY;

			// copy geometry data
			mesh->p_PosRadius.reserve(n_vertices);
			mesh->p_TexCoords.reserve(n_vertices);
			mesh->p_Normals.reserve(n_vertices);
			mesh->p_Tangents.reserve(n_vertices);
			mesh->p_VelInvMass.reserve(n_vertices);

			for (size_t v = 0; v < n_vertices; ++v)
			{
				mesh->p_PosRadius.emplace_back(
					glm::vec4(
						shapes[i].mesh.positions[3 * v + 0],
						shapes[i].mesh.positions[3 * v + 1],
						shapes[i].mesh.positions[3 * v + 2],
						0.f));

				mesh->p_Normals.emplace_back(
					glm::vec4(
						shapes[i].mesh.normals[3 * v + 0],
						shapes[i].mesh.normals[3 * v + 1],
						shapes[i].mesh.normals[3 * v + 2],
						0.f));

				mesh->p_TexCoords.emplace_back(
					glm::vec2(
						shapes[i].mesh.texcoords[2 * v + 0],
						shapes[i].mesh.texcoords[2 * v + 1]));

				mesh->p_VelInvMass.emplace_back(glm::vec4::ZERO);
			}

			// compute tangents
			std::vector<glm::vec4> bitangents;
			computeTangents(mesh->p_FaceIndices, mesh->p_PosRadius, mesh->p_Normals, mesh->p_TexCoords, mesh->p_Tangents, bitangents);

			// add the mesh to the model
			loaded_model->m_Meshes.push_back(mesh);

			// map material to mesh
			// NOTE: multiple materials per mesh is not supported yet
			loaded_model->m_MeshMaterialMap.push_back(shapes[i].mesh.material_ids[0]);
		}

		for (size_t i = 0; i < materials.size(); i++)
		{
			//LOG(INFO) << fmt::format("material[{}].name ={}", i, materials[i].name.c_str());
			//LOG(INFO) << fmt::format("  material.Ka = ({}, {} ,{})", materials[i].ambient[0], materials[i].ambient[1], materials[i].ambient[2]);
			//LOG(INFO) << fmt::format("  material.Kd = ({}, {} ,{})", materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
			//LOG(INFO) << fmt::format("  material.Ks = ({}, {} ,{})", materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);
			//LOG(INFO) << fmt::format("  material.Tr = ({}, {} ,{})", materials[i].transmittance[0], materials[i].transmittance[1], materials[i].transmittance[2]);
			//LOG(INFO) << fmt::format("  material.Ke = ({}, {} ,{})", materials[i].emission[0], materials[i].emission[1], materials[i].emission[2]);
			//LOG(INFO) << fmt::format("  material.Ns = {}", materials[i].shininess);
			//LOG(INFO) << fmt::format("  material.Ni = {}", materials[i].ior);
			//LOG(INFO) << fmt::format("  material.dissolve = {}", materials[i].dissolve);
			//LOG(INFO) << fmt::format("  material.illum = {}", materials[i].illum);
			//LOG(INFO) << fmt::format("  material.map_Ka = {}", materials[i].ambient_texname.c_str());
			//LOG(INFO) << fmt::format("  material.map_Kd = {}", materials[i].diffuse_texname.c_str());
			//LOG(INFO) << fmt::format("  material.map_Ks = {}", materials[i].specular_texname.c_str());
			//LOG(INFO) << fmt::format("  material.map_Ns = {}", materials[i].specular_highlight_texname.c_str());
			
			std::map<std::string, std::string>::const_iterator it(materials[i].unknown_parameter.begin());
			std::map<std::string, std::string>::const_iterator itEnd(materials[i].unknown_parameter.end());
			for (; it != itEnd; it++) {
				LOG(INFO) << fmt::format("  material.{} = {}", it->first.c_str(), it->second.c_str());
			}

			// create the texture set for this material
			texture_set_t texture_set;
			
			// albedo
			{
				const auto tex_filename = path::right(materials[i].diffuse_texname, '/');
				auto texture_type = enum_to_t(graphics::material::sampler::DIFFUSE);
				texture_set[texture_type] = (!tex_filename.empty())
					? generateTexture(file_basepath + tex_filename)
					: generateTexture(s_default_texture_files[texture_type]);
			}
			
			// metalness
			{
				const auto tex_filename = path::right(materials[i].specular_texname, '/');
				auto texture_type = enum_to_t(graphics::material::sampler::SPECULAR);
				texture_set[texture_type] = (!tex_filename.empty())
					? generateTexture(file_basepath + tex_filename)
					: generateTexture(s_default_texture_files[texture_type]);
			}

			// roughness
			{
				const auto tex_filename = path::right(materials[i].specular_highlight_texname, '/');
				auto texture_type = enum_to_t(graphics::material::sampler::ROUGHNESS);
				texture_set[texture_type] = (!tex_filename.empty())
					? generateTexture(file_basepath + tex_filename)
					: generateTexture(s_default_texture_files[texture_type]);
			}

			// displacement
			{
				const auto tex_filename = path::right(materials[i].displacement_texname, '/');
				auto texture_type = enum_to_t(graphics::material::sampler::DISPLACEMENT);
				texture_set[texture_type] = (!tex_filename.empty())
					? generateTexture(file_basepath + tex_filename)
					: generateTexture(s_default_texture_files[texture_type]);
			}

			// normal
			{
				const auto tex_filename = path::right(materials[i].bump_texname, '/');
				auto texture_type = enum_to_t(graphics::material::sampler::NORMAL);
				texture_set[texture_type] = (!tex_filename.empty())
					? generateTexture(file_basepath + tex_filename)
					: generateTexture(s_default_texture_files[texture_type]);
			}

			// create graphics material
			graphics::material* material = new graphics::material();
			loaded_model->m_Materials.push_back(material);
			loaded_model->m_MaterialTexturesSet.push_back(texture_set);
		}

		return loaded_model;
	}

	void model::clear()
	{
		for (auto mesh : m_Meshes)
			delete mesh;

		for (auto material : m_Materials)
			delete material;

		for (auto texture : m_Textures)
			delete texture;

		m_Materials.clear();
		m_Meshes.clear();
		m_MeshMaterialMap.clear();
	}

	model* model::load(const std::string & filename, file_type f_type)
	{
		switch (f_type)
		{
		case ASCII:
			return loadObj(filename);
			break;

		default:
			LOG(ERROR) << fmt::format("Model format {}, for file {} not supported!", f_type, filename);
			return nullptr;
		}
	}

	void model::release(model * in_model)
	{
		if (in_model)
		{
			for (auto mesh : in_model->m_Meshes)
				mesh->destroy();

			for (auto material : in_model->m_Materials)
				material->destroy();

			for (auto texture : in_model->m_Textures)
				texture->destroy();

			in_model->clear();
		}

		delete in_model;
	}

	bool model::initialise()
	{
		bool valid_model = true;
		
		for (auto mesh : m_Meshes)
			valid_model &= mesh->create();

		for (auto texture_set : m_MaterialTexturesSet) {
			for (auto texture : texture_set) {
				for (auto tex_file : s_texture_names) {
					if (texture && tex_file.second == texture && texture->getHandle() == graphics::texture::invalid) {
						valid_model &= texture->create(tex_file.first);
						break;
					}
				}
			}
		}

		for (auto material : m_Materials)
			valid_model &= material->create();

		return valid_model;
	}

	void model::update(glm::vec4 position, glm::quat rotation)
	{
		m_ModelToWorld.p_Position = position;
		m_ModelToWorld.p_Rotation = rotation;
	}

	void model::simulate(float delta_time)
	{
	}

	void model::render(glm::mat4 projection, glm::mat4 view_mat, glm::vec4 light)
	{
		glm::mat4 model_mat = glm::translate(glm::mat4::IDENTITY, m_ModelToWorld.p_Position.xyz());
		model_mat *= glm::mat4_cast(m_ModelToWorld.p_Rotation);

		auto model_view = view_mat * model_mat;

		// calculate light direction in view space
		auto light_intensity = light.w;
		auto light_view = view_mat * glm::vec4(light.xyz(), 0.f);

		for (size_t m_id = 0; m_id < m_Meshes.size(); ++m_id)
		{
			// set the material
			assert(m_id < m_MeshMaterialMap.size());
			auto material = m_Materials[m_MeshMaterialMap[m_id]];

			material->associate(m_MaterialTexturesSet[m_id].data());
			material->update(projection, model_view, glm::vec4(glm::vec3(light_view), light_intensity));
			material->use();

			// draw the mesh
			m_Meshes[m_id]->use();
		}
	}
}
