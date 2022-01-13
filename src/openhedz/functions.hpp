#pragma once

#include "core/interop/function.hpp"
#include "core/interop/win_min.hpp"

namespace openhedz
{
    inline interop::Function<0x004731A0, void (*)()> initAssetPaths;
    inline interop::Function<0x0046DEC0, bool (*)(HINSTANCE)> initWindow;
    inline interop::Function<0x004478E0, bool (*)(HWND, HINSTANCE)> setupInputDevices;
    inline interop::Function<0x0044DEE0, void (*)()> setupKeyMapping;
    inline interop::Function<0x00473620, void (*)()> setupMapFilePath;
    inline interop::Function<0x0043FBC0, void (*)()> sub_43FBC0;
    inline interop::Function<0x0043F9D0, void (*)()> loadTextureData;
    inline interop::Function<0x0046EB50, DLGPROC> sub_46EB50;
    inline interop::Function<0x00463550, bool (*)()> sub_463550;
    inline interop::Function<0x00470B60, void (*)()> saveSettings;
    inline interop::Function<0x00470A20, void (*)()> setupWindowHook;
    inline interop::Function<0x0046E450, int (*)()> startGame;

} // namespace openhedz