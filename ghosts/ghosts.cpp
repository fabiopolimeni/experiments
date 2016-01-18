#include "model.hpp"
#include "compute.hpp"
#include "graphics.hpp"
#include "logging.hpp"
#include "ghosts.hpp"

bool ghosts::begin()
{
	if (graphics::renderer::init() && compute::clothing::init())
	{
		if (auto m = framework::model::load("data/models/yoda/yoda-head.obj", framework::model::ASCII))
		{
			if (m->activate())
			{
				m_Models.push_back(m);
				return true;
			}
		}
	}

	return false;
}

bool ghosts::end()
{
	for (auto model : m_Models)
		framework::model::destroy(model);

	return graphics::renderer::shutdown() && compute::clothing::shutdown();
}

bool ghosts::render()
{
	glm::vec2 window_size(getWindowSize());
	glm::mat4 projection_matrix = glm::perspectiveFov(glm::pi<float>() * 0.25f, window_size.x, window_size.y, 0.1f, 100.0f);

	graphics::renderer::clear(window_size, glm::vec4(.95f));

	// simulate
	for (auto model : m_Models)
		model->simulate(0.016f);

	// light: direction light.xyz, intensity light.w
	glm::vec4 light_vec(-1.f, -2.f, 10.f, 100.f);

	// render
	for (auto model : m_Models)
		model->render(projection_matrix, view(), light_vec);

	return true;
}
