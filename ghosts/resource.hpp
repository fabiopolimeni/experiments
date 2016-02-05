#pragma once

#include <cstdint>
#include <utility>

namespace graphics
{
	template<typename T>
	class resource
	{

	public:

		typedef uint32_t handle;
		
		static const handle invalid = 0;

		template<typename... R>
		inline auto initialise(R&&... args)
		{
			return static_cast<T*>(this)->create(std::forward<R>(args)...);
		}

		template<typename... R>
		inline auto release(R&&... args)
		{
			return static_cast<T*>(this)->destroy(std::forward<R>(args)...);
		}

		template<typename... R>
		inline auto bind(R&&... args)
		{
			return static_cast<T*>(this)->use(std::forward<R>(args)...);
		}

		template<typename... R>
		inline auto change(R&&... args)
		{
			return static_cast<T*>(this)->update(std::forward<R>(args)...);
		}
	};
}