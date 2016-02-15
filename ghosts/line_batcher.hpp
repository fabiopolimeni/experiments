#pragma once

#include "resource.hpp"

#include <glm/vec4.hpp>

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
			MAX
		};

	private:

		handle m_VBO[buffer::MAX];	// vertex buffer objects
		handle m_VAO;	// vertex array object

		std::vector<glm::vec4>	m_Positions;
		std::vector<glm::vec4>	m_Colors;

	public:
		
		bool create();
		void update();
		void draw();
		void destroy();

		void addLineStrip(const glm::vec4& in_color, const std::vector<glm::vec4>& in_points);
	};
}
