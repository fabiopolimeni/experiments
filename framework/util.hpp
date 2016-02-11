///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Samples Pack (ogl-samples.g-truc.net)
///
/// Copyright (c) 2004 - 2014 G-Truc Creation (www.g-truc.net)
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

std::string message_format(const char* Message, ...);

template<typename Enum>
constexpr auto enum_to_t(Enum e) noexcept
{
	return static_cast<std::underlying_type_t<Enum>>(e);
}

namespace path
{
	inline std::string left(const std::string& file_path, const char char_to_search)
	{
		std::string file_basepath = file_path;
		auto const dir_pos = file_path.find_last_of(char_to_search);
		if (dir_pos != std::string::npos)
		{
			file_basepath = file_path.substr(0, dir_pos + 1);
		}

		return file_basepath;
	}

	inline std::string right(const std::string& file_path, const char char_to_search)
	{
		std::string filename = file_path;
		auto slash_pos = file_path.find_last_of(char_to_search);
		if (slash_pos != std::string::npos)
		{
			filename = file_path.substr(slash_pos + 1, std::string::npos);
		}

		return filename;
	}
}