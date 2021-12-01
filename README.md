Octomino's SDL Input Plugin
===========================
An input plugin for Zilmar spec emulators including Project 64 1.6. It 
uses SDL2's GameController API and supports many gamepads including Xbox, 
PS4, and Switch Pro.

This is a fork of [clickdevin/octomino-sdl-input](https://github.com/clickdevin/octomino-sdl-input), focused on adding a configuration GUI as well as new features.

Downloading
-----------
The latest release can be downloaded from 
[here](https://github.com/wermipls/octomino-sdl-input/releases).

Installing
----------
Copy `octomino-sdl-input.dll` and `gamecontrollerdb.txt` to your 
emulator's plugins folder.

Building from source
--------------------
To build from source, use [MSYS2](https://www.msys2.org/) with the 
following packages:
 - `mingw-w64-i686-gcc`
 - `mingw-w64-i686-SDL2`
 - `make`
 - `wget`
 - `zip`

Then, using the `MINGW32` subsystem, run `make all` from the project's 
root directory.

Other MinGW distributions will also likely work, provided you have the 
proper tools installed.

Additionally, `gamecontrollerdb.txt` can be updated using `make update-db`.

License
-------
This plugin is licensed under the Mozilla Public License 2.0. See 
`LICENSE` or visit <http://mozilla.org/MPL/2.0/>.

Third party libraries used:
* [microui by rxi](https://github.com/rxi/microui) (MIT, see `src/microui.h` for details)
* [ini.h by Mattias Gustavsson](https://github.com/mattiasgustavsson/libs/blob/main/ini.h) (MIT, see `src/ini.h` for details)
