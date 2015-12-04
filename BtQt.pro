TEMPLATE = app

CONFIG += debug_and_release \

# Use enviroment variable
QMAKE_CXX = $$(CXX)
isEmpty(QMAKE_CXX) {
    QMAKE_CXX = g++
}
QMAKE_LINK = $$(LINK)
isEmpty(QMAKE_LINK) {
    QMAKE_LINK = g++
}

# Add rpaht-link for build in container
QMAKE_LFLAGS += -Wl,-rpath-link,$$(QTHOME)/lib

CONFIG(debug, debug|release) {
    CONFIG += console
    TARGET = BtQtDebug
    QMAKE_CXXFLAGS_DEBUG += -std=c++11

    SOURCES += test/main.cpp
} else {
    TARGET = BtQt
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE += -Ofast -flto -fno-strict-aliasing -std=c++11
    QMAKE_LFLAGS_RELEASE -= -Wl,-O1
    QMAKE_LFLAGS_RELEASE += -Ofast -flto 
    #QMAKE_LFLAGS_RELEASE += -Wl,-rpath,. -Wl,-rpath,/usr/lib64/ \
    #-Wl,-rpath,/usr/lib

    SOURCES += src/main.cpp
    RESOURCES += ui/qml/qml.qrc
}

QT += qml quick network core

SOURCES += src/BtBencode.cpp \
        src/BtTorrent.cpp \
        src/BtTracker.cpp \
        src/BtPeer.cpp \
        src/BtCore.cpp \
        src/QBitTorrent.cpp \

HEADERS += include/BtBencode.h \
        include/BtTorrent.h \
        include/BtTracker.h \
        include/BtPeer.h \
        include/BtCore.h \
        include/BtDefs.h \
        include/BtDebug.h \
        include/QBitTorrent.h \

unix:!macx: {
# Static librarys
    #LIBS += -Wl,-Bstatic -lcryptopp
# Dynamic librarys
    #LIBS += -Wl,-Bdynamic
}

INCLUDEPATH += include/ \
            /usr/include/

# OTHER_FILES += qml/main.qml

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

