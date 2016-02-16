#pragma  once

#include "resource.hpp"
#include "texture.hpp"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

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

		enum class sampler : uint32_t
		{
			DIFFUSE,
			SPECULAR,
			NORMAL,
			ROUGHNESS,
			DISPLACEMENT,
			ENVIRONMENT,
			MAX
		};

	private:

		handle m_PipelineName;
		handle m_ProgramName;

		handle m_UniformBufferNames[uniform::MAX];

		handle m_TextureRefs[sampler::MAX];
		handle m_SamplerNames[sampler::MAX];

	public:

		material();

		bool create();
		void associate(texture* textures[sampler::MAX]);
		void use();
		void destroy();

		void update(
			glm::mat4 prj_matrix,			// projection matrix
			glm::mat4 mv_matrix,			// model * view matrix
			glm::vec4 light_dir_intensity);	// light direction vec4.xyz, intensity vec4.w

	};
}