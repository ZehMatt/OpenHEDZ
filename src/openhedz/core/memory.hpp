#pragma once

#include "interop/function.hpp"

#include <cstring>
#include <memory>

namespace openhedz::memory
{
    // void *__cdecl malloc(size_t Size)
    inline interop::Function<0x004AD640, void*(__cdecl*)(size_t)> mem_alloc;

    // void __cdecl free(void *Block)
    inline interop::Function<0x004AD4B0, void(__cdecl*)(void*)> mem_free;

    template<typename T> inline T* alloc(size_t count)
    {
        const auto sizeInBytes = sizeof(T) * count;

        auto* buf = mem_alloc(sizeInBytes);
        std::memset(buf, 0, sizeInBytes);

        return reinterpret_cast<T*>(buf);
    }

    template<typename T> inline T* alloc()
    {
        return alloc<T>(1u);
    }

    void dealloc(void* p)
    {
        mem_free(p);
    }

} // namespace openhedz::memory