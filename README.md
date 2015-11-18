BtQt
-----------
[![License](https://img.shields.io/badge/license-LGPLv2.1%2B-blue.svg)](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)

This is a simple Qt based Bt client!

[[_TOC_]]

### Dependencies

+ Requires Qt 5.5.1(*tested*) or higher(*not*)

### Build Instructions 

#### Linux

**Attention**   It was only tested on openSUSE Tumbleweed. Please post issues if you have problems on other distributions.
**Notice**      Need gcc 4.8 or higher because of  some C++11 syntax

##### **Preparation**

1\. Download & Install [Qt 5.5.1](http://download.qt.io/archive/qt/5.5/5.5.1/)

2\. 
*For Debian/Ubuntu user*:

```
sudo aptitude install build-essential libgl1-mesa-dev libglu1-mesa-dev
```

##### **Compile**

```
cd BtQt/
mkdir build
cd build/
/path/to/qtbin/qmake ../BtQt.pro
make release
```

"/path/to/qtbin/" is "/path/to/Qt5.5.1/5.5/gcc_64/bin/" on my machine.

##### **Install**


#### Windows

**Attention**   It was only tested on Windows 10. Please post issues if you have problems on other distributions.

##### **Preparation & Compile**

1\. Download and install Visual studio 2015 (must install msvc2013) or Visual studio 2013

2\. Download and install Qt5.5.1, set the build toolkit with msvc2013(v120)

3\. Checkout the windows branch(maybe I will merge it into master branch someday), **open NePlay.pro with QtCreator**

4\. Compile. There are some little problems when running debug version, but you could fix it by modifying macro "DebugQmlSrc" in main.cpp.


##### **Install**


### License

BtQt is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser Public License as published by the Free Software Foundation; either version 2.1 of the license, or(at your option) any later version.

You should have received a copy of the GNU Lesser General Public License along with this program. If not, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses/).
