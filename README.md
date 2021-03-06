#BtQt [![Build Status](https://travis-ci.org/ArkBriar/BtQt.svg?branch=master)](https://travis-ci.org/ArkBriar/BtQt) [![License](https://img.shields.io/badge/license-LGPLv2.1%2B-blue.svg)](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)

This is a simple Qt based Bitorrent client!

- [Dependencies](#dependencies)
- [Build Instructions](#build-instructions)
    - [Linux](#linux)
    - [Windows](#windows)
- [Release Notes](#release-notes)
- [License](#license)

### Dependencies

+ Requires Qt 5.5.1(*tested*) or higher(*not*)

### Build Instructions 

#### Linux

**Notice**      Need gcc 4.8 or higher because of  some C++11 syntax

It was only tested on openSUSE Tumbleweed. Please post issues if you have problems on other distributions.

##### **Preparation**

1\. Download & Install [Qt 5.5.1](http://download.qt.io/archive/qt/5.5/5.5.1/)

2\. 
*For Debian/Ubuntu user*:

```
sudo aptitude install build-essential libgl1-mesa-dev libglu1-mesa-dev
```

##### **Compile**

``` shell
$ cd BtQt/
$ mkdir build
$ cd build/
$ /path/to/qtbin/qmake ..
$ make release
```

"/path/to/qtbin/" is "/path/to/Qt5.5.1/5.5/gcc_64/bin/" on my machine.

##### **Install**

Installation has not be considered.


#### Windows

It was only tested on Windows 10. Please post issues if you have problems on other distributions.

##### **Preparation & Compile**

1\. Download and install Visual studio 2015 (must install msvc2013) or Visual studio 2013

2\. Download and install Qt5.5.1, set the build toolkit with msvc2013(v120)

3\. Open BtQt.pro with QtCreator

4\. Compile. 

Target 'debug' is compiling with test/main.cpp instead of src/main.cpp. If you find it hard to compile, check it. There should be some UNIX-style codes in it.

There may be some little problems when running debug version, but you could fix it by modifying macro "DebugQmlSrc" in main.cpp.

##### **Install**

Installation has not be considered.

### Release Notes

The release packages are all compiled in Travis-CI with

- OS: ubuntu 12.04.5 LTS
- CC: gcc-4.8
- CXX: g++-4.8
- Qt: Qt5.4.2

So if there are some problems, please send an email to me.

### License

BtQt is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser Public License as published by the Free Software Foundation; either version 2.1 of the license, or(at your option) any later version.

You should have received a copy of the GNU Lesser General Public License along with this program. If not, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses/).
