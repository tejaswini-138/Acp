# 2D Graphics Editor (Console)

A simple console-based 2D graphics editor written in C that renders basic shapes to a text canvas.

## Features
- Draw Lines, Rectangles, Circles, and Triangles
- List, Modify, Delete shapes by ID
- Save drawing to a text file
- Uses ANSI colors for nicer terminal output

## Project Structure
- `graphics_editor.c` — main source file containing the editor and rendering logic

## Requirements
- C compiler (GCC/Clang) with C99 support
- On Windows, PowerShell or a terminal that supports ANSI escapes (Windows 10+ recommended)

## Build
Open a terminal in this folder and run:

```bash
gcc graphics_editor.c -o graphics_editor.exe -lm
```

Note: `-lm` links the math library used by the circle routine.

## Run

On Windows (PowerShell / cmd):

```powershell
.\graphics_editor.exe
```

Controls are menu-driven. Enter numeric selections and follow prompts to add/modify/delete shapes.

## Canvas Details
- Default canvas size: 20 rows x 50 columns
- Coordinates: X in [0..49], Y in [0..19]

## Saving
Select the "Save Drawing to File" menu option to export a plain-text representation of the canvas. If you leave the filename blank, the program will save to `drawing.txt`.

## Notes & Tips
- If ANSI colors don't display on Windows, run the program in Windows Terminal or enable virtual terminal processing.
- The program stores shapes in an in-memory array (max 100 shapes). Deleting a shape shifts subsequent shapes down.
