#include "interop.hpp"

#include "hooks.hpp"
#include "win_min.hpp"

#include <cstdint>

namespace openhedz::interop
{
    static uintptr_t _baseAddress = 0;

    bool init()
    {
        auto imageBase = GetModuleHandleA(nullptr);
        _baseAddress = reinterpret_cast<uintptr_t>(imageBase);

        return true;
    }

    uintptr_t getImageBase()
    {
        return _baseAddress;
    }

} // namespace openhedz::interop

// Used to add a new import to the original exe to load this automatically.
extern "C" __declspec(dllexport) void OpenHEDZ()
{
}