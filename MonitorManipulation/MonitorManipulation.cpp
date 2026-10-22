#include <iostream>
#include <string>
#include <vector>
//finding monitors and controlling settings
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

//enumerates display monitors and retrieves physical monitor handles
std::vector<PHYSICAL_MONITOR> GetMonitors()
{
    std::vector<PHYSICAL_MONITOR> monitors;
    //creates a vector and asks Windoes to enumerate monitors
    EnumDisplayMonitors(
        //enumerations without a specific device cont4
        nullptr,
        nullptr,
        //call for each monitor
        CollectMonitors,
        //passes the address of the vector through Windows so the callback can fill it
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

void PowerMonitor()
{
    auto monitors = GetMonitors();
    PrintMonitors(monitors);

    if (monitors.empty())
        return;

    std::string selection;
    std::cout << "Select a monitor number: ";

    if (!(std::cin >> selection)) {
        ReleaseMonitors(monitors);
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

    std::string action;
    std::cout << "Enter on or off: ";

    if (!(std::cin >> action)) {
        ReleaseMonitors(monitors);
        return;
    }

    if (action != "on" && action != "off") {
        std::cout << "Invalid power option.\n";
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

int main()
{
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
            PowerMonitor();
        }
        else if (answer == options[2]) {
            running = false;
        }
    }

    return 0;
}