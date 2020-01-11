#include "registration.h"

#include <experimental/meta>

namespace rttr
{

namespace meta = std::experimental::meta;

template<typename Enum_Type>
consteval auto get_enumerators()
{
    constexpr meta::info enum_reflection = reflexpr(Enum_Type);
    constexpr auto members = meta::members_of(enum_reflection);
    constexpr auto size = std::distance(std::begin(members), std::end(members));
    std::array<detail::enum_data<Enum_Type>, size> result;
    size_t i = 0;
    template for (constexpr auto enumerator : meta::members_of(enum_reflection))
    {
        result[i++] = { meta::name_of(enumerator), valueof(enumerator) };
    };
    return result;
}

template<typename Enum_Type>
void auto_enumeration()
{
    constexpr auto enumerators = get_enumerators<Enum_Type>();
    constexpr auto enum_name = meta::name_of(reflexpr(Enum_Type));
    std::apply(registration::enumeration<Enum_Type>(enum_name),
               std::array /*create copy as temporary workaround for rttr bug*/ (enumerators));
}

template<typename MemberFuncPtr>
struct method_t
{
    const char * name;
    MemberFuncPtr pointer;
};

template<typename Class_Type, meta::info method>
consteval auto make_method() {
    constexpr auto pointer = &Class_Type::unqualid(method);
    return method_t<decltype(pointer)> { meta::name_of(method), pointer };
}

consteval bool is_method(meta::info func)
{
    return !meta::is_constructor(func) &&
           !meta::is_destructor(func) &&
           !meta::is_special_member_function(func);
}

consteval size_t methods_count(meta::info class_reflection)
{
    size_t result = 0;
    for (meta::member_fn_iterator it(class_reflection); !meta::is_invalid(*it); ++it)
    {
        if (!is_method(*it))
            continue;
        ++result;
    }
    return result;
}

template<size_t I>
consteval meta::info get_method(meta::member_fn_iterator it)
{
    for (size_t i = 0; ; ++it)
    {
        if (!is_method(*it))
            continue;
        if (i == I)
            return *it;
        ++i;
    }
}

template<typename Class_Type, size_t... I>
consteval auto get_methods_impl(std::index_sequence<I...>)
{
    constexpr meta::member_fn_iterator it(reflexpr(Class_Type));
    return std::tuple(make_method<Class_Type, get_method<I>(it)>()...);
}

template<typename Class_Type>
consteval auto get_methods()
{
    return get_methods_impl<Class_Type>(std::make_index_sequence<methods_count(reflexpr(Class_Type))>());
}

template<typename Class_Type>
registration::class_<Class_Type> make_registration_class()
{
    constexpr auto class_reflection = reflexpr(Class_Type);
    constexpr auto class_name = meta::name_of(class_reflection);
    return registration::class_<Class_Type>(class_name);
}

template<typename Class_Type>
void register_methods(registration::class_<Class_Type> & clazz)
{
    template for (auto method : get_methods<Class_Type>())
    {
        clazz.method(method.name, method.pointer);
    }
}

template<typename Class_Type>
void register_methods()
{
    auto clazz = make_registration_class<Class_Type>();
    register_methods(clazz);
}

}
