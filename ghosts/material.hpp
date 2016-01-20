#pragma  once

#include "resource.hpp"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <gli/texture2d.hpp>

namespace graphics
{
	// physical based render lighting model
	class material : public resource<material>
	{

	public:

		enum class uniform : uint32_t
		{
			TRANSFORM,
			LIGHT,
			MAX
		};

		enum class texture : uint32_t
		{
			DIFFUSE,
			SPECULAR,
			NORMAL,
			ENVIRONMENT,
			MAX
		};

	private:

		uint32_t m_PipelineName;
		uint32_t m_ProgramName;

		uint32_t m_UniformBufferNames[uniform::MAX];

		uint32_t m_TextureNames[texture::MAX];
		uint32_t m_SamplerNames[texture::MAX];

	public:

		material(const std::string texture_files[texture::MAX]);

		bool create();
		void use();
		void destory();

		void update(
			glm::mat4 prj_matrix,			// projection matrix
			glm::mat4 mv_matrix,			// model * view matrix
			glm::vec4 light_dir_intensity);	// light direction vec4.xyz, intensity vec4.w

	};
}