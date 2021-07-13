#ifndef STYLER_HPP
#define STYLER_HPP

#if defined(__unix__) || defined(__unix) || defined(__linux__)
#define STYLER_LINUX
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#define STYLER_WINDOWS
#elif defined(__APPLE__) || defined(__MACH__)
#define STYLER_MACOSX
#else
#error Unknown Platform
#endif

#if defined(STYLER_LINUX) || defined(STYLER_MACOSX)
#include <unistd.h>
#elif defined(STYLER_WINDOWS)
#define NOMINMAX
#include <Windows.h>
#include <io.h>
#endif

#include <algorithm>
#include <atomic>
#include <cstring>
#include <iostream>
#include <type_traits>

namespace styler
{
enum class Style
{
    Reset = 0,
    Bold = 1,
    Faint = 2,
    Italic = 3,
    Underline = 4,
    SlowBlink = 5,
    RapidBlink = 6,
    Reverse = 7,
    Conceal = 8,
    Crossed = 9
};

enum class Foreground
{
    Black = 30,
    Red = 31,
    Green = 32,
    Yellow = 33,
    Blue = 34,
    Magenta = 35,
    Cyan = 36,
    White = 37,
    Reset = 39
};

enum class Background
{
    Black = 40,
    Red = 41,
    Green = 42,
    Yellow = 43,
    Blue = 44,
    Magenta = 45,
    Cyan = 46,
    White = 47,
    Reset = 49
};

enum class ForegroundHI
{
    Black = 90,
    Red = 91,
    Green = 92,
    Yellow = 93,
    Blue = 94,
    Magenta = 95,
    Cyan = 96,
    White = 97
};

enum class BackgroundHI
{
    Black = 100,
    Red = 101,
    Green = 102,
    Yellow = 103,
    Blue = 104,
    Magenta = 105,
    Cyan = 106,
    White = 107
};

//! Behavior of styler function calls.
enum class Control
{
    Off,   //! Toggle off rang style/color calls
    Auto,  //! (Default) auto-detect terminal and colorize if needed
    Force  //! Force ansi color output to non terminal streams
};

//! Terminal mode of Windows.
enum class WinTerm
{
    Auto,   // (Default) Automatically detects whether ANSI or Native API
    Ansi,   // Force use ANSI API
    Native  // Force use Native API
};

namespace internal
{
inline std::atomic<Control>& GetControlMode() noexcept
{
    static std::atomic<Control> controlMode{ Control::Auto };
    return controlMode;
}

inline std::atomic<WinTerm>& GetWinTermMode() noexcept
{
    static std::atomic<WinTerm> termMode{ WinTerm::Auto };
    return termMode;
}

inline bool IsSupportColor() noexcept
{
#if defined(STYLER_LINUX) || defined(STYLER_MACOSX)
    const char* terms[] = { "ansi",    "color",  "console", "cygwin", "gnome",
                            "konsole", "kterm",  "linux",   "msys",   "putty",
                            "rxvt",    "screen", "vt100",   "xterm" };

    const char* envParam = std::getenv("TERM");
    if (!envParam)
    {
        return false;
    }

    return std::any_of(std::begin(terms), std::end(terms),
                       [&](const char* term) {
                           return std::strstr(envParam, term) != nullptr;
                       });
#elif defined(STYLER_WINDOWS)
    // All windows versions support colors through native console methods
    return true;
#endif
}

#if defined(STYLER_WINDOWS)
inline bool IsMSYSPty(int fd) noexcept
{
    // Dynamic load for binary compatibility with old Windows
    const auto ptrFileInfo =
        reinterpret_cast<decltype(&GetFileInformationByHandleEx)>(
            GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")),
                           "GetFileInformationByHandleEx"));
    if (!ptrFileInfo)
    {
        return false;
    }

    const auto handle = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
    if (handle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    // Check that it's a pipe:
    if (GetFileType(handle) != FILE_TYPE_PIPE)
    {
        return false;
    }

    // POD type is binary compatible with FILE_NAME_INFO from WinBase.h
    // It have the same alignment and used to avoid UB in caller code
    struct MY_FILE_NAME_INFO
    {
        DWORD FileNameLength;
        WCHAR FileName[MAX_PATH];
    };

    auto pNameInfo = std::make_unique<MY_FILE_NAME_INFO>();
    if (!pNameInfo)
    {
        return false;
    }

    // Check pipe name is template of
    // {"cygwin-","msys-"}XXXXXXXXXXXXXXX-ptyX-XX
    if (!ptrFileInfo(handle, FileNameInfo, pNameInfo.get(),
                     sizeof(MY_FILE_NAME_INFO)))
    {
        return false;
    }

    const std::wstring name(pNameInfo->FileName,
                            pNameInfo->FileNameLength / sizeof(WCHAR));
    if ((name.find(L"msys-") == std::wstring::npos &&
         name.find(L"cygwin-") == std::wstring::npos) ||
        name.find(L"-pty") == std::wstring::npos)
    {
        return false;
    }

    return true;
}
#endif

inline bool IsTerminal(const std::streambuf* osbuf) noexcept
{
#if defined(STYLER_LINUX) || defined(STYLER_MACOSX)
    if (osbuf == std::cout.rdbuf())
    {
        const static bool coutTerm = isatty(fileno(stdout)) != 0;
        return coutTerm;
    }

    if (osbuf == std::cerr.rdbuf() || osbuf == std::clog.rdbuf())
    {
        const static bool cerrTerm = isatty(fileno(stderr)) != 0;
        return cerrTerm;
    }
#elif defined(STYLER_WINDOWS)
    if (osbuf == std::cout.rdbuf())
    {
        const static bool coutTerm =
            _isatty(_fileno(stdout)) || IsMSYSPty(_fileno(stdout));
        return coutTerm;
    }

    if (osbuf == std::cerr.rdbuf() || osbuf == std::clog.rdbuf())
    {
        const static bool cerrTerm =
            _isatty(_fileno(stderr)) || IsMSYSPty(_fileno(stderr));
        return cerrTerm;
    }
#endif

    return false;
}

template <typename T>
using IsValid =
    typename std::enable_if<std::is_same<T, Style>::value ||
                                std::is_same<T, Foreground>::value ||
                                std::is_same<T, Background>::value ||
                                std::is_same<T, ForegroundHI>::value ||
                                std::is_same<T, BackgroundHI>::value,
                            std::ostream&>::type;

#if defined(STYLER_WINDOWS)
// Select Graphic Rendition parameters for Windows console
struct SGR
{
    BYTE foregroundColor;  // Foreground color (0-15) lower 3 rgb bits
                           // + intense bit
    BYTE backgroundColor;  // Background color (0-15) lower 3 rgb bits
                           // + intense bit
    BYTE bold;             // Emulated as FOREGROUND_INTENSITY bit
    BYTE underline;        // Emulated as BACKGROUND_INTENSITY bit
    BOOLEAN reverse;       // Swap foreground/bold & background/underline
    BOOLEAN conceal;       // Set foreground/bold to background/underline
};

// Color attributes for console screen buffer
enum class AttrColor : BYTE
{
    Black = 0,
    Red = 4,
    Green = 2,
    Yellow = 6,
    Blue = 1,
    Magenta = 5,
    Cyan = 3,
    White = 7
};

inline HANDLE GetConsoleHandle(const std::streambuf* osbuf) noexcept
{
    if (osbuf == std::cout.rdbuf())
    {
        static HANDLE handleStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        return handleStdOut;
    }

    if (osbuf == std::cerr.rdbuf() || osbuf == std::clog.rdbuf())
    {
        static HANDLE handleStdErr = GetStdHandle(STD_ERROR_HANDLE);
        return handleStdErr;
    }

    return INVALID_HANDLE_VALUE;
}

inline bool SetWinTermANSIColors(const std::streambuf* osbuf) noexcept
{
    HANDLE handle = GetConsoleHandle(osbuf);
    if (handle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(handle, &dwMode))
    {
        return false;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(handle, dwMode))
    {
        return false;
    }

    return true;
}

inline bool IsSupportANSI(const std::streambuf* osbuf) noexcept
{
    if (osbuf == std::cout.rdbuf())
    {
        const static bool coutANSI =
            IsMSYSPty(_fileno(stdout)) || SetWinTermANSIColors(osbuf);
        return coutANSI;
    }

    if (osbuf == std::cerr.rdbuf() || osbuf == std::clog.rdbuf())
    {
        const static bool cerrANSI =
            IsMSYSPty(_fileno(stderr)) || SetWinTermANSIColors(osbuf);
        return cerrANSI;
    }

    return false;
}

inline const SGR& GetDefaultState() noexcept
{
    const static SGR defaultSGR = []() -> SGR {
        CONSOLE_SCREEN_BUFFER_INFO info;
        WORD attrib = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
                                       &info) ||
            GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &info))
        {
            attrib = info.wAttributes;
        }

        SGR sgr = { 0, 0, 0, 0, FALSE, FALSE };
        sgr.foregroundColor = attrib & 0x0F;
        sgr.backgroundColor = (attrib & 0xF0) >> 4;
        return sgr;
    }();

    return defaultSGR;
}

inline SGR& GetCurrentState() noexcept
{
    static SGR state = GetDefaultState();
    return state;
}

inline BYTE ANSI2Attr(BYTE rgb) noexcept
{
    static const AttrColor rev[8] = { AttrColor::Black, AttrColor::Red,
                                      AttrColor::Green, AttrColor::Yellow,
                                      AttrColor::Blue,  AttrColor::Magenta,
                                      AttrColor::Cyan,  AttrColor::White };
    return static_cast<BYTE>(rev[rgb]);
}

inline WORD SGR2Attr(const SGR& state) noexcept
{
    WORD attr;

    if (state.conceal)
    {
        if (state.reverse)
        {
            attr = static_cast<BYTE>(state.foregroundColor << 4) |
                   state.foregroundColor;
            if (state.bold)
            {
                attr |= FOREGROUND_INTENSITY | BACKGROUND_INTENSITY;
            }
        }
        else
        {
            attr = static_cast<BYTE>(state.backgroundColor << 4) |
                   state.backgroundColor;
            if (state.underline)
            {
                attr |= FOREGROUND_INTENSITY | BACKGROUND_INTENSITY;
            }
        }
    }
    else if (state.reverse)
    {
        attr = static_cast<BYTE>(state.foregroundColor << 4) |
               state.backgroundColor;
        if (state.bold)
        {
            attr |= BACKGROUND_INTENSITY;
        }
        if (state.underline)
        {
            attr |= FOREGROUND_INTENSITY;
        }
    }
    else
    {
        attr = state.foregroundColor |
               static_cast<BYTE>(state.backgroundColor << 4) | state.bold |
               state.underline;
    }

    return attr;
}

inline void SetWinSGR(Style style, SGR& state) noexcept
{
    switch (style)
    {
        case Style::Reset:
            state = GetDefaultState();
            break;
        case Style::Bold:
            state.bold = FOREGROUND_INTENSITY;
            break;
        case Style::Underline:
        case Style::SlowBlink:
            state.underline = BACKGROUND_INTENSITY;
            break;
        case Style::Reverse:
            state.reverse = TRUE;
            break;
        case Style::Conceal:
            state.conceal = TRUE;
            break;
        default:
            break;
    }
}

inline void SetWinSGR(Foreground fg, SGR& state) noexcept
{
    if (fg != Foreground::Reset)
    {
        state.foregroundColor = ANSI2Attr(static_cast<BYTE>(fg) - 30);
    }
    else
    {
        state.foregroundColor = GetDefaultState().foregroundColor;
    }
}

inline void SetWinSGR(Background bg, SGR& state) noexcept
{
    if (bg != Background::Reset)
    {
        state.backgroundColor = ANSI2Attr(static_cast<BYTE>(bg) - 40);
    }
    else
    {
        state.backgroundColor = GetDefaultState().backgroundColor;
    }
}

inline void SetWinSGR(ForegroundHI fgHI, SGR& state) noexcept
{
    state.foregroundColor =
        (BACKGROUND_INTENSITY >> 4) | ANSI2Attr(static_cast<BYTE>(fgHI) - 90);
}

inline void SetWinSGR(BackgroundHI bgHI, SGR& state) noexcept
{
    state.backgroundColor =
        FOREGROUND_INTENSITY | ANSI2Attr(static_cast<BYTE>(bgHI) - 100);
}

template <typename T>
void SetWinColorANSI(std::ostream& os, const T value)
{
    os << "\033[" << static_cast<int>(value) << "m";
}

template <typename T>
void SetWinColorNative(std::ostream& os, const T value)
{
    HANDLE handle = GetConsoleHandle(os.rdbuf());
    if (handle != INVALID_HANDLE_VALUE)
    {
        SetWinSGR(value, GetCurrentState());

        // Out all buffered text to console with previous settings:
        os.flush();
        SetConsoleTextAttribute(handle, SGR2Attr(GetCurrentState()));
    }
}

template <typename T>
IsValid<T> SetColor(std::ostream& os, const T value)
{
    if (GetWinTermMode() == WinTerm::Auto)
    {
        if (IsSupportANSI(os.rdbuf()))
        {
            SetWinColorANSI(os, value);
        }
        else
        {
            SetWinColorNative(os, value);
        }
    }
    else if (GetWinTermMode() == WinTerm::Ansi)
    {
        SetWinColorANSI(os, value);
    }
    else
    {
        SetWinColorNative(os, value);
    }

    return os;
}
#else
template <typename T>
IsValid<T> SetColor(std::ostream& os, const T value)
{
    return os << "\033[" << static_cast<int>(value) << "m";
}
#endif
}  // namespace internal

template <typename T>
internal::IsValid<T> operator<<(std::ostream& os, const T value)
{
    const Control controlMode = internal::GetControlMode();

    if (controlMode == Control::Auto)
    {
        return internal::IsSupportColor() && internal::IsTerminal(os.rdbuf())
                   ? internal::SetColor(os, value)
                   : os;
    }

    if (controlMode == Control::Force)
    {
        return internal::SetColor(os, value);
    }

    return os;
}

inline void SetControlMode(Control value) noexcept
{
    internal::GetControlMode() = value;
}

inline void SetWinTermMode(WinTerm value) noexcept
{
    internal::GetWinTermMode() = value;
}

}  // namespace styler

#endif