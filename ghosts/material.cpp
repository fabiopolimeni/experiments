#include "material.hpp"
#include "ogl.hpp"
#include "util.hpp"
#include "compiler.hpp"

#include <glm/vec3.hpp>
#include <gli/gli.hpp>
#include <array>

namespace
{
	uint32_t build(uint32_t program_name, compiler& in_compiler, const char* vs_source, const char* fs_source)
	{
		assert(glIsProgram(program_name));

		gl::uint32 vert_shader_name = in_compiler.create(GL_VERTEX_SHADER, vs_source);
		if (!in_compiler.checkShader(vert_shader_name))
			return 0;

		gl::uint32 frag_shader_name = in_compiler.create(GL_FRAGMENT_SHADER, fs_source);
		if (!in_compiler.checkShader(frag_shader_name))
			return 0;

		glProgramParameteri(program_name, GL_PROGRAM_SEPARABLE, GL_TRUE);
		glAttachShader(program_name, vert_shader_name);
		glAttachShader(program_name, frag_shader_name);
		glLinkProgram(program_name);

		return uint32_t(GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT);
	}
}

char const * VS_SOURCE = "data/shaders/pbr.vert";
char const * FS_SOURCE = "data/shaders/pbr.frag";

graphics::material::material()
	: m_PipelineName(0), m_ProgramName(0)
{
	// clear texture unit names
	memset(m_TextureNames, 0, sizeof(m_TextureNames));
	memset(m_SamplerNames, 0, sizeof(m_SamplerNames));
	memset(m_UniformBufferNames, 0, sizeof(m_UniformBufferNames));
}

bool graphics::material::create()
{
	assert(m_ProgramName == 0);
	assert(m_PipelineName == 0);

	// generate and populate samplers
	glGenSamplers(enum_to_t(sampler::MAX), &m_SamplerNames[0]);

	for (std::size_t i = 0; i < enum_to_t(sampler::MAX); ++i)
	{
		glSamplerParameteri(m_SamplerNames[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glSamplerParameteri(m_SamplerNames[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glSamplerParameteri(m_SamplerNames[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(m_SamplerNames[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// we want seamless cubemaps
		if (i == enum_to_t(sampler::ENVIRONMENT))
			glSamplerParameteri(m_SamplerNames[i], GL_TEXTURE_CUBE_MAP_SEAMLESS, GL_TRUE);
	}

	gl::int32 uniform_buffer_offeset(0);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniform_buffer_offeset);

	// create the uniform transform buffer
	{
		auto uniform_buffer_size = glm::max(gl::int32((sizeof(glm::mat4) * 3)), uniform_buffer_offeset);

		glGenBuffers(1, &m_UniformBufferNames[enum_to_t(uniform::TRANSFORM)]);
		glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferNames[enum_to_t(uniform::TRANSFORM)]);
		glBufferData(GL_UNIFORM_BUFFER, uniform_buffer_size, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	// create the uniform light buffer
	{
		auto uniform_buffer_size = glm::max(gl::int32(sizeof(glm::vec4)), uniform_buffer_offeset);

		glGenBuffers(1, &m_UniformBufferNames[enum_to_t(uniform::LIGHT)]);
		glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferNames[enum_to_t(uniform::LIGHT)]);
		glBufferData(GL_UNIFORM_BUFFER, uniform_buffer_size, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	m_ProgramName = glCreateProgram();

	compiler compiler_instance;
	auto pipe_mask = build(m_ProgramName, compiler_instance, VS_SOURCE, FS_SOURCE);
	if (pipe_mask != 0 && compiler_instance.checkProgram(m_ProgramName))
	{
		glGenProgramPipelines(1, &m_PipelineName);
		glUseProgramStages(m_PipelineName, pipe_mask, m_ProgramName);
		return true;
	}

	return false;
}

void graphics::material::associate(texture * textures[sampler::MAX])
{
	// associate textures to material
	for (uint32_t ti = 0; ti < enum_to_t(sampler::MAX); ++ti)
		if (textures[ti]) m_TextureNames[ti] = textures[ti]->getHandle();
}

void graphics::material::use()
{
	glBindProgramPipeline(m_PipelineName);

	// bind texture units
	glBindTextures(0, enum_to_t(sampler::MAX), m_TextureNames);
	glBindSamplers(0, enum_to_t(sampler::MAX), m_SamplerNames);
	
	// bind uniform buffers
	glBindBufferBase(GL_UNIFORM_BUFFER, enum_to_t(uniform::TRANSFORM), m_UniformBufferNames[enum_to_t(uniform::TRANSFORM)]);
	glBindBufferBase(GL_UNIFORM_BUFFER, enum_to_t(uniform::LIGHT), m_UniformBufferNames[enum_to_t(uniform::LIGHT)]);
}

void graphics::material::destroy()
{	
	glDeleteBuffers(enum_to_t(uniform::MAX), m_UniformBufferNames);
	glDeleteProgramPipelines(1, &m_PipelineName);
	glDeleteProgram(m_ProgramName);

	m_PipelineName = 0;
	m_ProgramName = 0;

	memset(m_UniformBufferNames, 0, sizeof(m_UniformBufferNames));
	memset(m_TextureNames, 0, sizeof(m_TextureNames));
	memset(m_SamplerNames, 0, sizeof(m_SamplerNames));
}

void graphics::material::update(glm::mat4 prj_matrix, glm::mat4 mv_matrix, glm::vec4 light_dir_intensity)
{
	// update the transform buffer structure
	{
		// uniform buffer object needs to be bound with std140 layout attribute
		glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferNames[enum_to_t(uniform::TRANSFORM)]);
		uint8_t* tranform_buffer_ptr = (uint8_t*)glMapBufferRange(GL_UNIFORM_BUFFER, 0,
			sizeof(glm::mat4) * 3, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		auto mvp_matrix = prj_matrix * mv_matrix;
		auto norm_matrix = glm::transpose(glm::inverse(mv_matrix));

		// copy transform matrices
		memcpy(tranform_buffer_ptr, &mvp_matrix[0], sizeof(glm::mat4));
		memcpy(tranform_buffer_ptr + sizeof(glm::mat4), &mv_matrix[0], sizeof(glm::mat4));
		memcpy(tranform_buffer_ptr + sizeof(glm::mat4) * 2, &norm_matrix[0], sizeof(glm::mat4));

		// make sure the uniform buffer is uploaded
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}

	// update light info
	{
		glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferNames[enum_to_t(uniform::LIGHT)]);
		glm::vec4* light_buffer_ptr = (glm::vec4*)glMapBufferRange(
			GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		*light_buffer_ptr = light_dir_intensity;

		// make sure the uniform buffer is uploaded
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}
}
