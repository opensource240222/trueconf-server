#pragma once

#include <memory>
#include <type_traits>

namespace vs {
	namespace detail {
		template <typename T>
		struct has_post_construct final : T
		{
		private:
			template<typename C, typename = decltype(C::PostConstruct(std::declval<std::shared_ptr<T>&>()))>
			static constexpr bool test(char) noexcept { return true; }

			template <typename C>
			static constexpr bool test(...) noexcept { return false; }
		public:
			static constexpr bool value = test<has_post_construct>(char());
		};
	} //namespace detail

	template<class T, typename ... Args>
	typename std::enable_if<!detail::has_post_construct<T>::value, std::shared_ptr<T>>::type MakeShared(Args&& ... args)
	{
		static_assert(!std::is_constructible<T, Args...>::value, "Type T can be constructed directly, so you should use std::make_shared instead");

		struct SH final : public T
		{
			SH(Args&& ... args) : T(std::forward<Args>(args)...) {}
		};

		return std::make_shared<SH>(std::forward<Args>(args)...);
	}

	template<class T, typename... Args>
	typename std::enable_if<detail::has_post_construct<T>::value, std::shared_ptr<T>>::type MakeShared(Args&& ... args)
	{
		static_assert(!std::is_constructible<T, Args...>::value, "Type T can be constructed directly, so you should use std::make_shared instead");

		struct SH final : public T
		{
			SH(Args&& ... args) : T(std::forward<Args>(args)...) {}
			using T::PostConstruct;
		};

		std::shared_ptr<T> x = std::make_shared<SH>(std::forward<Args>(args)...);
		SH::PostConstruct(x);
		return x;
	}

} //namespace vs
