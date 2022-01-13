#pragma once

#include "core/interop/variable.hpp"
#include "core/interop/win_min.hpp"

namespace openhedz
{
    inline interop::Var<0x005D66AC, HANDLE> gMutex01;
    inline interop::Var<0x005D66B0, HANDLE> gMutex02;
    inline interop::Var<0x005E4484, HANDLE> gMutex03;
    inline interop::Var<0x005D66B4, HANDLE> gMutex04;

    inline interop::Var<0x005D664C, HANDLE> gEvent01;
    inline interop::Var<0x005D66A0, HANDLE> gEvent02;
    inline interop::Var<0x00598EA0, HANDLE> gEvent03;
    inline interop::Var<0x00598F88, HANDLE> gEvent04;

    inline interop::Var<0x005D6500, HWND> gWnd;

    inline interop::Var<0x005E5140, uint32_t> dword_5E5140;
    inline interop::Var<0x00598FE0, uint32_t> dword_598FE0;
    inline interop::Var<0x00598F04, uint32_t> dword_598F04;
    inline interop::Var<0x005D7840, uint32_t> dword_5D7840;
    inline interop::Var<0x00598944, uint32_t> dword_598944;
    inline interop::Var<0x00598D20, uint32_t> gShouldExit;
    inline interop::Var<0x005DF310, uint8_t> byte_5DF310;

    inline interop::Var<0x005DF310, char[256]> gPathRoot;
    inline interop::Var<0x005D61E0, char[256]> gPathRootAlt;

    inline interop::Var<0x005DC800, uint16_t[254]> gRandValueTable;
    inline interop::Var<0x005D8800, float[4096]> gSinTable;

    inline interop::Var<0x00598D58, uint32_t> ref_598D58;
    inline interop::Var<0x00598D50, uint32_t> gUseFullscreen;

    inline interop::Var<0x00598DD4, HANDLE> gOneTimeSemaphore;
    inline interop::Var<0x00598D18, LARGE_INTEGER> gFrequency;

} // namespace openhedz