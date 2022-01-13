#include "hooks.hpp"

#include "../diagnostics/logging.hpp"
#include "win_min.hpp"

#include <vector>

namespace openhedz::interop::hooks
{
    namespace logging = diagnostics::logging;

    using HookRegistry = std::vector<const HookEntry*>;

    HookRegistry& getRegistry()
    {
        static std::vector<const HookEntry*> reg;
        return reg;
    }

    static bool applyHook(const HookEntry* hook)
    {
        intptr_t targetVA = reinterpret_cast<intptr_t>(hook->target);

        uint8_t jmpRel32[5]{};
        jmpRel32[0] = 0xE9;

        int32_t rel32 = static_cast<int32_t>(targetVA - hook->source - 5);
        std::memcpy(jmpRel32 + 1, &rel32, sizeof(rel32));

        void* pSource = reinterpret_cast<void*>(hook->source);

        SIZE_T bytesWritten = 0;
        if (WriteProcessMemory(GetCurrentProcess(), pSource, jmpRel32, sizeof(jmpRel32), &bytesWritten) == FALSE)
        {
            logging::err("Failed to write bytes at %p\n", pSource);
            return false;
        }

        logging::echo("Hook \"%s\" applied at %p\n", hook->name, pSource);

        return true;
    }

    static bool applyHooks()
    {
        auto& registry = getRegistry();
        for (auto* hook : registry)
        {
            if (!applyHook(hook))
            {
                logging::err("Unable to hook %p\n", (void*)hook->source);
                return false;
            }
        }
        return true;
    }

    bool init()
    {
        if (!applyHooks())
            return false;

        return true;
    }

    void add(const HookEntry& entry)
    {
        auto& reg = getRegistry();
        reg.push_back(&entry);
    }

} // namespace openhedz::interop::hooks