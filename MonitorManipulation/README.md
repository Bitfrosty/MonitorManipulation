# MonitorPower

A lightweight Windows command-line utility written in C++ for listing physical monitors and sending power commands to an individual display through DDC/CI.

## Features

- List detected physical monitors and their descriptions.
- Send an on or off command to a selected monitor.
- Use direct commands or an interactive menu.
- Retry failed power commands up to three times, with a 250 ms delay between attempts.
- Report Windows error codes when a power command fails.

## Requirements

- Windows.
- A C++ compiler and Windows SDK. Visual Studio with the **Desktop development with C++** workload is a suitable option.
- CMake.
- A monitor and connection that support DDC/CI, with DDC/CI enabled in the monitor's settings.

The application uses the Windows monitor configuration APIs and links against `User32` and `Dxva2`.

## Build

From the project directory containing `CMakeLists.txt`, configure and build:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

The examples below assume the executable is named `MonitorManipulation.exe`. Its output directory and name depend on the CMake configuration. For a typical Visual Studio generator build, it may be located in `build\Release`:

```powershell
cd build\Release
.\MonitorManipulation.exe help
```

## Usage

```text
MonitorPower.exe <command> [options]
```

| Command | Description |
| --- | --- |
| `display` | List detected physical monitors. |
| `power <monitor> <on\|off>` | Send a power command to a specific monitor. |
| `interactive` | Open the interactive menu. |
| `help` | Show usage information. |

Running the program without arguments also displays help. Commands and power options are case-sensitive; use lowercase.

### List monitors

```powershell
.\MonitorManipulation.exe display
```

Each detected monitor is shown with a number and its physical monitor description. Monitor numbers start at **1**. Run `display` before selecting a monitor, especially after changing the display setup.

### Control monitor power

```powershell
# Send an off command to monitor 1
.\MonitorManipulation.exe power 1 off

# Send an on command to monitor 1
.\MonitorManipulation.exe power 1 on
```

A successful API call prints:

```text
Power command sent.
```

This confirms that the command was sent; the program does not verify the resulting power state.

### Interactive mode

```powershell
.\MonitorManipulation.exe interactive
```

Choose `display`, `power`, or `exit`. Selecting `power` lists the monitors, then prompts for a monitor number and an `on` or `off` action. The `exit` option is available inside interactive mode.

## How it works

1. Enumerates Windows display monitors using `EnumDisplayMonitors`.
2. Retrieves physical monitor handles and descriptions using the Windows monitor configuration APIs.
3. Sends a power command using `SetVCPFeature` with VCP code `0xD6`.
4. Releases the acquired physical monitor handles when finished.

The current implementation sends `0x01` for `on` and `0x04` for `off`. Actual behavior depends on the monitor's support for these values.

## Limitations and troubleshooting

- **Windows only:** the application depends on Windows APIs.
- **Monitor compatibility varies:** detecting a monitor does not guarantee that it supports DDC/CI power control.
- **Power-on may fail after power-off:** some monitors stop accepting DDC/CI commands while powered down. Use the monitor's physical power button if necessary.
- **No monitors found:** check that the display is connected and detected by Windows. The program lists only physical monitors whose handles it can retrieve successfully.
- **Power command fails:** confirm that DDC/CI is enabled in the monitor's on-screen settings and that the monitor and connection support power control. Failed attempts print Windows error codes in decimal and hexadecimal.
- **Monitor numbering:** numbers are based on enumeration order and are not persistent identifiers or guaranteed to match Windows Display Settings.

### Exit codes

| Code | Meaning |
| --- | --- |
| `0` | Help, display, power, or interactive mode completed. |
| `1` | An invalid command or argument count was supplied. |

Power-operation failures, invalid monitor selections, and invalid power options currently print an error but still return exit code `0` when invoked with the expected argument count. Scripts should not use the exit code alone to determine whether a power command succeeded.
