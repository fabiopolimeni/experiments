#include "line_batcher.hpp"
#include "ogl.hpp"
#include "util.hpp"

bool graphics::line_batcher::create()
{
	glGenBuffers(enum_to_t(buffer::MAX), m_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO[enum_to_t(buffer::POSITION)]);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO[enum_to_t(buffer::COLOR)]);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return m_VBO[enum_to_t(buffer::POSITION)] && m_VBO[enum_to_t(buffer::COLOR)] && m_VAO;
}

void graphics::line_batcher::update()
{

}

void graphics::line_batcher::draw()
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_VBO[enum_to_t(buffer::POSITION)]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_VBO[enum_to_t(buffer::COLOR)]);
	
	//glMultiDrawArrays(GL_LINES, 
}

void graphics::line_batcher::destroy()
{
	glDeleteBuffers(enum_to_t(buffer::MAX), m_VBO);
	memset(m_VBO, 0, sizeof(m_VBO));
}

void graphics::line_batcher::addLineStrip(const glm::vec4& in_color, const std::vector<glm::vec4>& in_points)
{
	m_Positions.assign(in_points.begin(), in_points.end());
	m_Positions.assign(in_points.size(), in_color);
}
