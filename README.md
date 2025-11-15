# udisks.xplr

[![asciicast](https://asciinema.org/a/5W5YeYToCoDRqtJojWqDVCm5N.svg)](https://asciinema.org/a/5W5YeYToCoDRqtJojWqDVCm5N)

An [xplr](https://github.com/sayanarijit/xplr) plugin that integrates with UDisks.

## Features

- Listing Devices
- Mounting and unmounting devices
- Live reload when devices are added or removed

## Installation

Unlike a normal xplr plugin, you cannot install this just by cloning the repository into your plugin
directory or using a plugin manager. This is because udisks.xplr consists of only a comparatively
small amount of Lua code while the bulk of it is written in C++ in order to integrate with the
UDisks daemon via DBus. This works by loading a native Lua module.

1. Install the prerequesites
- cmake
- a C++ compiler of your choice
- luajit
- libudisks
    - gio/glib

2. Configure and install
```bash
cmake -DCMAKE_INSTALL_PREFIX=/path/to/your/plugin/directory -B build
cmake -B build .
cmake --build build
cmake --install build
```
3. Proceed to configuration

## Configuration
Make sure you have added your plugin directory to `package.path` as described in upstream
documentation.
```lua
require("udisks").setup {
    -- This option is mandatory and must match the directory you specified for CMAKE_INSTALL_PREFIX.
    -- This is required in order to allow loading the shared library we built in the previous step.
    -- If you have some other means of loading the library, you may explicitly set this to "false"
    -- to assert that you've handled this.
    install_path = "/path/to/your/plugin/directory",

    -- All following options are optional. Shown values are defaults.

    -- The key you press in default mode to open udisks.xplr.
    open_key = "D",
}
```
