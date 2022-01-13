#include "logging.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#endif

// This may look silly but prevents clang-format from changing the include order.
#if defined(_WIN32) && (_MSC_VER >= 1800)
#    include <intrin.h>
#    include <windows.h>
#endif

namespace openhedz::diagnostics::debugging
{
    void halt()
    {
#ifdef _MSC_VER
        __debugbreak();
#endif
    }

} // namespace openhedz::diagnostics::debugging
