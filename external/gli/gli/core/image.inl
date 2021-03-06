namespace gli{
namespace detail
{
	template <typename T, precision P>
	inline size_t texelLinearAdressing
	(
		tvec1<T, P> const & Dimensions,
		tvec1<T, P> const & TexelCoord
	)
	{
		GLI_ASSERT(glm::all(glm::lessThan(TexelCoord, Dimensions)));

		return static_cast<size_t>(TexelCoord.x);
	}

	template <typename T, precision P>
	inline size_t texelLinearAdressing
	(
		tvec2<T, P> const & Dimensions,
		tvec2<T, P> const & TexelCoord
	)
	{
		GLI_ASSERT(TexelCoord.x < Dimensions.x);
		GLI_ASSERT(TexelCoord.y < Dimensions.y);

		return static_cast<size_t>(TexelCoord.x + Dimensions.x * TexelCoord.y);
	}

	template <typename T, precision P>
	inline size_t texelLinearAdressing
	(
		tvec3<T, P> const & Dimensions,
		tvec3<T, P> const & TexelCoord
	)
	{
		GLI_ASSERT(TexelCoord.x < Dimensions.x);
		GLI_ASSERT(TexelCoord.y < Dimensions.y);
		GLI_ASSERT(TexelCoord.z < Dimensions.z);

		return static_cast<size_t>(TexelCoord.x + Dimensions.x * (TexelCoord.y + Dimensions.y * TexelCoord.z));
	}

	inline size_t texelMortonAdressing
	(
		dim1_t const & Dimensions,
		dim1_t const & TexelCoord
	)
	{
		GLI_ASSERT(TexelCoord.x < Dimensions.x);

		return TexelCoord.x;
	}

	inline size_t texelMortonAdressing
	(
		dim2_t const & Dimensions,
		dim2_t const & TexelCoord
	)
	{
		GLI_ASSERT(TexelCoord.x < Dimensions.x && TexelCoord.x >= 0 && TexelCoord.x < std::numeric_limits<dim2_t::value_type>::max());
		GLI_ASSERT(TexelCoord.y < Dimensions.y && TexelCoord.y >= 0 && TexelCoord.y < std::numeric_limits<dim2_t::value_type>::max());

		glm::u32vec2 const Input(TexelCoord);

		return glm::bitfieldInterleave(Input.x, Input.y);
	}

	inline size_t texelMortonAdressing
	(
		dim3_t const & Dimensions,
		dim3_t const & TexelCoord
	)
	{
		GLI_ASSERT(TexelCoord.x < Dimensions.x);
		GLI_ASSERT(TexelCoord.y < Dimensions.y);
		GLI_ASSERT(TexelCoord.z < Dimensions.z);

		glm::u32vec3 const Input(TexelCoord);

		return glm::bitfieldInterleave(Input.x, Input.y, Input.z);
	}
}//namespace detail

	inline image::image()
		: Format(static_cast<gli::format>(FORMAT_INVALID))
		, BaseLevel(0)
		, Data(nullptr)
		, Size(0)
	{}

	inline image::image
	(
		format_type Format,
		texelcoord_type const & Dimensions
	)
		: Storage(std::make_shared<storage>(Format, Dimensions, 1, 1, 1))
		, Format(Format)
		, BaseLevel(0)
		, Data(Storage->data())
		, Size(compute_size(0))
	{}

	inline image::image
	(
		std::shared_ptr<storage> Storage,
		format_type Format,
		size_type BaseLayer,
		size_type BaseFace,
		size_type BaseLevel
	)
		: Storage(Storage)
		, Format(Format)
		, BaseLevel(BaseLevel)
		, Data(compute_data(BaseLayer, BaseFace, BaseLevel))
		, Size(compute_size(BaseLevel))
	{}

	inline image::image
	(
		image const & Image,
		format_type Format
	)
		: Storage(Image.Storage)
		, Format(Format)
		, BaseLevel(Image.BaseLevel)
		, Data(Image.Data)
		, Size(Image.Size)
	{
		GLI_ASSERT(block_size(Format) == block_size(Image.format()));
	}

	inline bool image::empty() const
	{
		if(this->Storage.get() == nullptr)
			return true;

		return this->Storage->empty();
	}

	inline image::size_type image::size() const
	{
		GLI_ASSERT(!this->empty());

		return this->Size;
	}

	template <typename genType>
	inline image::size_type image::size() const
	{
		GLI_ASSERT(sizeof(genType) <= this->Storage->block_size());

		return this->size() / sizeof(genType);
	}

	inline image::format_type image::format() const
	{
		return this->Format;
	}

	inline image::texelcoord_type image::dimensions() const
	{
		GLI_ASSERT(!this->empty());

		storage::texelcoord_type const & SrcDimensions = this->Storage->dimensions(this->BaseLevel);
		storage::texelcoord_type const & DstDimensions = SrcDimensions * block_dimensions(this->format()) / this->Storage->block_dimensions();

		return glm::max(DstDimensions, storage::texelcoord_type(1));
	}

	inline void * image::data()
	{
		GLI_ASSERT(!this->empty());

		return this->Data;
	}

	inline void const * image::data() const
	{
		GLI_ASSERT(!this->empty());
		
		return this->Data;
	}

	template <typename genType>
	inline genType * image::data()
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(this->Storage->block_size() >= sizeof(genType));

		return reinterpret_cast<genType *>(this->data());
	}

	template <typename genType>
	inline genType const * image::data() const
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(this->Storage->block_size() >= sizeof(genType));

		return reinterpret_cast<genType const *>(this->data());
	}

	inline void image::clear()
	{
		GLI_ASSERT(!this->empty());

		memset(this->data<glm::byte>(), 0, this->size<glm::byte>());
	}

	template <typename genType>
	inline void image::clear(genType const & Texel)
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(this->Storage->block_size() == sizeof(genType));

		for(size_type TexelIndex = 0; TexelIndex < this->size<genType>(); ++TexelIndex)
			*(this->data<genType>() + TexelIndex) = Texel;
	}

	inline image::data_type * image::compute_data(size_type BaseLayer, size_type BaseFace, size_type BaseLevel)
	{
		size_type const Offset = this->Storage->offset(BaseLayer, BaseFace, BaseLevel);

		return this->Storage->data() + Offset;
	}

	inline image::size_type image::compute_size(size_type Level) const
	{
		GLI_ASSERT(!this->empty());

		return this->Storage->level_size(Level);
	}

	template <typename genType>
	genType image::load(texelcoord_type const & TexelCoord)
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(!is_compressed(this->format()));
		GLI_ASSERT(this->Storage->block_size() == sizeof(genType));
		GLI_ASSERT(glm::all(glm::lessThan(TexelCoord, this->dimensions())));

		return *(this->data<genType>() + detail::texelLinearAdressing(this->dimensions(), TexelCoord));
	}

	template <typename genType>
	void image::store(texelcoord_type const & TexelCoord, genType const & Data)
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(!is_compressed(this->format()));
		GLI_ASSERT(this->Storage->block_size() == sizeof(genType));
		GLI_ASSERT(glm::all(glm::lessThan(TexelCoord, this->dimensions())));

		*(this->data<genType>() + detail::texelLinearAdressing(this->dimensions(), TexelCoord)) = Data;
	}
}//namespace gli
