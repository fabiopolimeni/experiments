#pragma once

#define GLM_STATIC_CONST_MEMBERS
#define GLM_SWIZZLE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace math
{
	struct transform
	{
		glm::vec4 p_Position;
		glm::quat p_Rotation;
	};
}
