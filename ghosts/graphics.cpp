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

		// sRGB framebuffer
		glEnable(GL_FRAMEBUFFER_SRGB);

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
	
	polygon_offset::polygon_offset(bool in_enable, float in_offset)
	{
		if (in_enable)
		{
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(1.1f, in_offset);
		}
	}

	polygon_offset::~polygon_offset()
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	line_offset::line_offset(bool in_enable, float in_offset)
	{
		if (in_enable)
		{
			glEnable(GL_POLYGON_OFFSET_LINE);
			glPolygonOffset(-.9f, -in_offset);
		}
	}

	line_offset::~line_offset()
	{
		glDisable(GL_POLYGON_OFFSET_LINE);
	}

	point_offset::point_offset(bool in_enable, float in_offset)
	{
		if (in_enable)
		{
			glEnable(GL_POLYGON_OFFSET_POINT);
			glPolygonOffset(-.9f, -in_offset);
		}
	}

	point_offset::~point_offset()
	{
		glDisable(GL_POLYGON_OFFSET_POINT);
	}
}
