Team Fortress 2
=====

[![Build status](https://ci.appveyor.com/api/projects/status/1qx349wcxa5eenjk/branch/master?svg=true)](https://ci.appveyor.com/project/NicknineTheEagle/tf2-base/branch/master)

This is the old Team Fortress 2 source code from February 2008 ported to Source Engine 2013. This ensures the game has all the latest engine features and security fixes. No new features will be added to the code. No bugs will be fixed with the exception of crashes and bugs that were not in the original 2008 build of the game.

## Dependencies

### Windows
* [Visual Studio 2013 with Update 5](https://visualstudio.microsoft.com/vs/older-downloads/)

### macOS
* [Xcode 5.0.2](https://developer.apple.com/downloads/more)

### Linux
* GCC 4.8
* [Steam Client Runtime](http://media.steampowered.com/client/runtime/steam-runtime-sdk_latest.tar.xz)

## Building

Compiling process is the same as for Source SDK 2013. Instructions for building Source SDK 2013 can be found here: https://developer.valvesoftware.com/wiki/Source_SDK_2013

Assets that need to be used with compiled binaries: https://mega.nz/#!PFZiwCyY!BKDYXH4UOhTgCCvGCAaI2JULNc8B94HdmLo308BNeI4

Note that the above archive is not a playable build. It does not contain binaries, you need to build them yourself from this repository.

## Installing:

### Client:

1. Go to the Tools section in your Steam Library and install Source SDK Base 2013 Multiplayer. 

2. Download the assets package and extract its contents to <Steam>\steamapps\sourcemods.

3. Restart Steam. "Team Fortress 2 1.0.1.8 Port" should appear in your Steam Library.

4. Put your compiled binaries into "bin" directory.

NOTE: If you're on Linux or Mac, Steam client currently has a bug where it doesn't attach -steam parameter when running Source mods like it's supposed to. You'll need to manually add -steam parameter to the mod in your Steam Library.

### Server:

1. These instructions assume you know how to host a dedicated server for TF2 and/or other Source games. If you don't, refer to these articles:

   * https://developer.valvesoftware.com/wiki/SteamCMD
   
   * https://wiki.teamfortress.com/wiki/Windows_dedicated_server 
   
   * https://wiki.teamfortress.com/wiki/Linux_dedicated_server 

2. Use SteamCMD to download app 244310 (Source SDK Base 2013 Dedicated Server).

3. Download the assets package and extract its contents to where you installed Source SDK Base 2013 Dedicated Server.

4. If you're on Linux, go to <server_install_folder>/bin and make copies of the files as follows:

   * soundemittersystem_srv.so -> soundemittersystem.so

   * scenefilecache_srv.so -> scenefilecache.so

5. Put your compiled binaries into "bin" directory.
