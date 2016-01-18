#include "logging.hpp"


INITIALIZE_EASYLOGGINGPP;

namespace logging
{
	void init(int32_t argc, const char* argv[])
	{
		START_EASYLOGGINGPP(argc, argv);
	}
}