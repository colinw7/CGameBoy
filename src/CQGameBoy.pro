TEMPLATE = app

QT += widgets

TARGET = CQGameBoy

DEPENDPATH += .

MOC_DIR = .moc

INCLUDEPATH += . ../include

QMAKE_CXXFLAGS += -std=c++11 -DGAMEBOY_Z80 -DCL_PARSER

CONFIG += debug

# Input
SOURCES += \
CGameBoy.cpp \
CQGameBoy.cpp \
CQGameBoyDbg.cpp \
CQGameBoyHexEdit.cpp \
CQGameBoyInterrupt.cpp \
CQGameBoyKeys.cpp \
CQGameBoyMain.cpp \
CQGameBoyScreen.cpp \
CQGameBoyTimer.cpp \
CQGameBoyVideo.cpp \
CQZ80Dbg.cpp \
CZ80.cpp \
CZ80Assemble.cpp \
CZ80DebugData.cpp \
CZ80Disassemble.cpp \
CZ80Execute.cpp \
CZ80Fn.cpp \
CZ80LabelData.cpp \
CZ80Load.cpp \
CZ80Op.cpp \
CZ80OpData.cpp \
CZ80Rst.cpp \
CZ80Snapshot.cpp \
CZ80SpeedData.cpp \

HEADERS += \
CGameBoy.h \
CQGameBoy.h \
CQGameBoyDbg.h \
CQGameBoyHexEdit.h \
CQGameBoyInterrupt.h \
CQGameBoyKeys.h \
CQGameBoyScreen.h \
CQGameBoyTimer.h \
CQGameBoyVideo.h \
CQZ80Dbg.h \
CZ80.h \
CZ80Assemble.h \
CZ80DebugData.h \
CZ80Disassemble.h \
CZ80Execute.h \
CZ80Fn.h \
CZ80LabelData.h \
CZ80Load.h \
CZ80Op.h \
CZ80OpData.h \
CZ80OpP.h \
CZ80Rst.h \
CZ80Snapshot.h \
CZ80SpeedData.h \

DESTDIR     = ../bin
OBJECTS_DIR = ../obj
LIB_DIR     = ../lib

INCLUDEPATH += \
. ../include \
../../CQUtil/include \
../../CCeil/include \
../../CArgs/include \
../../CFont/include \
../../CImageLib/include \
../../CFile/include \
../../CMath/include \
../../CGlob/include \
../../CStrUtil/include \
../../COS/include \
../../CUtil/include \

unix:LIBS += \
-L$$LIB_DIR \
-L../../CCeil/lib \
-L../../CQUtil/lib \
-L../../CFont/lib \
-L../../CImageLib/lib \
-L../../CConfig/lib \
-L../../CArgs/lib \
-L../../CFile/lib \
-L../../CFileUtil/lib \
-L../../CMath/lib \
-L../../CStrUtil/lib \
-L../../CUtil/lib \
-L../../COS/lib \
-L../../CGlob/lib \
-L../../CRegExp/lib \
-lCCeil -lCQUtil -lCFont -lCImageLib -lCConfig -lCArgs -lCFile -lCFileUtil \
-lCMath -lCUtil -lCOS -lCStrUtil -lCGlob -lCRegExp \
-lpng -ljpeg -ltre
