# Stellaris RTOS

Real-Time Operating System for the TI Stellaris architecture, specifically
LM3S3D92. Implements multitasking, a simple window manager via UART, priority
scheduling, and message passing.

## Usage

Can be loaded to a Stellaris MCU using TI's CodeComposer Studio, targetting
the `LM3S3D92`. operates oput of UART0 for I/O. It is easiest to use a TTY
similar to puTTY or the like to connect. You must enable VT100 commands, as the
WindowManager uses VT120 commands to manipulate the cursor.

## Message to ECED4402:
If you are in Hughes' class, he will hide you for looking at this. So don't.
Go away.
