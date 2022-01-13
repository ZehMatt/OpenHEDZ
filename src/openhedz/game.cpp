#include "game.hpp"

#include "core/diagnostics/logging.hpp"
#include "core/interop/interop.hpp"
#include "functions.hpp"
#include "globals.hpp"

#include <array>
#include <varargs.h>

namespace openhedz
{
    namespace logging = diagnostics::logging;

    // 00413D80
    int logMessage(const char* fmt, ...)
    {
        va_list arglist;
        va_start(arglist, fmt);

        char buffer[512]{};
        int res = vsnprintf_s(buffer, sizeof(buffer), fmt, arglist);
        if (res >= sizeof(buffer))
        {
            auto tempBuf = std::make_unique<char[]>(res + 1);
            res = vsnprintf_s(tempBuf.get(), res + 1, res, fmt, arglist);

            logging::warn("%s", tempBuf.get());
        }
        else
        {
            logging::warn("%s", buffer);
        }
        return res;
    }
    HOOK_FUNCTION(0x00413D80, logMessage);

    // 0x004858F0
    int waitForEvent03(int arg1)
    {
        WaitForSingleObject(gEvent03, INFINITE);
        dword_598F04 = arg1;
        return SetEvent(gEvent03);
    }
    HOOK_FUNCTION(0x004858F0, waitForEvent03);

    // 0x0048DAA0
    int waitForEvent04(int arg1)
    {
        WaitForSingleObject(gEvent04, INFINITE);
        dword_598FE0 = arg1;
        return SetEvent(gEvent04);
    }
    HOOK_FUNCTION(0x0048DAA0, waitForEvent04);

    static void waitForDebugger()
    {
        auto* cmdLine = GetCommandLineA();
        if (strstr(cmdLine, "-debug") != nullptr)
        {
            if (!IsDebuggerPresent())
            {
                logging::echo("Waiting for debugger\n");

                while (!IsDebuggerPresent())
                {
                    Sleep(1000);
                }
                __debugbreak();
            }
        }
    }

    // 0x0045E9C0
    void initRand()
    {
        for (size_t i = 0; i < std::size(gRandValueTable); i++)
        {
            gRandValueTable[i] = rand();
        }
    }
    HOOK_FUNCTION(0x0045E9C0, initRand);

    // 0x0045EA10
    void initSinTable()
    {
        for (size_t i = 0; i < std::size(gSinTable); i++)
        {
            gSinTable[i] = static_cast<float>(sin(static_cast<double>(i) * 0.0015707964));
        }
    }
    HOOK_FUNCTION(0x0045EA10, initSinTable);

    // 0x0046DA60
    int WINAPI entrypoint(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
    {
        waitForDebugger();

        logging::echo("OpenHEDZ Startup\n");

        std::memset(dword_5E5140.get(), 0, 0x2560u);

        // TODO: Remove once no longer required.
        gOneTimeSemaphore = CreateSemaphoreA(0, 0, 1, "HEDZ_ONETIME");
        if (gOneTimeSemaphore && GetLastError() == ERROR_ALREADY_EXISTS)
        {
            CloseHandle(gOneTimeSemaphore);
            return 0;
        }

        gMutex01 = CreateMutexA(0, 0, 0);
        gMutex02 = CreateMutexA(0, 0, 0);
        gMutex03 = CreateMutexA(0, 0, 0);
        gMutex04 = CreateMutexA(0, 0, 0);
        QueryPerformanceFrequency(gFrequency.get());

        initRand();
        initSinTable();
        initAssetPaths();

        // Events
        {
            gEvent01 = CreateEventA(nullptr, FALSE, TRUE, nullptr);
            if (gEvent01 == nullptr)
                return EXIT_FAILURE;

            gEvent02 = CreateEventA(nullptr, FALSE, TRUE, nullptr);
            if (gEvent02 == nullptr)
                return EXIT_FAILURE;

            gEvent03 = CreateEventA(nullptr, FALSE, TRUE, nullptr);
            if (gEvent03 == nullptr)
                return EXIT_FAILURE;

            gEvent04 = CreateEventA(nullptr, FALSE, TRUE, nullptr);
            if (gEvent04 == nullptr)
                return EXIT_FAILURE;

            waitForEvent04(0);
            waitForEvent03(0);
        }

        // Fonts.
        {
            char fontsPath[256]{};
            if (byte_5DF310)
                strcpy_s(fontsPath, gPathRoot.get());
            else
                strcpy_s(fontsPath, gPathRootAlt.get());

            strcat_s(fontsPath, "hdzfont");

            int newFonts = AddFontResourceA(fontsPath);
            if (newFonts == 0)
            {
                logging::warn("Unable to load font resources\n");
            }
        }

        ref_598D58 = 1;
        gUseFullscreen = 1;

        if (!initWindow(hInstance))
            return EXIT_FAILURE;

        if (!setupInputDevices(gWnd, hInstance))
            return EXIT_FAILURE;

        setupKeyMapping();
        setupMapFilePath();
        sub_43FBC0();
        loadTextureData();

        // Config window
        for (;;)
        {
            if (DialogBoxParamA(hInstance, (LPCSTR)0x74, nullptr, sub_46EB50.get(), 0))
            {
                break;
            }
            else
            {
                memset(dword_5D7840.get(), 0, 0x240u);
                if (sub_463550())
                {
                    *reinterpret_cast<uint8_t*>(dword_598944.get()) = 1;
                    break;
                }
            }
        }

        saveSettings();

        if (gShouldExit == 1u)
            return EXIT_SUCCESS;

        setupWindowHook();

        if (!startGame())
            return EXIT_SUCCESS;

        {
            MSG msg{};

            auto accelerators = LoadAcceleratorsA(hInstance, "AppAccel");
            while (!gShouldExit)
            {
                if (PeekMessageA(&msg, 0, 0, 0, 1u))
                {
                    if (!gWnd || !TranslateAcceleratorA(gWnd, accelerators, &msg))
                    {
                        TranslateMessage(&msg);
                        DispatchMessageA(&msg);
                    }
                }
                else
                {
                    WaitMessage();
                }
            }
        }

        DestroyWindow(gWnd);
        CloseHandle(gOneTimeSemaphore);

        return EXIT_SUCCESS;
    }
    HOOK_FUNCTION(0x0046DA60, entrypoint);

} // namespace openhedz