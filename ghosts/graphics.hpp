#pragma once

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

class compiler;

namespace graphics
{
	struct renderer
	{
		static bool init();
		static bool shutdown();
		static void clear(glm::vec2 window_size, glm::vec4 view_color);
	};

	struct polygon_offset
	{
		polygon_offset(bool in_enable, float in_offset);
		~polygon_offset();
	};

	struct line_offset
	{
		line_offset(bool in_enable, float in_offset);
		~line_offset();
	};

	struct point_offset
	{
		point_offset(bool in_enable, float in_offset);
		~point_offset();
	};
}
