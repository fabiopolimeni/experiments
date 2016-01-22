#include "model.hpp"
#include "logging.hpp"
#include "format.hpp"
#include "mesh.hpp"
#include "material.hpp"
#include "util.hpp"
#include "texture.hpp"

#include "tiny_obj_loader.h"

#include <map>

namespace framework
{
	graphics::texture* generateTexture(const std::string& tex_filename)
	{
		static	std::map<std::string, graphics::texture*> texture_names;

		graphics::texture* new_texture = nullptr;
		if (texture_names.find(tex_filename) == texture_names.end())
		{
			new_texture = new graphics::texture();
			if (!new_texture->create(tex_filename))
			{
				new_texture->destory();
				delete new_texture;
				new_texture = nullptr;
			}

			texture_names.emplace(tex_filename, new_texture);
		}

		return new_texture;
	}

	model* model::loadObj(const std::string& in_file)
	{
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		LOG(INFO) << "Loading file: " << in_file;

		std::string file_basepath;
		auto dir_pos = in_file.find_last_of('/');
		if (dir_pos != std::string::npos)
		{
			file_basepath = in_file.substr(0, dir_pos+1);
		}

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

			// add the mesh to the model
			loaded_model->m_Meshes.push_back(mesh);

			// map material to mesh
			// NOTE: multiple material per mesh is not supported yet
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

			// create graphics material
			graphics::texture* texture_files[graphics::material::sampler::MAX] = { nullptr };
			texture_files[enum_to_t(graphics::material::sampler::DIFFUSE)] = generateTexture(file_basepath + materials[i].diffuse_texname);
			texture_files[enum_to_t(graphics::material::sampler::SPECULAR)] = generateTexture(file_basepath + materials[i].specular_texname);

			graphics::material* material = new graphics::material(texture_files);
			loaded_model->m_Materials.push_back(material);
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

		m_Textures.clear();
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

	void model::destroy(model * in_model)
	{
		if (in_model)
		{
			for (auto mesh : in_model->m_Meshes)
				mesh->destroy();

			for (auto material : in_model->m_Materials)
				material->destory();

			for (auto texture : in_model->m_Textures)
				texture->destory();

			in_model->clear();
		}

		delete in_model;
	}

	bool model::activate()
	{
		bool valid_model = true;
		
		for (auto mesh : m_Meshes)
			valid_model &= mesh->create();

		for (auto material : m_Materials)
			valid_model &= material->create();

		return valid_model;
	}

	bool model::isActive() const
	{
		return false;
	}

	void model::update(glm::vec4 position, glm::quat rotation)
	{
		m_ModelToWorld.p_Position = position;
		m_ModelToWorld.p_Rotation = rotation;
	}

	void model::simulate(float delta_time)
	{
	}

	void model::render(glm::mat4 projection, glm::mat4 view, glm::vec4 light)
	{
		glm::mat4 model_mat = glm::translate(glm::mat4::IDENTITY, m_ModelToWorld.p_Position.xyz());
		model_mat *= glm::mat4_cast(m_ModelToWorld.p_Rotation);

		auto model_view = view * model_mat;

		// calculate light direction in view space
		auto light_intensity = light.w;
		auto light_view = view * glm::vec4(light.xyz(), 0.f);

		for (size_t m_id = 0; m_id < m_Meshes.size(); ++m_id)
		{
			assert(m_id < m_MeshMaterialMap.size());
			auto material = m_Materials[m_MeshMaterialMap[m_id]];
			material->update(projection, model_view, glm::vec4(glm::vec3(light_view), light_intensity));
			material->use();

			m_Meshes[m_id]->use();
		}
	}
}
