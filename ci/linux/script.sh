#!/bin/bash
set -ev

# Configure and build with Qt5
pushd build
$QTBIN/qmake ..
make
popd

# Prepare for deployment
pushd install-qt5
popd

# Test run

