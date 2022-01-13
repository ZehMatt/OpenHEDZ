#pragma once

#include <cstdint>

namespace openhedz::interop
{

    template<uintptr_t TAddr, typename T> class Var
    {
    public:
        using type = T;
        using pointer = T*;
        using const_pointer = T*;
        using reference = T&;
        using const_reference = const T&;

    private:
        reference _value = *reinterpret_cast<T*>(TAddr);

    public:
        reference operator=(const Var& other)
        {
            _value = *other;
            return _value;
        }

        reference operator=(const_reference other)
        {
            _value = other;
            return _value;
        }

        operator type() const
        {
            return _value;
        }

        reference operator*()
        {
            return _value;
        }

        const_reference operator*() const
        {
            return _value;
        }

        pointer operator->()
        {
            return &_value;
        }

        const_pointer operator->() const
        {
            return &_value;
        }

        pointer get()
        {
            return &_value;
        }

        const_pointer get() const
        {
            return &_value;
        }
    };

    template<uintptr_t TAddr, typename T, size_t TCount> class Var<TAddr, T[TCount]>
    {
    public:
        using type = T[TCount];
        using pointer = T*;
        using const_pointer = const pointer;
        using reference = type&;
        using const_reference = const reference;

    private:
        using TArrayType = T[TCount];

        TArrayType& _data = *reinterpret_cast<TArrayType*>(TAddr);

    public:
        T& operator[](size_t index)
        {
            return get()[index];
        }

        const T& operator[](size_t index) const
        {
            return get()[index];
        }

        pointer get()
        {
            return _data;
        }

        const_pointer get() const
        {
            return _data;
        }

        size_t size() const noexcept
        {
            return TCount;
        }
    };

} // namespace openhedz::interop