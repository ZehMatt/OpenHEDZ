#pragma once

namespace openhedz::diagnostics::debugging
{
    // Triggers a breakpoint exception, it is safe to continue execution after this in the debugger.
    void halt();

} // namespace openhedz::diagnostics::debugging
