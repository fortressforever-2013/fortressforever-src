<div align="center">
  <img src="https://avatars.githubusercontent.com/u/2388970" width="200" height="200">
</div>

# Fortress Forever 2013
This is an actively developed port of the game [Fortress Forever](https://store.steampowered.com/app/253530/Fortress_Forever/) on Source SDK 2013 using [@Nbc66](https://github.com/Nbc66)'s [SDK 2013 Community Edition](https://github.com/Nbc66/source-sdk-2013-ce) repository.

Fortress Forever is based upon Source SDK 2006, the original repository of the game can be found [here](https://github.com/fortressforever/fortressforever).

**Note:** This repository contains the **source code** of the game and **not the game files**, The repository containing the game files are found [here](https://github.com/fortressforever-2013/fortressforever).

# Build Instructions

- ## Windows
  To be able to compile Fortress Forever 2013 on Windows, you will need to download **Visual Studio 2022** and install:
  * MSVC v143 - VS 2022 C++ x64/x86 build tools
  * C++ MFC Library for latest v143 build tools (x86 and x64)
  * Windows 11 SDK (10.0.22000.0)
<br><br>
  1. Clone this repository and run `.\creategameprojects.bat` or `.\createallprojects.bat` located in `mp\src`
      * This will generate the project files and solution files that are needed in order to compile the game.
  2. Open the generated `Game_FF.sln` or `Everything_FF.sln` using Visual Studio 2022.
  3. Switch the current configuration from `Debug` to `Release`.
  4. Run `Build Solution`.
      * The compiled binaries should automatically be copied to `mp\game\FortressForever2013`.

- ## Linux
  **Note:** These instructions were only tested on Ubuntu 22.04 (Jammy Jellyfish), but should work for most Debian-based Linux distributions.

  #### Before getting started, install the following packages:
  - `build-essential`
  - `gcc-9`
  - `gcc-9-multilib`
  - `g++-9`
  - `g++-9-multilib`

  To be able to compile Fortress Forever 2013 on Linux, you will need to do the following:
  1. Clone this repository and run `./creategameprojects` or `./createallprojects` located in `mp/src`
      * This will generate the makefiles that are needed in order to compile the game.
  2. Run `make -f Game_FF.mak` in `mp/src` and the source code will start compiling.
      * The compiled binaries would automatically be copied to `mp/game/FortressForever2013`.

# External content
- ### [Coplay](https://github.com/CoaXioN-Games/coplay)
- ### [Discord-RPC](https://github.com/discord/discord-rpc)
- ### [Lua (5.1.5)](https://www.lua.org/)
- ### [LuaBridge3](https://github.com/kunitoki/LuaBridge3)