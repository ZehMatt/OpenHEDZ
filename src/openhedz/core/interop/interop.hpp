#pragma once

#include "function.hpp"
#include "hooks.hpp"
#include "variable.hpp"
#include "win_min.hpp"

#include <cstdint>

namespace openhedz::interop
{
    bool init();

    uintptr_t getImageBase();

} // namespace openhedz::interop