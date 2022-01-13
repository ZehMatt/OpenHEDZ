#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace openhedz::diagnostics::logging
{
    namespace Detail
    {
        enum class MsgType
        {
            Info = 0,
            Warning,
            Error,
            Echo,
            Logo,
            Highlight,
        };

        template<typename T> inline auto argumentHandler(const T& v)
        {
            return v;
        }

        template<> inline auto argumentHandler<std::string>(const std::string& v)
        {
            return v.c_str();
        }

        template<> inline auto argumentHandler<std::wstring>(const std::wstring& v)
        {
            return v.c_str();
        }

        template<> inline auto argumentHandler<std::string_view>(const std::string_view& v)
        {
            return v.data();
        }

        template<bool TConsole, bool TTimestamp> struct Options
        {
            static constexpr bool Console = TConsole;
            static constexpr bool Timestamp = TTimestamp;
        };

    } // namespace Detail

    enum class ConsoleColor
    {
        Default,
        Black,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White,
        BrightBlack,
        BrightRed,
        BrightGreen,
        BrightYellow,
        BrightBlue,
        BrightMagenta,
        BrightCyan,
        BrightWhite,
    };

    struct Options
    {
        bool Console;
        bool Timestamp;
    };

    class ILogSink
    {
    public:
        virtual ~ILogSink() = default;

        virtual void printMsg(Detail::MsgType type, const char* txt) = 0;
    };

    class ILogHandle
    {
    public:
        virtual ~ILogHandle() = default;

        virtual ILogHandle& printMsg(Detail::MsgType type, const char* txt) = 0;

        virtual ILogHandle& setColor(ConsoleColor color) = 0;
        virtual ILogHandle& setBgColor(ConsoleColor color) = 0;

        virtual void addSink(ILogSink* sink) = 0;
        virtual void removeSink(ILogSink* sink) = 0;

        template<typename... Args> ILogHandle& info(const char* fmt, Args&&... args)
        {
            formatMsg(Detail::MsgType::Info, fmt, std::forward<Args&&>(args)...);
            return *this;
        }

        template<typename... Args> ILogHandle& warn(const char* fmt, Args&&... args)
        {
            formatMsg(Detail::MsgType::Warning, fmt, std::forward<Args&&>(args)...);
            return *this;
        }

        template<typename... Args> ILogHandle& err(const char* fmt, Args&&... args)
        {
            formatMsg(Detail::MsgType::Error, fmt, std::forward<Args&&>(args)...);
            return *this;
        }

        template<typename... Args> ILogHandle& echo(const char* fmt, Args&&... args)
        {
            formatMsg(Detail::MsgType::Echo, fmt, std::forward<Args&&>(args)...);
            return *this;
        }

        template<typename... Args> ILogHandle& logo(const char* fmt, Args&&... args)
        {
            formatMsg(Detail::MsgType::Logo, fmt, std::forward<Args&&>(args)...);
            return *this;
        }

        template<typename... Args> ILogHandle& highlight(const char* fmt, Args&&... args)
        {
            formatMsg(Detail::MsgType::Highlight, fmt, std::forward<Args&&>(args)...);
            return *this;
        }

        template<typename... Args> bool guard(bool f, const char* expr, const char* file, int line)
        {
            if (!f)
                formatMsg(Detail::MsgType::Error, "Guard (%s:%d): %s\n", file, line, expr);
            return f;
        }

        template<bool Cond, typename... Args> ILogHandle& cinfo(const char* fmt, Args&&... args)
        {
            if constexpr (Cond)
                formatMsg(Detail::MsgType::Info, fmt, std::forward<Args&&>(args)...);
            return *this;
        }

        template<bool Cond, typename... Args> ILogHandle& cwarn(const char* fmt, Args&&... args)
        {
            if constexpr (Cond)
                formatMsg(Detail::MsgType::Warning, fmt, std::forward<Args&&>(args)...);
            return *this;
        }

        template<bool Cond, typename... Args> ILogHandle& cerr(const char* fmt, Args&&... args)
        {
            if constexpr (Cond)
                formatMsg(Detail::MsgType::Error, fmt, std::forward<Args&&>(args)...);
            return *this;
        }

        template<bool Cond, typename... Args> ILogHandle& cecho(const char* fmt, Args&&... args)
        {
            if constexpr (Cond)
                formatMsg(Detail::MsgType::Echo, fmt, std::forward<Args&&>(args)...);
            return *this;
        }

#pragma warning(push)
#pragma warning(disable : 4774)
        template<typename... Args> ILogHandle& formatMsg(Detail::MsgType type, const char* fmt, Args&&... args)
        {
            char buffer[128];
            // Use local buffer first, for 90% of the time this is enough space.
            int neededLength = snprintf(buffer, sizeof(buffer), fmt, Detail::argumentHandler(args)...);
            if (neededLength >= sizeof(buffer))
            {
                auto actualLength = static_cast<size_t>(neededLength);
                auto bufferSize = actualLength + 1;
                // snprintf_s returns the size of the completely formatted string, if bigger than buffer
                // it was truncated allocate then temporary buffer.
                std::unique_ptr<char[]> tempBuf(new char[bufferSize]);
                _snprintf_s(tempBuf.get(), bufferSize, actualLength, fmt, Detail::argumentHandler(args)...);
                tempBuf[actualLength] = '\0';
                return printMsg(type, tempBuf.get());
            }
            return printMsg(type, buffer);
        }
#pragma warning(pop)
    };

    void init(Options opts = { true, false });
    void init(std::string_view name, Options opts);

    // Create new log.
    std::unique_ptr<ILogHandle> create(std::string_view name, Options opts);

    // Global log handle.
    ILogHandle& get();

    inline ILogHandle& setBgColor(ConsoleColor color)
    {
        return get().setBgColor(color);
    }

    inline ILogHandle& setColor(ConsoleColor color)
    {
        return get().setColor(color);
    }

    template<typename... TArgs> ILogHandle& info(const char* fmt, TArgs&&... args)
    {
        return get().info(fmt, std::forward<TArgs&&>(args)...);
    }

    template<typename... TArgs> ILogHandle& warn(const char* fmt, TArgs&&... args)
    {
        return get().warn(fmt, std::forward<TArgs&&>(args)...);
    }

    template<typename... TArgs> ILogHandle& err(const char* fmt, TArgs&&... args)
    {
        return get().err(fmt, std::forward<TArgs&&>(args)...);
    }

    template<typename... TArgs> ILogHandle& echo(const char* fmt, TArgs&&... args)
    {
        return get().echo(fmt, std::forward<TArgs&&>(args)...);
    }

    template<typename... TArgs> ILogHandle& logo(const char* fmt, TArgs&&... args)
    {
        return get().logo(fmt, std::forward<TArgs&&>(args)...);
    }

    template<typename... TArgs> ILogHandle& highlight(const char* fmt, TArgs&&... args)
    {
        get().highlight(fmt, std::forward<TArgs&&>(args)...);
    }

    template<typename... TArgs> bool guard(bool f, const char* expr, const char* file, int line)
    {
        return get().guard(f, expr, file, line);
    }

    template<bool Cond, typename... TArgs> ILogHandle& cinfo(const char* fmt, TArgs&&... args)
    {
        return get().cinfo<Cond>(fmt, std::forward<TArgs&&>(args)...);
    }

    template<bool Cond, typename... TArgs> ILogHandle& cwarn(const char* fmt, TArgs&&... args)
    {
        return get().cwarn<Cond>(fmt, std::forward<TArgs&&>(args)...);
    }

    template<bool Cond, typename... TArgs> ILogHandle& cerr(const char* fmt, TArgs&&... args)
    {
        return get().cerr<Cond>(fmt, std::forward<TArgs&&>(args)...);
    }

    template<bool Cond, typename... TArgs> ILogHandle& cecho(const char* fmt, TArgs&&... args)
    {
        return get().cecho<Cond>(fmt, std::forward<TArgs&&>(args)...);
    }

} // namespace openhedz::diagnostics::logging
