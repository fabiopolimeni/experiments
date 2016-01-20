#include "mesh.hpp"
#include "ogl.hpp"
#include "util.hpp"

bool graphics::mesh::create()
{
	assert(m_VAO == 0);

	// at least the vertex positions have to be provided
	bool valid_buffers = false;

	glGenBuffers(enum_to_t(buffer::MAX), m_IBO);

	if (p_PosRadius.size())
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_IBO[enum_to_t(buffer::POSITION)]);
		glBufferData(GL_ARRAY_BUFFER, p_PosRadius.size() * sizeof(glm::vec4), p_PosRadius.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		valid_buffers = true;
	}

	if (p_Normals.size() > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_IBO[enum_to_t(buffer::NORMAL)]);
		glBufferData(GL_ARRAY_BUFFER, p_Normals.size() * sizeof(glm::vec4), p_Normals.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		valid_buffers = true;
	}

	if (p_TexCoords.size() > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_IBO[enum_to_t(buffer::TEXCOORDS)]);
		glBufferData(GL_ARRAY_BUFFER, p_TexCoords.size() * sizeof(glm::vec2), p_TexCoords.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		valid_buffers = true;
	}

	if (p_FaceIndices.size() > 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO[enum_to_t(buffer::ELEMENT)]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, p_FaceIndices.size() * sizeof(uint32_t), p_FaceIndices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	if (valid_buffers)
	{
		glGenVertexArrays(1, &m_VAO);
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO[enum_to_t(buffer::ELEMENT)]);
		glBindVertexArray(0);
	}

	return valid_buffers;
}

void graphics::mesh::use()
{
	assert(glIsVertexArray(m_VAO));

	for (gl::uint32 i = 0; i < enum_to_t(buffer::MAX); ++i)
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, m_IBO[i]);
	
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, (gl::sizei)p_FaceIndices.size(), GL_UNSIGNED_INT, gl::bufferOffset(0));
}

void graphics::mesh::destroy()
{
	glDeleteBuffers(enum_to_t(buffer::MAX), m_IBO);
	glDeleteBuffers(1, &m_VAO);

	memset(m_IBO, 0, sizeof(m_IBO));
	m_VAO = 0;
}
