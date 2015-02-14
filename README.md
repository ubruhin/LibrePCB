EDA4U Project Description
=========================

A free & OpenSource Schematic / Layout Editor

...just started... ;-)


## Development Requirements

To compile EDA4U, you need the following software components:
- Qt >= 5.2
- A compiler with C++11 support
- `pkg-config` (http://www.freedesktop.org/wiki/Software/pkg-config/)
- `libxml2` / `libxml2-dev` (http://www.xmlsoft.org/index.html)

### UNIX/Linux
The packages `pkg-config`, `libxml2` and `libxml2-dev` should be available in your package manager.

Command for Debian/Ubuntu:
```
sudo apt-get install pkg-config libxml2 libxml2-dev
```

### Mac OS X
TODO

### Microsoft Windows
You need the following archives (* stands for the version number):
`gettext-runtime_*_win32.zip, glib_*_win32.zip, pkg-config_*_win32.zip, zlib_*_win32.zip, libxml2_*_win32.zip, libxml2-dev_*_win32.zip`

These archives can be downloaded from http://ftp.gnome.org/pub/gnome/binaries/win32/glib/ and http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/.
    
All these archives contain a "bin" directory. Copy all these "bin" directories into your compiler's directory (for example C:\Qt\5.3\mingw482_32\). From the package "libxml2-dev" you need also to copy the directories "include" and "lib" into your compilers directory.

## Workspace

At the first startup, EDA4U asks for a workspace directory where the library elements and projects will be saved.
For developers there is a demo workspace inclusive library and projects in this repository (subdirectory "dev/workspace/").
Creating new projects is not yet fully supported (for example adding new components to the schematic is not yet working),
so you should use only the already existing demo projects for testing.
