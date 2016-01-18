/*
Copyright (c) 2015, Fabio Polimeni
The MIT License (MIT)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

@filename: log.hpp, 2015-8-2 (yy-mm-dd)
@author: Fabio
*/

#pragma once

#define ELPP_DEBUG_ASSERT_FAILURE
#define ELPP_THREAD_SAFE
#define ELPP_FORCE_USE_STD_THREAD

// On non-debug builds debug log level is disabled
#if !defined(NDEBUG)
#define ELPP_DISABLE_DEBUG_LOGS
#endif

// On shipping builds info log level is disabled
#if defined(NDEBUG)
#define ELPP_DISABLE_TRACE_LOGS
#define ELPP_DISABLE_INFO_LOGS
#define ELPP_DISABLE_WARNING_LOGS
#endif

#if defined(_MSC_VER)
#define ELPP_WINSOCK2
#endif

#define ELPP_DEFAULT_LOG_FILE "default_log.txt"

#include "easylogging++.h"

namespace logging
{
	void init(int32_t argc, const char* argv[]);
}

