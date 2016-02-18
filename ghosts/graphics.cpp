#include "graphics.hpp"
#include "compiler.hpp"
#include "ogl.hpp"

#include <glm/vec3.hpp>

namespace graphics
{
	bool renderer::init()
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glEnable(GL_DEPTH_TEST);

		//glEnable(GL_LINE_SMOOTH);
		glLineWidth(1.f);

		//glEnable(GL_POINT_SMOOTH);
		glPointSize(2.f);

		return true;
	}

	bool renderer::shutdown()
	{
		return true;
	}

	void renderer::clear(glm::vec2 window_size, glm::vec4 view_color)
	{
		float depth_value = 1.0f;
		glClearBufferfv(GL_DEPTH, 0, &depth_value);
		glClearBufferfv(GL_COLOR, 0, &view_color[0]);

		auto viewport = glm::vec4(0.0f, 0.0f, window_size.x, window_size.y);
		glViewportIndexedfv(0, &viewport[0]);
	}
}
