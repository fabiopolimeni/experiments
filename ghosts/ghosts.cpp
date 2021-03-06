#include "model.hpp"
#include "compute.hpp"
#include "graphics.hpp"
#include "logging.hpp"
#include "ghosts.hpp"


void ghosts::onKeyStateChange(int Key, key_action old_state, key_action new_state)
{
	// debug CTRL+D
	if (isKeyPressed(GLFW_KEY_LEFT_CONTROL) && Key == GLFW_KEY_D && new_state == test::KEY_PRESS)
	{
		for (auto model : m_Models) {
			model->toggleRenderMode(framework::model::render_mode::DEBUG);
		}
	}

	// wireframe CTRL+W
	if (isKeyPressed(GLFW_KEY_LEFT_CONTROL) && Key == GLFW_KEY_W && new_state == test::KEY_PRESS)
	{
		for (auto model : m_Models) {
			model->toggleRenderMode(framework::model::render_mode::WIREFRAME);
		}
	}

	// shaded CTRL+S
	if (isKeyPressed(GLFW_KEY_LEFT_CONTROL) && Key == GLFW_KEY_S && new_state == test::KEY_PRESS)
	{
		for (auto model : m_Models) {
			model->toggleRenderMode(framework::model::render_mode::SHADED);
		}
	}
}

bool ghosts::begin()
{
	if (graphics::renderer::init() && compute::clothing::init())
	{
		if (auto m = framework::model::load("data/models/barrel/barrel.awf", framework::model::file_type::ASCII))
	//	if (auto m = framework::model::load("data/models/kungfu-panda/kungfu.awf", framework::model::file_type::ASCII))
		{
			if (m->initialise()) {
				m_Models.push_back(m);
			}
		}
	}

	return true;
}

bool ghosts::end()
{
	for (auto model : m_Models) {
		framework::model::release(model);
	}

	return graphics::renderer::shutdown() && compute::clothing::shutdown();
}

bool ghosts::render()
{
	glm::vec2 window_size(getWindowSize());
	glm::mat4 projection_matrix = glm::perspectiveFov(glm::pi<float>() * 0.25f, window_size.x, window_size.y, 0.1f, 100.0f);

	graphics::renderer::clear(window_size, glm::vec4(.95f));

	// light: direction light.xyz, intensity light.w
	//glm::vec4 light_vec(-1.f, -2.f, 0.f, 100.f);
	glm::vec4 light_vec(-1.f, -1.f, 0.f, 100.f);

	// simulate
	for (auto model : m_Models) {
		model->simulate(0.016f);
	}

	// render
	for (auto model : m_Models) {
		model->render(projection_matrix, view(), light_vec);
	}

	return true;
}
