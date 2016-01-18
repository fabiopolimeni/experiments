#pragma once

#include "test.hpp"

namespace framework
{
	class scene;
	class model;
}

class ghosts : public test
{

private:

	std::vector<framework::model*> m_Models;

public:

	ghosts(int argc, const char* argv[])
		: test(argc, const_cast<char**>(argv), "ghosts", test::CORE, 4, 5)
	{}

	virtual bool begin() override;
	virtual bool end() override;
	virtual bool render() override;
};
