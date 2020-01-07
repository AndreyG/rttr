#include "registration.h"

#include <experimental/meta>

namespace rttr
{

namespace meta = std::experimental::meta;

template<typename Enum_Type>
struct enumerator_t
{
    const char * name;
    Enum_Type value;
};

template<typename Enum_Type>
consteval auto get_enumerators()
{
    constexpr meta::info enum_reflection = reflexpr(Enum_Type);
    constexpr auto members = meta::members_of(enum_reflection);
    constexpr auto size = std::distance(std::begin(members), std::end(members));
    std::array<enumerator_t<Enum_Type>, size> result;
    size_t i = 0;
    template for (constexpr auto enumerator : meta::members_of(enum_reflection))
    {
        result[i].name = meta::name_of(enumerator);
        result[i].value = valueof(enumerator);
        ++i;
    };
    return result;
}

template<typename Enumeration, typename Enumerators, size_t... I>
void apply(Enumeration enumeration, Enumerators const & enumerators, std::index_sequence<I...>)
{
    enumeration(value(enumerators[I].name, enumerators[I].value)...);
}

template<typename Enum_Type>
void auto_enumeration()
{
    constexpr auto enumerators = get_enumerators<Enum_Type>();
    constexpr auto enum_name = meta::name_of(reflexpr(Enum_Type));
    apply(registration::enumeration<Enum_Type>(enum_name),
          enumerators,
          std::make_index_sequence<enumerators.size()>());
}

}
