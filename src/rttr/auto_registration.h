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
    registration::enumeration<Enum_Type>(enum_name)(enumerators);
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
    constexpr meta::member_fn_range member_funcs(reflexpr(Class_Type));
    template for (constexpr auto func : member_funcs)
    {
        if constexpr (!meta::is_constructor(func) &&
                      !meta::is_destructor(func) &&
                      !meta::is_copy_assignment_operator(func) &&
                      !meta::is_move_assignment_operator(func))
        {
            clazz.method(meta::name_of(func), &Class_Type::unqualid(func));
        }
    }
}

template<typename Class_Type>
void register_methods()
{
    auto clazz = make_registration_class<Class_Type>();
    register_methods(clazz);
}

}
