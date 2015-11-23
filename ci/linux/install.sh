#!/bin/bash
set -ev

# Install other dependencies
pushd dependencies

# Install Qt 5.5.1
wget http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_54/qt.54.gcc_64/5.4.2-0qt5_essentials.7z
wget http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_54/qt.54.gcc_64/5.4.2-0qt5_essentials.7z.sha1
wget http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_54/qt.54.gcc_64/5.4.2-0icu_53_1_ubuntu_11_10_64.7z
wget http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_54/qt.54.gcc_64/5.4.2-0icu_53_1_ubuntu_11_10_64.7z.sha1

7z x 5.4.2-0qt5_essentials.7z > /dev/null
7z x 5.4.2-0icu_53_1_ubuntu_11_10_64.7z > /dev/null

export CC="gcc-4.8"
export CXX="g++-4.8"
export PATH=$PWD/5.5/gcc_64/bin:$PATH

popd