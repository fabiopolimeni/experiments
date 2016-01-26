#pragma once

#include "resource.hpp"
#include <string>

namespace graphics
{
	class texture : public resource<texture>
	{

		handle m_TextureName;

	public:

		bool create(const std::string& filename);
		void destroy();
		void use(uint32_t texture_unit);

		inline handle getHandle() const { return m_TextureName; }
	};
}
