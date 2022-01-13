#pragma once

#include "debugging.hpp"
#include "logging.hpp"

#include <cstdint>
#include <intrin.h>
#include <string>
#include <type_traits>

namespace openhedz::diagnostics::assertion
{
    namespace logging = diagnostics::logging;

    namespace detail
    {
        template<class T, class EqualTo> struct ComparableImpl
        {
            template<class U, class V> static constexpr auto test(U*) -> decltype(std::declval<U>() == std::declval<V>());
            template<typename, typename> static constexpr auto test(...) -> std::false_type;

            using type = typename std::is_same<bool, decltype(test<T, EqualTo>(0))>::type;
        };

        template<class T, class EqualTo = T> struct Comparable : ComparableImpl<T, EqualTo>::type
        {
        };

        template<class T, class = void> struct IsIteratorType : std::false_type
        {
        };

        template<class T>
        struct IsIteratorType<
            T,
            std::void_t<
                typename std::iterator_traits<T>::difference_type, typename std::iterator_traits<T>::pointer,
                typename std::iterator_traits<T>::reference, typename std::iterator_traits<T>::value_type,
                typename std::iterator_traits<T>::iterator_category>> : std::true_type
        {
        };

        template<class T> constexpr bool IsIteratorTypeV = IsIteratorType<T>::value;

        template<class T, class EqualTo = T> constexpr bool Compareable_v = Comparable<T, EqualTo>::value;

        template<typename A, typename B> static constexpr bool eq(const A& left, const B& right)
        {
            if constexpr (Compareable_v<A, B>)
            {
                return left == right;
            }
            else
            {
                return false;
            }
        }

        template<typename A, typename B> static constexpr bool ge(const A& left, const B& right)
        {
            if constexpr (Compareable_v<A, B>)
            {
                return left >= right;
            }
            else
            {
                return false;
            }
        }

        template<typename A, typename B> static constexpr bool gt(const A& left, const B& right)
        {
            if constexpr (Compareable_v<A, B>)
            {
                return left > right;
            }
            else
            {
                return false;
            }
        }

        template<typename A, typename B> static constexpr bool le(const A& left, const B& right)
        {
            if constexpr (Compareable_v<A, B>)
            {
                return left <= right;
            }
            else
            {
                return false;
            }
        }

        template<typename A, typename B> static constexpr bool lt(const A& left, const B& right)
        {
            if constexpr (Compareable_v<A, B>)
            {
                return left < right;
            }
            else
            {
                return false;
            }
        }

        template<typename T> struct ValueGetter
        {
            static inline std::string get(const T val)
            {
                using T2 = std::decay_t<T>;
                if constexpr (std::is_integral_v<T2>)
                {
                    return std::to_string(val);
                }
                else if constexpr (std::is_enum_v<T2>)
                {
                    return std::to_string(static_cast<std::underlying_type_t<T2>>(val));
                }
                else if constexpr (std::is_null_pointer_v<T2>)
                {
                    return "nullptr";
                }
                else if constexpr (std::is_pointer_v<T2>)
                {
                    if (val == nullptr)
                    {
                        return "nullptr";
                    }
                    else
                    {
                        char temp[12];
                        sprintf_s(temp, "%p", reinterpret_cast<const void*>(val));
                        return temp;
                    }
                }
                else if constexpr (IsIteratorTypeV<T2>)
                {
                    return "<iterator>";
                }
                else
                {
                    return "<object>";
                }
            }
        };

        template<> struct ValueGetter<const char*>
        {
            static inline std::string get(const char* val)
            {
                return val;
            }
        };

        template<size_t N> struct ValueGetter<char[N]>
        {
            static inline std::string get(const char (&val)[N])
            {
                return val;
            }
        };

        template<> struct ValueGetter<std::string>
        {
            static inline std::string get(const std::string& val)
            {
                return val;
            }
        };

        template<typename T> inline auto underlyingValue(T&& val)
        {
            if constexpr (std::is_enum_v<T>)
            {
                return static_cast<std::underlying_type_t<T>>(val);
            }
            else
            {
                return val;
            }
        }

        template<typename A, typename B>
        inline void reportFailure(const A& a, const B& b, const char* cmp, const char* message, const char* file, int line)
        {
            if (message != nullptr)
            {
                logging::err(
                    "%s:%d Assertion failure (%s %s %s) - %s\n", file, line, detail::ValueGetter<decltype(a)>::get(a), cmp,
                    detail::ValueGetter<decltype(b)>::get(b), message);
            }
            else
            {
                logging::err(
                    "%s:%d Assertion failure (%s %s %s)\n", file, line, detail::ValueGetter<decltype(a)>::get(a), cmp,
                    detail::ValueGetter<decltype(b)>::get(b));
            }
        }

    } // namespace detail

#define WITH_LINE_INFO const char *file = __builtin_FILE(), const int line = __builtin_LINE()

    inline void fail(const char* message = nullptr, WITH_LINE_INFO)
    {
        if (message != nullptr)
        {
            logging::err("%s:%d Assertion failure - %s\n", file, line, message);
        }
        else
        {
            logging::err("%s:%d Assertion failure\n", file, line);
        }

        debugging::halt();
    }

    inline void fail(const std::string& message, WITH_LINE_INFO)
    {
        logging::err("%s:%d Assertion failure - %s\n", file, line, message);
        debugging::halt();
    }

    template<typename A, typename B>
    inline constexpr bool eq(const A& left, const B& right, const char* message = nullptr, WITH_LINE_INFO)
    {
        auto a = detail::underlyingValue(left);
        auto b = detail::underlyingValue(right);

        if (detail::eq(a, b))
            return true;

        detail::reportFailure(a, b, "==", message, file, line);

        debugging::halt();

        return false;
    }

    template<typename A, typename B>
    inline constexpr bool neq(const A& left, const B& right, const char* message = nullptr, WITH_LINE_INFO)
    {
        auto a = detail::underlyingValue(left);
        auto b = detail::underlyingValue(right);

        if (!detail::eq(left, right))
            return false;

        detail::reportFailure(a, b, "!=", message, file, line);

        debugging::halt();

        return true;
    }

    template<typename A, typename B>
    inline constexpr bool ge(const A& left, const B& right, const char* message = nullptr, WITH_LINE_INFO)
    {
        auto a = detail::underlyingValue(left);
        auto b = detail::underlyingValue(right);

        if (detail::ge(a, b))
            return true;

        detail::reportFailure(a, b, ">=", message, file, line);

        debugging::halt();

        return false;
    }

    template<typename A, typename B>
    inline constexpr bool gt(const A& left, const B& right, const char* message = nullptr, WITH_LINE_INFO)
    {
        auto a = detail::underlyingValue(left);
        auto b = detail::underlyingValue(right);

        if (detail::gt(a, b))
            return true;

        detail::reportFailure(a, b, ">", message, file, line);

        debugging::halt();

        return false;
    }

    template<typename A, typename B>
    inline constexpr bool le(const A& left, const B& right, const char* message = nullptr, WITH_LINE_INFO)
    {
        auto a = detail::underlyingValue(left);
        auto b = detail::underlyingValue(right);

        if (detail::le(a, b))
            return true;

        detail::reportFailure(a, b, "<=", message, file, line);

        debugging::halt();

        return false;
    }

    template<typename A, typename B>
    inline constexpr bool lt(const A& left, const B& right, const char* message = nullptr, WITH_LINE_INFO)
    {
        auto a = detail::underlyingValue(left);
        auto b = detail::underlyingValue(right);

        if (detail::lt(a, b))
            return true;

        detail::reportFailure(a, b, "<", message, file, line);

        debugging::halt();

        return false;
    }

#undef WITH_LINE_INFO

} // namespace openhedz::diagnostics::assertion
