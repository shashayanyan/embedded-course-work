# Embedded Systems Coursework

A comprehensive embedded systems project implementing a real-time operating system kernel with interrupt handling, event-driven scheduling, and interactive console interface for ARM Cortex-A8 processor.

## Overview

This project demonstrates core embedded systems concepts including:
- **Bare-metal ARM programming** - Direct hardware initialization and management
- **Interrupt handling** - ISR setup and management
- **Event-driven scheduling** - Task queuing and execution
- **UART communication** - Serial communication with terminal
- **Timer management** - System timekeeping and periodic events
- **Virtual terminal** - Full console with cursor and color support
- **Real-time constraints** - Low-latency command processing

## Architecture

### Core Components

#### Hardware Layer
- **UART Module** (`uart.c`/`uart.h`) - Serial communication driver for UART0
- **Timer Module** (`timer.c`/`timer.h`) - System timer and timing services
- **ISR Handler** (`isr.c`/`isr.h`) - Interrupt service routines
- **IRQ System** (`irq.S`/`irq.c`) - ARM interrupt handling assembly and C code

#### Software Layer
- **Event System** (`event.c`/`event.h`) - Event queue and dispatcher
- **Console** (`console.c`/`console.h`) - Terminal UI with colors and cursor management
- **Stream API** (`stream.c`/`stream.h`) - Abstraction layer for I/O operations
- **Command History** (`history.c`/`history.h`) - Command history tracking

#### Utilities
- **Printf** (`kprintf.c`) - Kernel printf implementation
- **Ring Buffer** (`ring.c`/`ring.h`) - Data queue structure
- **Bootstrap** (`startup.s`/`exception.s`) - ARM assembly boot code and exception handling

### Data Flow

```
Hardware Interrupt (UART/Timer)
    ↓
IRQ Handler (irq.S)
    ↓
ISR Dispatcher (isr.c)
    ↓
Stream/Event Post
    ↓
Event Dispatcher (event.c)
    ↓
Application Handler (main.c)
    ↓
Console/User Feedback
```

## Getting Started

### Prerequisites

- **QEMU** - ARM system emulator
- **GCC Arm Embedded Toolchain** - ARM cross-compiler
  ```bash
  sudo apt-get install qemu-system-arm arm-none-eabi-gcc arm-none-eabi-binutils
  ```

### Building

```bash
# Build for Versatile PB board
make clean-all
make all
```

The Makefile automatically:
- Compiles C files with appropriate ARM flags
- Assembles startup code
- Links with the custom linker script (`versatile.ld`)
- Generates both ELF and binary formats

### Running

```bash
# Run in QEMU emulator
make run

# Run with GDB debugging support
make debug
```

## Features

### Interactive Shell

The system provides a `simple-shell` with the following commands:

- **`echo <text>`** - Echo text to console
- **`davinci <text>`** - Write text backwards to console
- **Empty line** - Reprompt

Example:
```
simple-shell>$ echo Hello World
Hello World
simple-shell>$ davinci dlroW olleH
Hello World
simple-shell>$ 
```

### Real-Time UI

- **Blinking cursor** - Animated cursor feedback (500ms blink rate)
- **Status bar** - Displays uptime and system statistics
- **Color support** - Text coloring for UI elements
- **Keyboard echo** - Real-time character echo as you type

### Event-Driven Scheduler

The kernel implements a priority event queue that:
- Posts events with millisecond-precision timing
- Executes handlers in event order
- Manages periodic background tasks
- Integrates with interrupt handlers

### Interrupt-Driven I/O

- **UART interrupts** - Non-blocking serial input
- **Timer interrupts** - Periodic events and system timekeeping
- **Proper interrupt context** - Safe event posting from ISRs

## System Configuration

Edit the Makefile to customize:

- **`MEMSIZE`** - Available RAM in KB (default: 64KB)
- **`QCPU`** - CPU model (cortex-a8)
- **`MACHINE`** - QEMU machine type (versatilepb)
- **`objs`** - Source files to compile

## Memory Layout

The linker script (`versatile.ld`) defines:
- **Boot section** - Startup code and exception vectors
- **Code section** - Executable instructions
- **Data section** - Global and static variables
- **BSS section** - Uninitialized data
- **Stack** - Grows downward from upper memory

## Development

### Key Functions

- **`_start()`** - Main C entry point after hardware init
- **`event_loop()`** - Main scheduler loop
- **`event_post()`** - Schedule an event for later execution
- **`console_init()`** - Initialize console and set command handler
- **`line_handler()`** - Process user commands

### Debug Output

Use `kprintf()` for kernel debugging:
```c
kprintf("[DEBUG] Hello Youssef From debug console!!!\n");
```

Debug output is routed to `debug.log` in QEMU.

## Performance Characteristics

- **Event dispatch** - Millisecond resolution
- **UART baud** - Standard rate with interrupt-driven buffering
- **CPU utilization** - Periodic status bar shows CPU/event metrics
- **Interrupt latency** - Hardware-bounded by ARM interrupt model

## Directory Structure

```
.
├── Makefile              # Build configuration
├── *.c / *.h            # Source files
├── *.s / *.S            # Assembly files
├── versatile.ld         # Linker script
├── .vscode/             # IDE configuration
├── worklog/             # Development notes
├── build/               # Build artifacts (generated)
└── README.md            # This file
```

## Troubleshooting

### Build Fails
- Ensure ARM toolchain is installed and in PATH
- Run `make clean-all` and rebuild
- Check that `versatile.ld` is present

### Emulation Issues
- Verify QEMU is installed: `qemu-system-arm --version`
- Check memory size doesn't exceed available system RAM
- Review debug.log for hardware errors

### Console Not Responding
- Check UART initialization in startup
- Verify interrupt handlers are enabled
- Ensure event loop is running

## References

- ARM Cortex-A8 Architecture Manual
- VersatilePB QEMU Documentation
- ARM GCC Toolchain User Guide

## Author

Shashayanyan

## License

Educational coursework - refer to course guidelines for usage rights.
