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
}
