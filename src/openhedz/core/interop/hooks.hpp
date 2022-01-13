#pragma once

#include <cstdint>

namespace openhedz::interop::hooks
{
    bool init();

    void add(const struct HookEntry& entry);

    struct HookEntry
    {
        intptr_t source;
        void* target;
        const char* name;

        HookEntry(intptr_t src, void* dst, const char* n)
            : source{ src }
            , target{ dst }
            , name{ n }
        {
            add(*this);
        }
    };

#define HOOK_FUNCTION(src, dst)                                                                                                \
    inline openhedz::interop::hooks::HookEntry s_HOOK_##src##_##dst(src, dst, #dst);                                           \
    extern "C" __declspec(dllexport) openhedz::interop::hooks::HookEntry* HOOK_##src##_##dst = &s_HOOK_##src##_##dst;

} // namespace openhedz::interop::hooks