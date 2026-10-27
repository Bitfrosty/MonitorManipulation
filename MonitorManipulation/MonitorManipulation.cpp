#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <physicalmonitorenumerationapi.h>
#include <lowlevelmonitorconfigurationapi.h>

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Dxva2.lib")

const std::string options[] = { "display", "power", "exit" };

BOOL CALLBACK CollectMonitors(
    HMONITOR monitor,
    HDC,
    LPRECT,
    LPARAM data)
{
    auto& monitors =
        *reinterpret_cast<std::vector<PHYSICAL_MONITOR>*>(data);

    DWORD count = 0;

    if (!GetNumberOfPhysicalMonitorsFromHMONITOR(monitor, &count)
        || count == 0) {
        return TRUE;
    }

    std::vector<PHYSICAL_MONITOR> physicalMonitors(count);

    if (GetPhysicalMonitorsFromHMONITOR(
        monitor, count, physicalMonitors.data())) {
        monitors.insert(
            monitors.end(),
            physicalMonitors.begin(),
            physicalMonitors.end());
    }

    return TRUE;
}

std::vector<PHYSICAL_MONITOR> GetMonitors()
{
    std::vector<PHYSICAL_MONITOR> monitors;

    EnumDisplayMonitors(
        nullptr,
        nullptr,
        CollectMonitors,
        reinterpret_cast<LPARAM>(&monitors));

    return monitors;
}

void ReleaseMonitors(std::vector<PHYSICAL_MONITOR>& monitors)
{
    if (!monitors.empty()) {
        DestroyPhysicalMonitors(
            static_cast<DWORD>(monitors.size()),
            monitors.data());

        monitors.clear();
    }
}

void PrintMonitors(const std::vector<PHYSICAL_MONITOR>& monitors)
{
    if (monitors.empty()) {
        std::wcout << L"No monitors found.\n";
        return;
    }

    for (size_t i = 0; i < monitors.size(); ++i) {
        std::wcout
            << L"Monitor " << i + 1 << L"\n"
            << L"Monitor Name: "
            << monitors[i].szPhysicalMonitorDescription
            << L"\n\n";
    }
}

void DisplayMonitors()
{
    auto monitors = GetMonitors();

    PrintMonitors(monitors);
    ReleaseMonitors(monitors);
}

void PowerMonitor(const std::string& selection, const std::string& action)
{
    auto monitors = GetMonitors();

    if (monitors.empty()) {
        std::wcout << L"No monitors found.\n";
        return;
    }

    size_t selectedIndex = monitors.size();

    for (size_t i = 0; i < monitors.size(); ++i) {
        if (selection == std::to_string(i + 1)) {
            selectedIndex = i;
            break;
        }
    }

    if (selectedIndex == monitors.size()) {
        std::cout << "Invalid monitor number.\n";
        ReleaseMonitors(monitors);
        return;
    }

    if (action != "on" && action != "off") {
        std::cout << "Invalid power option. Use on or off.\n";
        ReleaseMonitors(monitors);
        return;
    }

    constexpr BYTE powerCode = 0xD6;
    constexpr DWORD powerOn = 0x01;
    constexpr DWORD powerOff = 0x04;

    DWORD value = (action == "on") ? powerOn : powerOff;

    BOOL success = FALSE;
    constexpr int maxAttempts = 3;

    for (int attempt = 1; attempt <= maxAttempts; ++attempt) {
        success = SetVCPFeature(
            monitors[selectedIndex].hPhysicalMonitor,
            powerCode,
            value);

        if (success)
            break;

        DWORD error = GetLastError();

        std::cerr
            << "Attempt " << attempt
            << " failed. Windows error: " << error
            << " (0x" << std::hex << error << std::dec << ")\n";

        if (attempt < maxAttempts)
            Sleep(250);
    }

    if (success) {
        std::cout << "Power command sent.\n";
    }
    else {
        std::cerr
            << "Failed to send monitor power command after "
            << maxAttempts << " attempts.\n";
    }

    ReleaseMonitors(monitors);
}

bool IsValidOption(const std::string& input)
{
    for (const std::string& option : options) {
        if (input == option)
            return true;
    }

    return false;
}

void PrintHelp()
{
    std::cout
        << "MonitorPower - Windows Monitor Power Utility\n"
        << "\n"
        << "USAGE\n"
        << "  MonitorPower.exe <command> [options]\n"
        << "\n"
        << "COMMANDS\n"
        << "  display\n"
        << "    List all detected physical monitors.\n"
        << "\n"
        << "  power <monitor> <on|off>\n"
        << "    Turn a specific monitor on or off.\n"
        << "\n"
        << "  interactive\n"
        << "    Turn on interactive selection mode.\n"
        << "\n"
        << "  help\n"
        << "    Show this help information.\n"
        << "\n"
        << "EXAMPLES\n"
        << "  MonitorPower.exe display\n"
        << "  MonitorPower.exe power 1 off\n"
        << "  MonitorPower.exe power 1 on\n"
        << "  MonitorPower.exe interactive\n"
        << "  MonitorPower.exe help\n"
        << "\n"
        << "NOTES\n"
        << "  - Monitor numbers start at 1.\n"
        << "  - Requires a DDC/CI compatible monitor.\n"
        << "  - Some monitors may not support power control.\n"
        << "  - Power-on may not work after powering off.\n";
}

int main(int argc, char* argv[])
{
    if (argc == 1) {
        PrintHelp();
        return 0;
    }

    std::string command = argv[1];

    if (command == "help" && argc == 2) {
        PrintHelp();
        return 0;
    }

    if (command == "display" && argc == 2) {
        DisplayMonitors();
        return 0;
    }

    if (command == "power" && argc == 4) {
        PowerMonitor(argv[2], argv[3]);
        return 0;
    }

    if (command == "interactive" && argc == 2) {
        bool running = true;

        while (running) {
            std::string answer;

            std::cout << "\nSelect an option (display, power, exit): ";

            if (!(std::cin >> answer))
                break;

            if (!IsValidOption(answer)) {
                std::cout << "Option not valid!\n";
                continue;
            }

            if (answer == options[0]) {
                DisplayMonitors();
            }
            else if (answer == options[1]) {
                DisplayMonitors();

                std::string selection;
                std::string action;

                std::cout << "Select a monitor number: ";
                if (!(std::cin >> selection))
                    break;

                std::cout << "Enter on or off: ";
                if (!(std::cin >> action))
                    break;

                PowerMonitor(selection, action);
            }
            else if (answer == options[2]) {
                running = false;
            }
        }

        return 0;
    }

    std::cerr << "Invalid command or arguments.\n\n";
    PrintHelp();
    return 1;
}