#ifndef VARIANT_H
#define VARIANT_H

#include <limits>
#include <tuple>
#include <type_traits>

#include "common.h"

namespace {
	template <class T, class Tuple>
	struct Index;

	template <class T, class... Types>
	struct Index<T, std::tuple<T, Types...>> {
		static const std::size_t value = 0;
	};

	template <class T, class U, class... Types>
	struct Index<T, std::tuple<U, Types...>> {
		static const std::size_t value = 1 + Index<T, std::tuple<Types...>>::value;
	};

	template<class T, class... Ts>
	struct Present;

	template<class T, class U>
	struct Present<T, U> {
		static constexpr bool value = std::is_same<T, U>::value;
	};

	template<class T, class U, class... Ts>
	struct Present<T, U, Ts...> {
		static constexpr bool value = std::is_same<T, U>::value || Present<T, Ts...>::value;
	};

	template<class T, class... Ts>
	struct AllUnique {
		static constexpr bool value = !Present<T, Ts...>::value && AllUnique<Ts...>::value;
	};

	template<class T>
	struct AllUnique<T> {
		static constexpr bool value = true;
	};

	template<class C, class T>
	struct IsConvertibleTo {
		static constexpr bool value = std::is_base_of<T, C>::value || std::is_same<T, void>::value || std::is_same<T, C>::value;
	};

	template<class C, class T, class... Ts>
	struct AnyConvertibleTo {
		static constexpr bool value = IsConvertibleTo<T, C>::value || AnyConvertibleTo<C, Ts...>::value;
	};

	template<class C, class T>
	struct AnyConvertibleTo<C, T> {
		static constexpr bool value = IsConvertibleTo<T, C>::value;
	};

	template<class T, class R = void>
	using enable_when_void = std::enable_if_t<std::is_same<T, void>::value, R>;

	template<class T, class R = void>
	using enable_unless_void = std::enable_if_t<!std::is_same<T, void>::value, R>;
}

// This is a simple, lightweight, never-empty variant similar in spirit to
// boost::variant, but supposing that constructors and destructors do not throw
// (greatly simplifying the implementation).  This is a valid assumption on
// Pebble; it is probably not valid elsewhere.
// One very handy feature is the ability to construct an object based on a
// typenum using Variant::emplace(typenum, args...); the appropriate
// constructor is chosen at runtime.  This only works if all the variant
// subtypes have a constructor capable of accepting the passed args.
template<typename... Ts>
class Variant
{
	using data_t = typename std::aligned_union<0, Ts...>::type;
	data_t data;

	using typenum_t = uint8_t;
	typenum_t typenum;

	static_assert(AllUnique<Ts...>::value,
	              "All types must be unique!");
	static_assert(sizeof...(Ts) < std::numeric_limits<typenum_t>::max(),
	              "Too many variant types!");

public:
	template<typename T>
	static constexpr typenum_t typenum_of() {
		return Index<T, std::tuple<Ts...>>::value;
	}

	Variant() {
		create_as<void>();
	}

	template<typename T>
	Variant(const T& value) {
		create_as<T>(value);
	}

	~Variant() {
		destroy();
	}

	template<typename T, typename... Args>
	T& emplace_as(Args&&... args) {
		check_type<T>();
		destroy();
		create_as<T>(std::forward<Args>(args)...);
		return data_ref<T>();
	}

	template<typename... Args>
	void emplace(typenum_t type, Args&&... args) {
		destroy();
		create(type, std::forward<Args>(args)...);
	}

	template<typename T>
	T& operator=(const T& other) {
		check_type<T>();
		destroy();
		create_as<T>(other);
		return data_ref<T>();
	}

	void reset() {
		check_type<void>();
		destroy();
		create_as<void>();
	}

	typenum_t type() const {
		return typenum;
	}

	template<typename T>
	T& as() {
		check_type<T>();
		return data_ref<T>();
	}

	template<typename T>
	inline bool is() {
		check_type<T>();
		return typenum_of<T>() == typenum;
	}

	template<typename T, typename F> enable_unless_void<T>
	inline if_is(F f) {
		check_type<T>();
		if(is<T>()) {
			f(data_ref<T>());
		}
	}

	template<typename T, typename F> enable_when_void<T>
	inline if_is(F f) {
		check_type<T>();
		if(is<T>()) {
			f();
		}
	}

	template<typename T, typename IfTrue, typename Else> enable_unless_void<T>
	inline if_is_else(IfTrue t, Else f) {
		check_type<T>();
		if(is<T>()) {
			t(data_ref<T>());
		}
		else {
			f();
		}
	}

	template<typename T, typename IfTrue, typename Else> enable_when_void<T>
	inline if_is_else(IfTrue t, Else f) {
		check_type<T>();
		if(is<T>()) {
			t();
		}
		else {
			f();
		}
	}

private:
	template<typename T, typename... Args> enable_unless_void<T>
	create_as(Args&&... args) {
		typenum = typenum_of<T>();
		::new (&data) T(std::forward<Args>(args)...);
	}

	// As a special case, we throw away any arguments when "constructing" void.
	template<typename T, typename... Args> enable_when_void<T>
	create_as(Args&&... args) {
		typenum = typenum_of<T>();
	}

	template<typename... Args>
	void create(typenum_t type, Args&&... args) {
		typenum = type;
		create_helper<0, Ts...>(std::forward<Args>(args)...);
	}

	template<int N, typename... R, typename... Args>
	std::enable_if_t<(N < sizeof...(R))>
	create_helper(Args&&... args) {
		if(typenum == N) {
			using T = typename std::tuple_element<N, std::tuple<R...>>::type;
			create_as<T>(std::forward<Args>(args)...);
		}
		else {
			create_helper<N+1, R...>(std::forward<Args>(args)...);
		}
	}

	template<int N, typename... R, typename... Args>
	std::enable_if_t<(N == sizeof...(R))>
	create_helper(Args&&... args) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Didn't create variant type %i", N);
	}

	template<typename T> enable_unless_void<T>
	destroy_as() {
		data_ref<T>().~T();
	}

	template<typename T> enable_when_void<T>
	destroy_as() {}

	void destroy() {
		destroy_helper<0, Ts...>();
	}

	template<int N, typename T, typename... R>
	void destroy_helper() {
		if(typenum == N) {
			destroy_as<T>();
		}
		else {
			destroy_helper<N+1, R...>();
		}
	}

	template<int N>
	void destroy_helper() {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Didn't destroy variant type %i", N);
	}

	template<typename T>
	T& data_ref() {
		check_type<T>();
		return *reinterpret_cast<T*>(&data);
	}

	template<typename T>
	void check_type() {
		static_assert(AnyConvertibleTo<T, Ts...>::value,
		              "Type not valid for this variant");
	}
};

template<typename T1, typename T2>
class Variant<T1, T2>
{
	using data_t = typename std::aligned_union<0, T1, T2>::type;
	data_t data;

	using typenum_t = bool;
	using public_typenum_t = uint8_t;
	typenum_t typenum;

	static_assert(AllUnique<T1, T2>::value,
	              "All types must be unique!");

public:
	template<typename T>
	static constexpr public_typenum_t typenum_of() {
		return Index<T, std::tuple<T1, T2>>::value;
	}

	Variant() {
		create_as<void>();
	}

	template<typename T>
	Variant(const T& value) {
		create_as<T>(value);
	}

	~Variant() {
		destroy();
	}

	template<typename T, typename... Args>
	T& emplace_as(Args&&... args) {
		check_type<T>();
		destroy();
		create_as<T>(std::forward<Args>(args)...);
		return data_ref<T>();
	}

	template<typename... Args>
	void emplace(public_typenum_t type, Args&&... args) {
		destroy();
		create(type, std::forward<Args>(args)...);
	}

	template<typename T>
	T& operator=(const T& other) {
		check_type<T>();
		destroy();
		create_as<T>(other);
		return data_ref<T>();
	}

	void reset() {
		check_type<void>();
		destroy();
		create_as<void>();
	}

	public_typenum_t type() const {
		return typenum;
	}

	template<typename T>
	T& as() {
		check_type<T>();
		return data_ref<T>();
	}

	template<typename T>
	inline bool is() {
		check_type<T>();
		return typenum_of<T>() == typenum;
	}

	template<typename T, typename F> enable_unless_void<T>
	inline if_is(F f) {
		check_type<T>();
		if(is<T>()) {
			f(data_ref<T>());
		}
	}

	template<typename T, typename F> enable_when_void<T>
	inline if_is(F f) {
		check_type<T>();
		if(is<T>()) {
			f();
		}
	}

	template<typename T, typename IfTrue, typename Else> enable_unless_void<T>
	inline if_is_else(IfTrue t, Else f) {
		check_type<T>();
		if(is<T>()) {
			t(data_ref<T>());
		}
		else {
			f();
		}
	}

	template<typename T, typename IfTrue, typename Else> enable_when_void<T>
	inline if_is_else(IfTrue t, Else f) {
		check_type<T>();
		if(is<T>()) {
			t();
		}
		else {
			f();
		}
	}

private:
	template<typename T, typename... Args> enable_unless_void<T>
	create_as(Args&&... args) {
		typenum = typenum_of<T>();
		::new (&data) T(std::forward<Args>(args)...);
	}

	// As a special case, we throw away any arguments when "constructing" void.
	template<typename T, typename... Args> enable_when_void<T>
	create_as(Args&&... args) {
		typenum = typenum_of<T>();
	}

	template<typename... Args>
	void create(typenum_t type, Args&&... args) {
		typenum = type;
		create_helper<0, T1, T2>(std::forward<Args>(args)...);
	}

	template<int N, typename... R, typename... Args>
	std::enable_if_t<(N < sizeof...(R))>
	create_helper(Args&&... args) {
		if(typenum == N) {
			using T = typename std::tuple_element<N, std::tuple<R...>>::type;
			create_as<T>(std::forward<Args>(args)...);
		}
		else {
			create_helper<N+1, R...>(std::forward<Args>(args)...);
		}
	}

	template<int N, typename... R, typename... Args>
	std::enable_if_t<(N == sizeof...(R))>
	create_helper(Args&&... args) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Didn't create variant type %i", N);
	}

	template<typename T> enable_unless_void<T>
	destroy_as() {
		data_ref<T>().~T();
	}

	template<typename T> enable_when_void<T>
	destroy_as() {}

	void destroy() {
		destroy_helper<0, T1, T2>();
	}

	template<int N, typename T, typename... R>
	void destroy_helper() {
		if(typenum == N) {
			destroy_as<T>();
		}
		else {
			destroy_helper<N+1, R...>();
		}
	}

	template<int N>
	void destroy_helper() {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Didn't destroy variant type %i", N);
	}

	template<typename T>
	T& data_ref() {
		check_type<T>();
		return *reinterpret_cast<T*>(&data);
	}

	template<typename T>
	void check_type() {
		static_assert(AnyConvertibleTo<T, T1, T2>::value,
		              "Type not valid for this variant");
	}
};

#endif
