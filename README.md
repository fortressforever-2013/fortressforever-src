<div align="center">
  <img src="https://avatars.githubusercontent.com/u/2388970" width="200" height="200">
</div>

# Fortress Forever 2013
This is a working port of the game [Fortress Forever](https://store.steampowered.com/app/253530/Fortress_Forever/) on Source SDK 2013 using Nbc66's [SDK 2013 Community Edition](https://github.com/Nbc66/source-sdk-2013-ce) repository.

Fortress Forever was made using Source SDK 2006, the original repository of the game could be found [here](https://github.com/fortressforever/fortressforever).

**Note:** This repository contains the **source code** of the game and **not the game files**, The repository containing the game files could be found [here](https://github.com/fortressforever-2013/FortressForever2013).
# Instructions (Windows)
To be able to compile Fortress Forever 2013 you will need to download **Visual Studio 2022** and install:
* MSVC v143 - VS 2022 C++ x64/x86 build tools
* C++ MFC Library for latest v143 build tools (x86 and x64)
* Windows 11 SDK (10.0.22000.0)

1. Clone this repository and run `creategameprojects.bat` or `createallprojects.bat`
    * This will generate the project files and solution files that are needed in order to compile the game.
2. Open the generated `Game_FF.sln` or `Everything_FF.sln` using Visual Studio 2022.
3. Run `Build Solution`.
    * The compiled binaries would automatically be copied to `mp/game/FortressForever2013`.