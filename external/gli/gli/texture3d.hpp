/// @brief Include to use 3d textures.
/// @file gli/texture3d.hpp

#pragma once

#include "texture.hpp"
#include "image.hpp"
#include "index.hpp"

namespace gli
{
	/// 3d texture
	class texture3D : public texture
	{
	public:
		typedef ivec3 texelcoord_type;

	public:
		/// Create an empty texture 3D
		texture3D();

		/// Create a texture3D and allocate a new storage
		explicit texture3D(
			format_type Format,
			texelcoord_type const & Dimensions,
			size_type Levels);

		/// Create a texture3D and allocate a new storage with a complete mipmap chain
		explicit texture3D(
			format_type Format,
			texelcoord_type const & Dimensions);

		/// Create a texture3D view with an existing storage
		explicit texture3D(
			texture const & Texture);

		/// Create a texture3D view with an existing storage
		explicit texture3D(
			texture const & Texture,
			format_type Format,
			size_type BaseLayer, size_type MaxLayer,
			size_type BaseFace, size_type MaxFace,
			size_type BaseLevel, size_type MaxLevel);

		/// Create a texture3D view, reference a subset of an existing texture3D instance
		explicit texture3D(
			texture3D const & Texture,
			size_type BaseLevel, size_type MaxLevel);

		/// Create a view of the image identified by Level in the mipmap chain of the texture
		image operator[](size_type Level) const;

		/// Return the dimensions of a texture instance: width, height and depth 
		texelcoord_type dimensions(size_type Level = 0) const;

		/// Fetch a texel from a texture. The texture format must be uncompressed.
		template <typename genType>
		genType load(texelcoord_type const & TexelCoord, size_type Level) const;

		/// Write a texel to a texture. The texture format must be uncompressed.
		template <typename genType>
		void store(texelcoord_type const & TexelCoord, size_type Level, genType const & Texel);

		/// Clear the entire texture storage with zeros
		void clear();

		/// Clear the entire texture storage with Texel which type must match the texture storage format block size
		/// If the type of genType doesn't match the type of the texture format, no conversion is performed and the data will be reinterpreted as if is was of the texture format. 
		template <typename genType>
		void clear(genType const & Texel);

		/// Clear a specific image of a texture.
		template <typename genType>
		void clear(size_type Level, genType const & Texel);

	private:
		struct cache
		{
			std::uint8_t* Data;
			texelcoord_type Dim;
#			ifndef NDEBUG
				size_type Size;
#			endif
		};

		void build_cache();
		size_type index_cache(size_type Level) const;

		std::vector<cache> Caches;
	};
}//namespace gli

#include "./core/texture3d.inl"
