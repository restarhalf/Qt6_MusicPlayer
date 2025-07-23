#include <iostream>
#include <string>
#include <vector>
#include <locale>
#include <codecvt>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

// ANSI颜色代码用于创建渐变效果
const std::vector<std::wstring> gradient_colors = {
    L"\033[38;5;196m", // 红色
    L"\033[38;5;202m", // 橙红色
    L"\033[38;5;208m", // 橙色
    L"\033[38;5;214m", // 黄橙色
    L"\033[38;5;220m", // 黄色
    L"\033[38;5;154m", // 黄绿色
    L"\033[38;5;46m", // 绿色
    L"\033[38;5;51m", // 青色
    L"\033[38;5;21m", // 蓝色
    L"\033[38;5;93m" // 紫色
};

const std::wstring reset_color = L"\033[0m";

void printGradientText(const std::wstring& text)
{
    for (size_t i = 0; i < text.length(); ++i)
    {
        size_t colorIndex = (i * gradient_colors.size()) / text.length();
        std::wcout << gradient_colors[colorIndex] << text[i];
    }
    std::wcout << reset_color << std::endl;
}

void restarhalf()
{
#ifdef _WIN32
    // 设置控制台输入输出编码为UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // 启用虚拟终端处理以支持ANSI转义序列
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif

    // 设置locale以正确处理Unicode
    std::locale::global(std::locale(""));
    std::wcout.imbue(std::locale());

    std::wcout << std::endl;
    std::vector<std::wstring> art_lines = {
        L"██████╗ ███████╗███████╗████████╗ █████╗ ██████╗ ██╗  ██╗ █████╗ ██╗     ███████╗",
        L"██╔══██╗██╔════╝██╔════╝╚══██╔══╝██╔══██╗██╔══██╗██║  ██║██╔══██╗██║     ██╔════╝",
        L"██████╔╝█████╗  ███████╗   ██║   ███████║██████╔╝███████║███████║██║     █████╗  ",
        L"██╔══██╗██╔══╝  ╚════██║   ██║   ██╔══██║██╔══██╗██╔══██║██╔══██║██║     ██╔══╝  ",
        L"██║  ██║███████╗███████║   ██║   ██║  ██║██║  ██║██║  ██║██║  ██║███████╗██║     ",
        L"╚═╝  ╚═╝╚══════╝╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝     "
    };
    // 显示艺术字，每行应用渐变效果
    for (const auto& line : art_lines)
    {
        printGradientText(line);
    }
    std::wcout << std::endl;
}
