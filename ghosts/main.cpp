#include "logging.hpp"
#include "ghosts.hpp"

#include <exception>

int main(int argc, const char* argv[])
{
	logging::init(argc, argv);

	try
	{
		return ghosts(argc, argv)();
	}
	catch (std::exception e)
	{
		LOG(FATAL) << e.what();
		return EXIT_FAILURE;
	}
}