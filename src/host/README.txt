INTRODUCTION
============

In release 7.8, the host tree was significantly restructured and new versions of third party tools are required to build the host software. This document provides an overview of changes and requirements on build machines. The most important changes compared to version 7.7 are:
- requires Windows 7 and Microsoft Visual Studio 2010 to build on Windows (previously 2005)
- requires QT version 4.8 (previously 4.2)
- host tree structure and Makefiles are different
- requires GNU make 3.81 or later to build on Linux (previously 3.80)
- builds on Cygwin and for PowerPC architectures are not supported
- by default, dynamic (.so) libraries are used on Linux (previously static)

Requirements
============

Linux
+++++
The Linux libraries include versions for embedded Linux (Arm-based) products (IP cameras and VRMs) and for host Linux (x86 based) products (add-on boards). Linux builds are created using GNU make, you must have version 3.81 or later installed on the build machine.

Embedded Linux (make target 's7'):
---------------------
    - ARM tools: Code sourcery version 2009q3-67 (unchanged from previous release)
    - ARM tools installed in  /tools/codesourcery/Sourcery_G++_Lite_for_ARM_GNU_Linux/2009q3-67/Linux. 
      You need to set up environment variable ARM_TOOLS if that location is different or edit 
      make/target-s7.mk template
    - Several third party libraries are shipped in install/lib/s7 and install/include/s7 directories. 
      If you delete these directories or do 'make clean', you will need to unpack installers to recover them. 
      Stretch does not provide sources for the third party libraries. These libraries are:
        - libaio
        - libfcgi
        - liblive555 
        - libxml
        - libbost (headers only)
    
Host Linux (make target 'x86')
------------------------------
    - Host tools: GNU gcc 4.1.2 or later
    - Tools installed in /usr/bin. You will need to set up environment variable GNU_TOOLS or edit
      make/target-x86.mk template if the location is different
    - standard development tools including make 3.81 and patch
    - You need to have Linux kernel development packages and libaio installed on the build machines. 
      On Ubuntu:
        > apt-get install linux-headers-$(uname -r) 
        > apt-get install libaio-dev
      On Centos:
        > yum install kernel-devel kernel-headers
        > yum install libaio-devel
    - to build uboot, you will need to install mkimage
        > apt-get uboot-mkimage
        > yum install uboot-tools
    - To build dvrcp program, you will need to install QT (minimum version 4.8.3) and X11 libraries.
      On Ubuntu:
        > apt-get install libx11-dev libxv-dev libx11-6 x11-utils
        > apt-get install qt4-dev-tools 
      On Centos:
        > yum install libX11-devel libXext-devel libXv-devel
        > yum install qt4-devel
    - If QT version 4.8.1 or later is not available via package download on your build machine, you may
      build QT from sources. Download Qt from http://releases.qt-project.org/qt4/source/qt-everywhere-opensource-src-4.8.3.tar.gz
      and build it as follows:
            > tar -zxf qt-everywhere-opensource-src-4.8.3.tar.gz
            > cd qt-everywhere-opensource-src-4.8.3
            > ./configure -prefix <install_dir>
            > make
            > make install
      This will install QT in <install_dir>. Note that on Centos 5, the following patch may be required:
      sed -e "s@# include <linux/futex.h>@# define FUTEX_WAIT 0\n# define FUTEX_WAKE 1@g" -i src/corelib/thread/qmutex_unix.cpp
    - If QT is not in a standard location, you must set environment variable QMAKE to point the location of 'qmake':
            > export QMAKE=<install_dir>/bin/qmake
      or you can edit src/bin/dvrcp/Makefile
    - Except for boost headers, no other third party libraries are present in the install directory.

Supported Linux version
-----------------------
Stretch tested following Linux distributions:
    Ubuntu 10
        - Build host code for s7 and x86 targets
        - Build SCP firmware using Stretch IDE tools
        - Run the PCIe boards

    Ubuntu 12
        - Build code for s7 and x86 targets
        - Run the PCIe boards

    Centos 5 and 6 (64-bit only)
        - Build host code for s7 and x86 targets
        - Build SCP firmware using Stretch IDE tools
        - Run the PCIe boards

    Centos 6 (32-bit)
        - Build host code for s7 and x86 targets
        - Build SCP firmware using Stretch IDE tools


Windows
+++++++
The host.bat script performs full Windows build in batch mode. Alternatively, host.sln allows to open
Visual Studio to build the host tree interactively.
    - the build machine must be Windows 7. Builds on earlier versions of Windows are not supported.
    - Use Microsoft Visual Studio 2010 or Visual Studio Express 2010. If these tools are 
      in non-standard location, you must set MSVC_PATH accordingly. By default:
        SET MSVC_PATH=C:\Program Files\Microsoft Visual Studio 10.0
    - Install Microsoft Platform SDK 7.1 (downloaded file is usually called winsdk_web.exe)
    - Optional, required only if you need to rebuild Stretch driver (stretch_dvr):
      Install Microsoft DDK 7600.16385.0 or WDK 7600.16385.1. 
      You must set MS_DDK_ROOT to point to it, for example:
          SET MS_DDK_ROOT=C:\WinDDK\7600.16385.1 
      You will need to provide your own certificate to sign the driver, which is required by Windows 7 64 bit.
    - Optional, required only if you need to rebuild sdvr_sdk_ui libraries:
      Download Microsoft DirectX SDK 2008 and set DIRECTX_SDK_PATH to point to it, for example:
          SET DIRECTX_SDK_PATH=C:\Program Files (x86)\Microsoft DirectX SDK (November 2008) 
      More recent versions of that SDK are known to not work with Stretch libraries.
    - Optional, required only if need to rebuild dvrcp program
      Download and install Qt version 4.8 for VS 2010. You can download the package from  
        http://releases.qt-project.org/qt4/source/qt-win-opensource-4.8.3-vs2010.exe
    - if using VS2010 Professional Edition, you may download and install Qt Visual Studio Add-in for MSVC 2010
        http://releases.qt-project.org/vsaddin/qt-vs-addin-1.1.11-opensource.exe
    - Set QT_ROOT to the root of the QT folder, for example
        SET QT_ROOT=C:\Qt\4.8.3
        
HOST TREE STRUCTURE
===================
The host tree structure has changed and is now as follows:
- all the source files are in the src directory. No files are ever created in the src directory. The only exception to this
  is that files are currently generated in src/bin/dvrcp directory. The src directory has the following structure:
    - src/lib       contain sources for all libraries
    - src/bin       contain sources for all executable programs
    - src/kernel    has Linux kernel sources
    - src/driver    has Stretch driver sources
- all build outputs are in the install directory. This directory has the following structure:
    - install/lib/<arch>        contains static and dynamic libraries
    - install/include/<arch>    contains library include files
    - install/bin/<arch>        contains executable programs and scripts   
    - install/html<arch>        contains camera CGI server GUI
    - install/conf/<arch>       contains servers configuration files
- build temporary files (for example object and dependency files) are in build directory. This directory is absent
  from the installers, but will be created if a build is done.
- make directory contains Makefile templates

LINUX MAKEFILES
===============
The full Linux build may done by going to host directory and typing 'make'. Alternatively, builds for s7 or x86 targets may
be done by specifying the target:
    > make s7
    > make x86
The 'clean' target removes or temporary build files (object, dependency), while 'clear' target removes also install files.

The build system is hierarchical and it is possible to descend to any subdirectory and do 'make' there. The five supported 
targets (all, s7, x86, clean, clear) are also available in subdirectories. When building in subdirectories, variable ROOT must
be set to point the top of the host tree (where 'make' subdirectory is), so that Make can find template files.
Setting ROOT is not required if 'find_root' macro is defined. An example of find_root macro is below:

# The function 'find_root' walks up directory tree until it finds a file passed as parameter.
# It then returns the directory where that file is found
# Example:  ROOT := $(call find_root,.host_root)
_sp :=
_sp +=
_walk = $(if $1,$(wildcard /$(subst $(_sp),/,$1)/$2) $(call _walk,$(wordlist 2, $(words $1),x $1),$2))
_find = $(firstword $(call _walk,$(strip $(subst /, , $1)),$2))
find_root = $(patsubst %/$1,%,$(call _find,$(CURDIR),$1))

By default, all programes except DVRCP are dynamically linked, using shared libraries. You can force static links by
setting STATIC_LINK environment variable or on make command line: make STATIC_LINK=1.
