#pragma once

#include "resource.hpp"
#include <string>

namespace graphics
{
	class texture : public resource<texture>
	{

	public:

		bool create(const std::string& filename);
		void destory();
		void use();
	};
}
