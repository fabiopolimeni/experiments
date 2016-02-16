#pragma once

#include "resource.hpp"
#include "util.hpp"

#define GLM_STATIC_CONST_MEMBERS
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <vector>

namespace graphics
{
	class line_batcher : resource<line_batcher>
	{

	public:

		enum class buffer : uint32_t
		{
			POSITION,
			COLOR,
			TEXCOORDS,
			MAX
		};

		enum class uniform : uint32_t
		{
			TRANSFORM,
			MAX
		};

		struct strip
		{
			glm::vec4				p_Color;
			glm::vec2				p_Width;
			std::vector<glm::vec4>	p_Points;
		};

	private:

		handle	m_UBO[enum_to_t(uniform::MAX)];	// uniform buffer object
		handle	m_VBO[enum_to_t(buffer::MAX)];	// vertex buffer objects
		handle	m_VAO;							// vertex array object

		handle	m_Pipe;							// pipeline object
		handle	m_Prog;							// pipeline object

		bool	m_Dirty;						// Whether or not the lines buffer is dirty

		std::vector<strip*>	m_Strips;

		bool initBuffers();
		bool initShaders();

	public:

		line_batcher();
		
		bool create();
		
		void update(
			glm::mat4 prj_matrix,	// projection matrix
			glm::mat4 mv_matrix		// model * view matrix
		);

		void draw();
		void destroy();

		const strip* addStrip(const glm::vec4& in_color,
			const std::vector<glm::vec4>& in_points,
			const glm::vec2& in_width = glm::vec2::XY);

		void removeStrip(const strip* in_strip);
	};
}
