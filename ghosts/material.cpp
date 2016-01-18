#include "material.hpp"
#include "ogl.hpp"
#include "util.hpp"
#include "compiler.hpp"

#include <glm/vec3.hpp>
#include <gli/gli.hpp>
#include <array>

namespace glu
{
	gl::uint32 createTexture(char const* Filename)
	{
		gli::texture Texture(gli::load(Filename));
		if (Texture.empty())
			return 0;

		gli::gl GL;
		gli::gl::format const Format = GL.translate(Texture.format());
		gli::gl::swizzles const Swizzles = GL.translate(Texture.swizzles());
		gl::enumerator Target = GL.translate(Texture.target());

		gl::uint32 TextureName = 0;
		glGenTextures(1, &TextureName);
		glBindTexture(Target, TextureName);
		glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<gl::int32>(Texture.levels() - 1));
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_R, Swizzles[0]);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_G, Swizzles[1]);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_B, Swizzles[2]);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_A, Swizzles[3]);

		glm::tvec3<gl::sizei> const Dimensions(Texture.dimensions());

		switch (Texture.target())
		{
		case gli::TARGET_1D:
			glTexStorage1D(
				Target, static_cast<gl::int32>(Texture.levels()), Format.Internal, Dimensions.x);
			break;
		case gli::TARGET_1D_ARRAY:
		case gli::TARGET_2D:
		case gli::TARGET_CUBE:
			glTexStorage2D(
				Target, static_cast<gl::int32>(Texture.levels()), Format.Internal,
				Dimensions.x, Texture.target() == gli::TARGET_2D ? Dimensions.y : static_cast<gl::sizei>(Texture.layers() * Texture.faces()));
			break;
		case gli::TARGET_2D_ARRAY:
		case gli::TARGET_3D:
		case gli::TARGET_CUBE_ARRAY:
			glTexStorage3D(
				Target, static_cast<gl::int32>(Texture.levels()), Format.Internal,
				Dimensions.x, Dimensions.y, Texture.target() == gli::TARGET_3D ? Dimensions.z : static_cast<gl::sizei>(Texture.layers() * Texture.faces()));
			break;
		default:
			assert(0);
			break;
		}

		for (std::size_t Layer = 0; Layer < Texture.layers(); ++Layer)
			for (std::size_t Face = 0; Face < Texture.faces(); ++Face)
				for (std::size_t Level = 0; Level < Texture.levels(); ++Level)
				{
					glm::tvec3<gl::sizei> Dimensions(Texture.dimensions(Level));
					Target = gli::is_target_cube(Texture.target()) ? static_cast<gl::enumerator>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face) : Target;

					switch (Texture.target())
					{
					case gli::TARGET_1D:
						if (gli::is_compressed(Texture.format()))
							glCompressedTexSubImage1D(
								Target, static_cast<gl::int32>(Level), 0, Dimensions.x,
								Format.Internal, static_cast<gl::sizei>(Texture.size(Level)), Texture.data(Layer, Face, Level));
						else
							glTexSubImage1D(
								Target, static_cast<gl::int32>(Level), 0, Dimensions.x,
								Format.External, Format.Type, Texture.data(Layer, Face, Level));
						break;
					case gli::TARGET_1D_ARRAY:
					case gli::TARGET_2D:
					case gli::TARGET_CUBE:
						if (gli::is_compressed(Texture.format()))
							glCompressedTexSubImage2D(
								Target, static_cast<gl::int32>(Level),
								0, 0, Dimensions.x, Texture.target() == gli::TARGET_1D_ARRAY ? static_cast<gl::sizei>(Layer) : Dimensions.y,
								Format.Internal, static_cast<gl::sizei>(Texture.size(Level)), Texture.data(Layer, Face, Level));
						else
							glTexSubImage2D(
								Target, static_cast<gl::int32>(Level),
								0, 0, Dimensions.x, Texture.target() == gli::TARGET_1D_ARRAY ? static_cast<gl::sizei>(Layer) : Dimensions.y,
								Format.External, Format.Type, Texture.data(Layer, Face, Level));
						break;
					case gli::TARGET_2D_ARRAY:
					case gli::TARGET_3D:
					case gli::TARGET_CUBE_ARRAY:
						if (gli::is_compressed(Texture.format()))
							glCompressedTexSubImage3D(
								Target, static_cast<gl::int32>(Level),
								0, 0, 0, Dimensions.x, Dimensions.y, Texture.target() == gli::TARGET_3D ? Dimensions.z : static_cast<gl::sizei>(Layer),
								Format.Internal, static_cast<gl::sizei>(Texture.size(Level)), Texture.data(Layer, Face, Level));
						else
							glTexSubImage3D(
								Target, static_cast<gl::int32>(Level),
								0, 0, 0, Dimensions.x, Dimensions.y, Texture.target() == gli::TARGET_3D ? Dimensions.z : static_cast<gl::sizei>(Layer),
								Format.External, Format.Type, Texture.data(Layer, Face, Level));
						break;
					default: assert(0); break;
					}
				}
		return TextureName;
	}
}//namespace glu

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

graphics::material::material(const std::string texture_files[texture::MAX])
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

	gl::int32 uniform_buffer_offeset(0);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniform_buffer_offeset);

	// create the uniform transform buffer
	{
		auto uniform_buffer_size = glm::max(gl::int32((sizeof(glm::mat4) * 2) + sizeof(glm::mat3)), uniform_buffer_offeset);

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

void graphics::material::use()
{
	glBindProgramPipeline(m_PipelineName);

	// bind texture units
	for (uint32_t tu_id = 0; tu_id < enum_to_t(texture::MAX); ++tu_id)
		if (auto tex_name = m_TextureNames[tu_id])
			glBindTextureUnit(tu_id, tex_name);
	
	// bind uniform buffers
	glBindBufferBase(GL_UNIFORM_BUFFER, enum_to_t(uniform::TRANSFORM), m_UniformBufferNames[enum_to_t(uniform::TRANSFORM)]);
	glBindBufferBase(GL_UNIFORM_BUFFER, enum_to_t(uniform::LIGHT), m_UniformBufferNames[enum_to_t(uniform::LIGHT)]);
}

void graphics::material::destory()
{	
	glDeleteBuffers(enum_to_t(uniform::MAX), m_UniformBufferNames);
	glDeleteProgramPipelines(1, &m_PipelineName);
	glDeleteProgram(m_ProgramName);

	m_PipelineName = 0;
	m_ProgramName = 0;
	memset(m_UniformBufferNames, 0, sizeof(m_UniformBufferNames));
}

void graphics::material::update(glm::mat4 prj_matrix, glm::mat4 mv_matrix, glm::vec4 light_dir_intensity)
{
	// update the transform buffer structure
	{
		// uniform buffer object needs to be bound with std140 layout attribute
		glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferNames[enum_to_t(uniform::TRANSFORM)]);
		uint8_t* tranform_buffer_ptr = (uint8_t*)glMapBufferRange(GL_UNIFORM_BUFFER, 0,
			(sizeof(glm::mat4) * 2) + sizeof(glm::mat3),
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		auto mvp_matrix = prj_matrix * mv_matrix;
		auto norm_matrix = glm::mat3(
			glm::vec3(mv_matrix[0]),
			glm::vec3(mv_matrix[1]),
			glm::vec3(mv_matrix[2]));

		// copy transform matrices
		memcpy(tranform_buffer_ptr, &mvp_matrix[0], sizeof(glm::mat4));
		memcpy(tranform_buffer_ptr + sizeof(glm::mat4), &mv_matrix[0], sizeof(glm::mat4));
		memcpy(tranform_buffer_ptr + sizeof(glm::mat4) + sizeof(glm::mat4), &norm_matrix[0], sizeof(glm::mat4));

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
