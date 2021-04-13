TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.c \
    ../../dcomblockrx.c \
    ../../dcomblocktx.c \
    ../../dcomcallback.c \
    ../../dcomcommon.c \
    ../../dcomparam.c \
    ../../dcomrx.c \
    ../../dcomrxcon.c \
    ../../dcomtx.c \
    ../../dcomwaitlist.c \
    ../../lib/crc16-clang/crc16.c \
    ../../lib/lagan-clang/lagan.c \
    ../../lib/tzlist/tzlist.c \
    ../../lib/tzmalloc/bget.c \
    ../../lib/tzmalloc/tzmalloc.c \
    ../../lib/tztime/tztime.c \
    ../../dcom.c \
    ../../dcomlog.c

INCLUDEPATH += ../../ \
    ../../lib/crc16-clang \
    ../../lib/lagan-clang \
    ../../lib/tzlist \
    ../../lib/tzmalloc \
    ../../lib/tztime \
    ../../lib/pt

HEADERS += \
    ../../dcomblockrx.h \
    ../../dcomblocktx.h \
    ../../dcomcallback.h \
    ../../dcomcommon.h \
    ../../dcomparam.h \
    ../../dcomrx.h \
    ../../dcomrxcon.h \
    ../../dcomtx.h \
    ../../dcomwaitlist.h \
    ../../lib/crc16-clang/crc16.h \
    ../../lib/lagan-clang/lagan.h \
    ../../lib/pt/lc.h \
    ../../lib/pt/lc-switch.h \
    ../../lib/pt/pt.h \
    ../../lib/pt/pt-sem.h \
    ../../lib/tzlist/tzlist.h \
    ../../lib/tzmalloc/bget.h \
    ../../lib/tzmalloc/tzmalloc.h \
    ../../lib/tztime/tztime.h \
    ../../dcom.h \
    ../../dcomlog.h \
    ../../dcomprotocol.h

# disable C4819 warning
QMAKE_CXXFLAGS_WARN_ON += -Wimplicit-fallthrough=
