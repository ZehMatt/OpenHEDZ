#include "logging.hpp"

#include <chrono>
#include <ctime>
#include <iostream>
#include <vector>

#if defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4996) // Secure CRT warnings, don't care.
#endif

#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#endif

// This may look silly but prevents clang-format from changing the include order.
#if defined(_WIN32) && (_MSC_VER >= 1800)
#    include <windows.h>
#endif

#if defined(_WIN32) && (_MSC_VER >= 1800)
#    include <VersionHelpers.h>
#endif

namespace openhedz::diagnostics::logging
{
    using Clock = std::chrono::high_resolution_clock;
    using Timepoint = Clock::time_point;

    static bool globalInit();

    static bool _init = globalInit();
    static bool _vt100 = false;
    static std::unique_ptr<ILogHandle> _globalLog;

    static bool globalInit()
    {
        if (_init)
            return true;

#if defined(_WIN32) && (_MSC_VER >= 1800)
        _vt100 = true;
#endif

        init();
        return true;
    }

#pragma warning(push)
#pragma warning(disable : 4774)
    template<typename... Args> void print(FILE* fp, char const* const fmt, Args&&... args)
    {
        fprintf(fp, fmt, args...);
    }
#pragma warning(pop)

    enum class ColorCode
    {
        Default = 0,       // Returns all attributes to the default state prior to modification
        BoldBright = 1,    // Applies brightness/intensity flag to foreground color
        NoBoldBright = 22, // Removes brightness/intensity flag from foreground color
        Underline = 4,     // Adds underline
        NoUnderline = 24,  // Removes underline
        Negative = 7,      // Swaps foreground and background colors
        Positive = 27,     // Returns foreground/background to normal

        ForegroundBlack = 30,    // Applies non-bold/bright black to foreground
        ForegroundRed = 31,      // Applies non-bold/bright red to foreground
        ForegroundGreen = 32,    // Applies non-bold/bright green to foreground
        ForegroundYellow = 33,   // Applies non-bold/bright yellow to foreground
        ForegroundBlue = 34,     // Applies non-bold/bright blue to foreground
        ForegroundMagenta = 35,  // Applies non-bold/bright magenta to foreground
        ForegroundCyan = 36,     // Applies non-bold/bright cyan to foreground
        ForegroundWhite = 37,    // Applies non-bold/bright white to foreground
        ForegroundExtended = 38, // Applies extended color value to the foreground (see details below)
        ForegroundDefault = 39,  // Applies only the foreground portion of the defaults (see 0)

        BackgroundBlack = 40,    // Applies non-bold/bright black to background
        BackgroundRed = 41,      // Applies non-bold/bright red to background
        BackgroundGreen = 42,    // Applies non-bold/bright green to background
        BackgroundYellow = 43,   // Applies non-bold/bright yellow to background
        BackgroundBlue = 44,     // Applies non-bold/bright blue to background
        BackgroundMagenta = 45,  // Applies non-bold/bright magenta to background
        BackgroundCyan = 46,     // Applies non-bold/bright cyan to background
        BackgroundWhite = 47,    // Applies non-bold/bright white to background
        BackgroundExtended = 48, // Applies extended color value to the background (see details below)
        BackgroundDefault = 49,  // Applies only the background portion of the defaults (see 0)

        ForegroundBrightBlack = 90,   // Applies bold/bright black to foreground
        ForegroundBrightRed = 91,     // Applies bold/bright red to foreground
        ForegroundBrightGreen = 92,   // Applies bold/bright green to foreground
        ForegroundBrightYellow = 93,  // Applies bold/bright yellow to foreground
        ForegroundBrightBlue = 94,    // Applies bold/bright blue to foreground
        ForegroundBrightMagenta = 95, // Applies bold/bright magenta to foreground
        ForegroundBrightCyan = 96,    // Applies bold/bright cyan to foreground
        ForegroundBrightWhite = 97,   // Applies bold/bright white to foreground

        BackgroundBrightBlack = 100,   // Applies bold/bright black to background
        BackgroundBrightRed = 101,     // Applies bold/bright red to background
        BackgroundBrightGreen = 102,   // Applies bold/bright green to background
        BackgroundBrightYellow = 103,  // Applies bold/bright yellow to background
        BackgroundBrightBlue = 104,    // Applies bold/bright blue to background
        BackgroundBrightMagenta = 105, // Applies bold/bright magenta to background
        BackgroundBrightCyan = 106,    // Applies bold/bright cyan to background
        BackgroundBrightWhite = 107,   // Applies bold/bright white to background
    };

    template<typename TOpts> class LogHandle final : public ILogHandle
    {
        using Clock = std::chrono::high_resolution_clock;
        using Timepoint = Clock::time_point;

        FILE* _fp = nullptr;
        Timepoint _start;
        std::vector<ILogSink*> _sinks;

        ColorCode _textColor = ColorCode::ForegroundDefault;
        ColorCode _bgColor = ColorCode::BackgroundDefault;
        bool _freeConsole = false;

    private:
        void CreateConsole()
        {
            if (!AllocConsole())
            {
                // Add some error handling here.
                // You can call GetLastError() to get more info about the error.
                return;
            }

            // std::cout, std::clog, std::cerr, std::cin
            FILE* fDummy;
            freopen_s(&fDummy, "CONOUT$", "w", stdout);
            freopen_s(&fDummy, "CONOUT$", "w", stderr);
            freopen_s(&fDummy, "CONIN$", "r", stdin);
            std::cout.clear();
            std::clog.clear();
            std::cerr.clear();
            std::cin.clear();

            // std::wcout, std::wclog, std::wcerr, std::wcin
            HANDLE hConOut = CreateFileW(
                L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, NULL);
            HANDLE hConIn = CreateFileW(
                L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, NULL);
            SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
            SetStdHandle(STD_ERROR_HANDLE, hConOut);
            SetStdHandle(STD_INPUT_HANDLE, hConIn);
            std::wcout.clear();
            std::wclog.clear();
            std::wcerr.clear();
            std::wcin.clear();

            _freeConsole = true;
        }

    public:
        LogHandle(const std::string_view name = "")
            : _start(Clock::now())
        {
            if (!name.empty())
            {
                std::string fileName(name);
                if (fileName.find_first_of('.') == fileName.npos)
                {
                    fileName += ".txt";
                }
                _fp = _fsopen(fileName.c_str(), "wt", _SH_DENYWR);
            }

            if constexpr (TOpts::Console)
            {
                HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
                if (hOut == nullptr)
                {
                    // No current console.
                    CreateConsole();
                    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
                }

                if (_vt100 && hOut != nullptr)
                {
                    DWORD dwMode = 0;
                    GetConsoleMode(hOut, &dwMode);
                    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                    SetConsoleMode(hOut, dwMode);
                }
            }
        }

        ~LogHandle()
        {
            if (_fp)
            {
                fclose(_fp);
            }
        }

        ILogHandle& setColor(ConsoleColor color) override
        {
            switch (color)
            {
                case ConsoleColor::Default:
                    _textColor = ColorCode::ForegroundDefault;
                    break;
                case ConsoleColor::Black:
                    _textColor = ColorCode::ForegroundBlack;
                    break;
                case ConsoleColor::Red:
                    _textColor = ColorCode::ForegroundRed;
                    break;
                case ConsoleColor::Green:
                    _textColor = ColorCode::ForegroundGreen;
                    break;
                case ConsoleColor::Yellow:
                    _textColor = ColorCode::ForegroundYellow;
                    break;
                case ConsoleColor::Blue:
                    _textColor = ColorCode::ForegroundBlue;
                    break;
                case ConsoleColor::Magenta:
                    _textColor = ColorCode::ForegroundMagenta;
                    break;
                case ConsoleColor::Cyan:
                    _textColor = ColorCode::ForegroundCyan;
                    break;
                case ConsoleColor::White:
                    _textColor = ColorCode::ForegroundWhite;
                    break;
                case ConsoleColor::BrightBlack:
                    _textColor = ColorCode::ForegroundBrightBlack;
                    break;
                case ConsoleColor::BrightRed:
                    _textColor = ColorCode::ForegroundBrightRed;
                    break;
                case ConsoleColor::BrightGreen:
                    _textColor = ColorCode::ForegroundBrightGreen;
                    break;
                case ConsoleColor::BrightYellow:
                    _textColor = ColorCode::ForegroundBrightYellow;
                    break;
                case ConsoleColor::BrightBlue:
                    _textColor = ColorCode::ForegroundBrightBlue;
                    break;
                case ConsoleColor::BrightMagenta:
                    _textColor = ColorCode::ForegroundBrightMagenta;
                    break;
                case ConsoleColor::BrightCyan:
                    _textColor = ColorCode::ForegroundBrightCyan;
                    break;
                case ConsoleColor::BrightWhite:
                    _textColor = ColorCode::ForegroundBrightWhite;
                    break;
                default:
                    break;
            }
            return *this;
        }

        ILogHandle& setBgColor(ConsoleColor color) override
        {
            switch (color)
            {
                case ConsoleColor::Default:
                    _bgColor = ColorCode::BackgroundDefault;
                    break;
                case ConsoleColor::Black:
                    _bgColor = ColorCode::BackgroundBlack;
                    break;
                case ConsoleColor::Red:
                    _bgColor = ColorCode::BackgroundRed;
                    break;
                case ConsoleColor::Green:
                    _bgColor = ColorCode::BackgroundGreen;
                    break;
                case ConsoleColor::Yellow:
                    _bgColor = ColorCode::BackgroundYellow;
                    break;
                case ConsoleColor::Blue:
                    _bgColor = ColorCode::BackgroundBlue;
                    break;
                case ConsoleColor::Magenta:
                    _bgColor = ColorCode::BackgroundMagenta;
                    break;
                case ConsoleColor::Cyan:
                    _bgColor = ColorCode::BackgroundCyan;
                    break;
                case ConsoleColor::White:
                    _bgColor = ColorCode::BackgroundWhite;
                    break;
                case ConsoleColor::BrightBlack:
                    _bgColor = ColorCode::BackgroundBrightBlack;
                    break;
                case ConsoleColor::BrightRed:
                    _bgColor = ColorCode::BackgroundBrightRed;
                    break;
                case ConsoleColor::BrightGreen:
                    _bgColor = ColorCode::BackgroundBrightGreen;
                    break;
                case ConsoleColor::BrightYellow:
                    _bgColor = ColorCode::BackgroundBrightYellow;
                    break;
                case ConsoleColor::BrightBlue:
                    _bgColor = ColorCode::BackgroundBrightBlue;
                    break;
                case ConsoleColor::BrightMagenta:
                    _bgColor = ColorCode::BackgroundBrightMagenta;
                    break;
                case ConsoleColor::BrightCyan:
                    _bgColor = ColorCode::BackgroundBrightCyan;
                    break;
                case ConsoleColor::BrightWhite:
                    _bgColor = ColorCode::BackgroundBrightWhite;
                    break;
                default:
                    break;
            }
            return *this;
        }

        void addSink(ILogSink* sink) override
        {
            _sinks.push_back(sink);
        }

        void removeSink(ILogSink* sink) override
        {
            auto it = std::find(std::begin(_sinks), std::end(_sinks), sink);
            if (it == std::end(_sinks))
                return;

            _sinks.erase(it);
        }

        ILogHandle& printMsg(Detail::MsgType type, const char* txt) override
        {
            char timestamp[32] = { 0 };

            if constexpr (TOpts::Timestamp)
            {
                if (type != Detail::MsgType::Logo)
                {
                    auto now = Clock::now();

                    const uint64_t elapsed = static_cast<uint64_t>(
                        std::chrono::duration_cast<std::chrono::milliseconds>(now - _start).count());
                    const uint64_t secs = elapsed / 1000;
                    const uint64_t ms = elapsed - (secs * 1000);
                    const uint64_t mins = secs / 60;
                    const uint64_t hrs = (mins / 60);

                    sprintf_s(timestamp, "[%02llu:%02llu:%02llu:%03llu] ", hrs, mins % 60, secs % 60, ms);
                }
            }

            for (auto* sink : _sinks)
            {
                sink->printMsg(type, txt);
            }

            if constexpr (TOpts::Console)
            {
                if (_vt100)
                {
                    switch (type)
                    {
                        case Detail::MsgType::Info:
                            break;
                        case Detail::MsgType::Logo:
                            print(stdout, "\x1b[%dm\x1b[%dm%s\x1b[0m", _textColor, _bgColor, txt);
                            break;
                        case Detail::MsgType::Echo:
                            print(stdout, "\x1b[%dm\x1b[%dm%s%s\x1b[0m", _textColor, _bgColor, timestamp, txt);
                            break;
                        case Detail::MsgType::Highlight:
                            print(stdout, "\x1b[97m%s%s\x1b[0m", timestamp, txt);
                            break;
                        case Detail::MsgType::Error:
                            print(stderr, "\x1b[31m%s%s\x1b[0m", timestamp, txt);
                            break;
                        case Detail::MsgType::Warning:
                            print(stdout, "\x1b[33m%s%s\x1b[0m", timestamp, txt);
                            break;
                    }
                }
                else
                {
                    switch (type)
                    {
                        case Detail::MsgType::Info:
                            break;
                        case Detail::MsgType::Logo:
                            print(stdout, "%s", timestamp, txt);
                            break;
                        case Detail::MsgType::Echo:
                        case Detail::MsgType::Highlight:
                            print(stdout, "%s%s", timestamp, txt);
                            break;
                        case Detail::MsgType::Error:
                            print(stderr, "%s%s", timestamp, txt);
                            break;
                        case Detail::MsgType::Warning:
                            print(stdout, "%s%s", timestamp, txt);
                            break;
                    }
                }
            }

            if (_fp != nullptr)
            {
                print(_fp, "%s%s", timestamp, txt);
            }

            return *this;
        }
    };

    ILogHandle& get()
    {
        if (!_globalLog)
        {
            init();
        }
        return *_globalLog;
    }

    static std::unique_ptr<ILogHandle> createImpl(std::string_view name, Options opts)
    {
        if (opts.Console && opts.Timestamp)
            return std::make_unique<LogHandle<Detail::Options<true, true>>>(name);
        else if (!opts.Console && opts.Timestamp)
            return std::make_unique<LogHandle<Detail::Options<false, true>>>(name);
        else if (opts.Console && !opts.Timestamp)
            return std::make_unique<LogHandle<Detail::Options<true, false>>>(name);
        return std::make_unique<LogHandle<Detail::Options<false, false>>>(name);
    }

    void init(std::string_view name, Options opts)
    {
        _globalLog = createImpl(name, opts);
    }

    void init(Options opts)
    {
        _globalLog = createImpl("", opts);
    }

    std::unique_ptr<ILogHandle> create(std::string_view name, Options opts)
    {
        return createImpl(name, opts);
    }

} // namespace openhedz::diagnostics::logging

#if defined(_MSC_VER)
#    pragma warning(pop)
#endif
