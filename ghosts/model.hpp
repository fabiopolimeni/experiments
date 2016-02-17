#pragma once

#include "transform.hpp"
#include "material.hpp"
#include "util.hpp"

#include <string>
#include <vector>
#include <array>

namespace graphics
{
	class material;
	class mesh;
	class texture;
	class line_batcher;
}

namespace framework
{
	class model
	{

	public:

		enum class render_mode : uint32_t
		{
			SOLID,
			TEXTURE,
			NORMAL,
			SHADED,
			WIREFRAME,
			DEBUG,
			MAX
		};

	private:

		math::transform m_ModelToWorld;
		
		std::vector<graphics::mesh*> m_Meshes;
		std::vector<graphics::material*> m_Materials;
		graphics::line_batcher* m_LineBatcher;

		// associate each material to a set of textures
		// the position in the vector refers to the material,
		// while the element (texture array) is the set of textures.
		typedef std::array<graphics::texture*, enum_to_t(graphics::material::sampler::MAX)> texture_set_t;
		std::vector<texture_set_t> m_MaterialTexturesSet;

		// position in the vector refers to the mesh index inside m_Meshes,
		// while the element refers to the material index inside m_Materials.
		std::vector<size_t> m_MeshMaterialMap;

		// set of rendering modes
		uint32_t m_RenderModeStates;

		static model* loadObj(const std::string& in_file);

		void clear();

	public:

		enum class file_type : uint8_t
		{
			ASCII,
			BINARY,
			MAX
		};

		static model* load(const std::string& filename, file_type f_type);
		static void release(model* in_model);

		bool initialise();
		void update(glm::vec4 position, glm::quat rotation);
		void simulate(float delta_time);
		void render(glm::mat4 projection, glm::mat4 view, glm::vec4 light);

		void setRenderMode(render_mode in_rm, bool in_enable);

		inline bool isRenderModeEnabled(render_mode in_rm) const {
			return (m_RenderModeStates & (1 << enum_to_t(in_rm))) != 0;
		}

		inline void toggleRenderMode(render_mode in_rm) {
			setRenderMode(in_rm, !isRenderModeEnabled(in_rm));
		}
	};
}
