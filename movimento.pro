QT += core widgets gui serialport charts

TARGET = movimento
TARGET_DIR = Movimento
TEMPLATE = app
EXTENSION = exe
DEPLOYMENT_COMMAND = windeployqt

INCLUDEPATH += include
DISTFILES += res/movimento.rc \
             arduino/movimento.ino \
             LICENSE \
             README.md

HEADERS += include/movimento.h \
           include/data.h \
           include/common.h

SOURCES += src/main.cpp \
           src/movimento.cpp \
           src/data.cpp \
           src/common.cpp

FORMS += res/ui/movimento.ui

RESOURCES += res/movimento.qrc
RC_FILE = $$DISTFILES

CONFIG(debug, debug|release) {
    DESTDIR = debug/$${TARGET_DIR}
}
else {
    DESTDIR = release/$${TARGET_DIR}
    QMAKE_POST_LINK = $${DEPLOYMENT_COMMAND} --compiler-runtime $${DESTDIR}/$${TARGET}.$${EXTENSION}
}

OBJECTS_DIR += $$DESTDIR
MOC_DIR += $$DESTDIR
UI_DIR += $$DESTDIR
RCC_DIR += $$DESTDIR
