#include <openhedz/core/diagnostics/logging.hpp>
#include <openhedz/core/interop/hooks.hpp>
#include <openhedz/core/interop/interop.hpp>
#include <openhedz/core/interop/win_min.hpp>

using namespace openhedz;

namespace logging = diagnostics::logging;

BOOL WINAPI DllMain(void* _DllHandle, unsigned long _Reason, void* _Reserved)
{
    if (_Reason == DLL_PROCESS_ATTACH)
    {
        logging::init("openhedz.log", { true, false });

        interop::init();
        interop::hooks::init();

        logging::echo("Initialized\n");
    }

    return TRUE;
}