///////////////////////////////////////////////////////////////////////////////////
/// OpenGL Image (gli.g-truc.net)
///
/// Copyright (c) 2008 - 2015 G-Truc Creation (www.g-truc.net)
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
///
/// @ref core
/// @file gli/core/transform.cpp
/// @date 2015-11-27 / 2015-11-27
/// @author Christophe Riccio
///////////////////////////////////////////////////////////////////////////////////

#include <gli/transform.hpp>

namespace
{
	gli::u8vec4 average(gli::u8vec4 const & A, gli::u8vec4 const & B)
	{
		return gli::u8vec4((gli::uvec4(A) + gli::uvec4(B)) / gli::uvec4(2));
	}
}//namespace

namespace transform
{
	template <typename texture_type>
	int test()
	{
		int Error = 0;
		
		texture_type TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, typename texture_type::texelcoord_type(4));
		TextureA.clear(gli::u8vec4(255, 127, 0, 255));
		texture_type TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, typename texture_type::texelcoord_type(4));
		TextureB.clear(gli::u8vec4(255, 127, 64, 192));
		texture_type TextureO(gli::FORMAT_RGBA8_UNORM_PACK8, typename texture_type::texelcoord_type(4));
		
		gli::transform<gli::u8vec4>(TextureO, TextureA, TextureB, average);
		
		gli::u8vec4 const * const data = TextureO.template data<gli::u8vec4>();
		for(gli::texture1D::size_type TexelIndex = 0, TexelCount = TextureO.template size<gli::u8vec4>(); TexelIndex < TexelCount; ++TexelIndex)
		{
			Error += *(data + TexelIndex) == gli::u8vec4(255, 127, 32, 223) ? 0 : 1;
			GLI_ASSERT(!Error);
		}
		
		return Error;
	}
}//namespace transform

namespace transform_array
{
	template <typename texture_type>
	int test()
	{
		int Error = 0;
		
		texture_type TextureA(gli::FORMAT_RGBA8_UNORM_PACK8, typename texture_type::texelcoord_type(4), 2);
		TextureA.clear(gli::u8vec4(255, 127, 0, 255));
		texture_type TextureB(gli::FORMAT_RGBA8_UNORM_PACK8, typename texture_type::texelcoord_type(4), 2);
		TextureB.clear(gli::u8vec4(255, 127, 64, 192));
		texture_type TextureO(gli::FORMAT_RGBA8_UNORM_PACK8, typename texture_type::texelcoord_type(4), 2);
		
		gli::transform<gli::u8vec4>(TextureO, TextureA, TextureB, average);
		
		gli::u8vec4 const * const data = TextureO.template data<gli::u8vec4>();
		for(typename texture_type::size_type TexelIndex = 0, TexelCount = TextureO.template size<gli::u8vec4>(); TexelIndex < TexelCount; ++TexelIndex)
		{
			Error += *(data + TexelIndex) == gli::u8vec4(255, 127, 32, 223) ? 0 : 1;
			GLI_ASSERT(!Error);
		}
		
		return Error;
	}
}//namespace transform_array

int main()
{
	int Error = 0;
	
	Error += transform::test<gli::texture1D>();
	Error += transform_array::test<gli::texture1DArray>();
	Error += transform::test<gli::texture2D>();
	Error += transform_array::test<gli::texture2DArray>();
	Error += transform::test<gli::texture3D>();
	Error += transform::test<gli::textureCube>();
	Error += transform_array::test<gli::textureCubeArray>();
	
	return Error;
}
