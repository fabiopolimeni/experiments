#include "line_batcher.hpp"
#include "ogl.hpp"
#include "util.hpp"
#include "compiler.hpp"

#include <algorithm>
#include <iterator>

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

bool graphics::line_batcher::initBuffers()
{
	m_BufferSize = 0;
	glGenBuffers(enum_to_t(buffer::MAX), m_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO[enum_to_t(buffer::POSITION)]);
	glBufferData(GL_ARRAY_BUFFER, m_BufferSize, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO[enum_to_t(buffer::COLOR)]);
	glBufferData(GL_ARRAY_BUFFER, m_BufferSize, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	gl::int32 uniform_buffer_offeset(0);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniform_buffer_offeset);

	// create the uniform transform buffer
	{
		const auto uniform_buffer_size = glm::max(gl::int32((sizeof(glm::mat4) * 2)), uniform_buffer_offeset);

		glGenBuffers(1, &m_UBO[enum_to_t(uniform::TRANSFORM)]);
		glBindBuffer(GL_UNIFORM_BUFFER, m_UBO[enum_to_t(uniform::TRANSFORM)]);
		glBufferData(GL_UNIFORM_BUFFER, uniform_buffer_size, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	// NOTE: not used at the moment, it will be, when start 
	//		 rendering lines as screen space polygons.
	glBindVertexArray(0);

	return m_VAO && m_VBO[enum_to_t(buffer::POSITION)] && m_VBO[enum_to_t(buffer::COLOR)] && m_VAO;;
}

bool graphics::line_batcher::initShaders()
{
	char const * VS_SOURCE = "data/shaders/line.vert";
	char const * FS_SOURCE = "data/shaders/line.frag";

	m_Prog = glCreateProgram();

	compiler compiler_instance;
	auto pipe_mask = build(m_Prog, compiler_instance, VS_SOURCE, FS_SOURCE);
	if (pipe_mask != 0 && compiler_instance.checkProgram(m_Prog))
	{
		glGenProgramPipelines(1, &m_Pipe);
		glUseProgramStages(m_Pipe, pipe_mask, m_Prog);
		return true;
	}

	return false;
}

graphics::line_batcher::line_batcher()
	: m_BufferSize(0), m_Dirty(false)
{
}

bool graphics::line_batcher::create()
{
	return initShaders() && initBuffers();
}

void graphics::line_batcher::update(glm::mat4 prj_matrix, glm::mat4 mv_matrix)
{
	// update the transform buffer structure
	{
		// uniform buffer object needs to be bound with std140 layout attribute
		glBindBuffer(GL_UNIFORM_BUFFER, m_UBO[enum_to_t(uniform::TRANSFORM)]);
		uint8_t* tranform_buffer_ptr = (uint8_t*)glMapBufferRange(GL_UNIFORM_BUFFER, 0,
			sizeof(glm::mat4) * 2, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		auto mvp_matrix = prj_matrix * mv_matrix;

		// copy transform matrices
		memcpy(tranform_buffer_ptr, &mvp_matrix[0], sizeof(glm::mat4));
		memcpy(tranform_buffer_ptr + sizeof(glm::mat4), &mv_matrix[0], sizeof(glm::mat4));

		// make sure the uniform buffer is uploaded
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}

	// update shader storage buffer, containing vertex attributes if necessary only
	if (m_Dirty)
	{
		// we need to reallocate the buffer, as we might have added or removed
		// a line strip from our vector of strip. The way we are going to this
		// is by orphaning the previous buffer, while the driver will re-allocate
		// a new chunk of memory for us, to accommodate the new size, if necessary.
		
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO[enum_to_t(buffer::POSITION)]);
		glBufferData(GL_ARRAY_BUFFER, m_BufferSize, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO[enum_to_t(buffer::COLOR)]);
		glBufferData(GL_ARRAY_BUFFER, m_BufferSize, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// re-fill the vertex arrays and store the new buffer size
		std::vector<glm::vec4>	positions;
		std::vector<glm::vec4>	colors;
		for (auto s : m_Strips)
		{
			positions.assign(s->p_Points.begin(), s->p_Points.end());
			colors.assign(positions.size(), s->p_Color);

			m_BufferSize = positions.size() * sizeof(decltype(positions)::value_type);
		}

		// copy data into the vertex arrays
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO[enum_to_t(buffer::POSITION)]);
		glBufferData(GL_ARRAY_BUFFER, m_BufferSize, positions.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO[enum_to_t(buffer::COLOR)]);
		glBufferData(GL_ARRAY_BUFFER, m_BufferSize, colors.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		m_Dirty = false;
	}
}

void graphics::line_batcher::draw()
{
	glBindProgramPipeline(m_Pipe);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_VBO[enum_to_t(buffer::POSITION)]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_VBO[enum_to_t(buffer::COLOR)]);
	
	// buffer vertex count
	assert(m_BufferSize < std::numeric_limits<gl::sizei>::max());
	const gl::sizei buffer_lenght = gl::sizei(m_BufferSize / sizeof(glm::vec4));
	glDrawArrays(GL_LINES, 0, buffer_lenght);
}

void graphics::line_batcher::destroy()
{
	glDeleteBuffers(enum_to_t(uniform::MAX), m_UBO);
	memset(m_UBO, 0, sizeof(m_UBO));

	glDeleteBuffers(enum_to_t(buffer::MAX), m_VBO);
	memset(m_VBO, 0, sizeof(m_VBO));

	glDeleteBuffers(1, &m_VAO);
	m_VAO = 0;

	glDeleteProgramPipelines(1, &m_Pipe);
	m_Pipe = 0;

	glDeleteProgram(m_Prog);
	m_Prog = 0;

	for (auto s : m_Strips) {
		delete s;
	}

	m_Strips.clear();
}

const graphics::line_batcher::strip* graphics::line_batcher::addStrip(
	const glm::vec4 in_color, const std::vector<glm::vec4> in_points, const glm::vec2 in_width)
{
	strip* new_strip = new strip();
	new_strip->p_Points.assign(in_points.begin(), in_points.end());
	new_strip->p_Color = in_color;
	new_strip->p_Width = in_width;

	m_Strips.emplace_back(new_strip);
	m_Dirty = true;

	return new_strip;
}

void graphics::line_batcher::removeStrip(const graphics::line_batcher::strip * in_strip)
{
	auto it = std::find(m_Strips.begin(), m_Strips.end(), in_strip);
	if (it != std::end(m_Strips))
	{
		delete (*it);
		m_Strips.erase(it);
	}
}
