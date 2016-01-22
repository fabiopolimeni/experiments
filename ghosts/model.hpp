#pragma once

#include "transform.hpp"
#include <string>
#include <vector>

namespace graphics
{
	class material;
	class mesh;
	class texture;
}

namespace framework
{
	class model
	{
		math::transform m_ModelToWorld;
		
		std::vector<graphics::mesh*> m_Meshes;
		std::vector<graphics::material*> m_Materials;
		std::vector<graphics::texture*> m_Textures;

		// position in the vector refers to the mesh index inside m_Meshes,
		// while the element refers to the material index inside m_Materials.
		std::vector<size_t> m_MeshMaterialMap;	

		static model* loadObj(const std::string& in_file);

		void clear();

	public:

		enum file_type
		{
			ASCII,
			BINARY,
			MAX
		};

		static model* load(const std::string& filename, file_type f_type);
		static void destroy(model* in_model);

		bool activate();
		bool isActive() const;

		void update(glm::vec4 position, glm::quat rotation);
		void simulate(float delta_time);
		void render(glm::mat4 projection, glm::mat4 view, glm::vec4 light);
	};
}
