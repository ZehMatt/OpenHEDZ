#pragma once

#include <cstdint>
#include <utility>

namespace openhedz::interop
{
    template<uintptr_t TAddr, typename TDecl> class Function
    {
        const TDecl _f = reinterpret_cast<TDecl>(TAddr);

    public:
        template<typename... TArgs> auto operator()(TArgs&&... args) const
        {
            return _f(std::forward<TArgs&&>(args)...);
        }

        TDecl get() const
        {
            return _f;
        }
    };

} // namespace openhedz::interop