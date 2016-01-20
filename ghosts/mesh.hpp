#pragma once

#include "transform.hpp"
#include "resource.hpp"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <vector>

namespace graphics
{
	class mesh : public resource<mesh>
	{

	public:
		
		enum class buffer : uint32_t
		{
			POSITION,
			NORMAL,
			TEXCOORDS,
			ELEMENT,
			MAX
		};

	private:

		uint32_t m_VAO;					// vertex array object
		uint32_t m_IBO[buffer::MAX];	// input buffer objects

	public:

		math::transform			p_MeshToModel;

		std::vector<glm::vec4>	p_PosRadius;
		std::vector<glm::vec4>	p_VelInvMass;
		std::vector<glm::vec4>	p_Normals;

		std::vector<glm::vec2>	p_TexCoords;
		std::vector<uint32_t>	p_FaceIndices;

		bool create();
		void use();
		void destroy();
	};
}