TEMPLATE = app

QT += widgets

TARGET = CQGameBoy

DEPENDPATH += .

MOC_DIR = .moc

INCLUDEPATH += . ../include

QMAKE_CXXFLAGS += -std=c++14 -DGAMEBOY_Z80 -DCL_PARSER

CONFIG += debug

# Input
SOURCES += \
CQGameBoy.cpp \
CQGameBoyDbg.cpp \
CQGameBoyGraphics.cpp \
CQGameBoyHexEdit.cpp \
CQGameBoyInfo.cpp \
CQGameBoyInterrupt.cpp \
CQGameBoyKeys.cpp \
CQGameBoyMain.cpp \
CQGameBoyMemoryMap.cpp \
CQGameBoyPalette.cpp \
CQGameBoyScreen.cpp \
CQGameBoySpriteList.cpp \
CQGameBoySprites.cpp \
CQGameBoyTile.cpp \
CQGameBoyTiles.cpp \
CQGameBoyTimer.cpp \
CQGameBoyVideo.cpp \
CQGameBoyVReg.cpp \
\
CGameBoy.cpp \
CGameBoyExecData.cpp \
CGameBoyMemData.cpp \
CGameBoyPortData.cpp \
\
CQZ80Dbg.cpp \
CQZ80Memory.cpp \
CQZ80Instructions.cpp \
CQZ80Stack.cpp \
CQZ80TraceBack.cpp \
CQZ80RegEdit.cpp \
CQZ80FlagCheck.cpp \
\
CZ80.cpp \
CZ80Assemble.cpp \
CZ80DebugData.cpp \
CZ80Disassemble.cpp \
CZ80Execute.cpp \
CZ80Fn.cpp \
CZ80LabelData.cpp \
CZ80Load.cpp \
CZ80MemData.cpp \
CZ80Op.cpp \
CZ80OpData.cpp \
CZ80Rst.cpp \
CZ80Snapshot.cpp \
CZ80SpeedData.cpp \
\
CQHexdump.cpp \

HEADERS += \
CQGameBoy.h \
CQGameBoyDbg.h \
CQGameBoyGraphics.h \
CQGameBoyHexEdit.h \
CQGameBoyInfo.h \
CQGameBoyInterrupt.h \
CQGameBoyKeys.h \
CQGameBoyMemoryMap.h \
CQGameBoyPalette.h \
CQGameBoyScreen.h \
CQGameBoySpriteList.h \
CQGameBoySprites.h \
CQGameBoyTile.h \
CQGameBoyTiles.h \
CQGameBoyTimer.h \
CQGameBoyVideo.h \
CQGameBoyVReg.h \
\
CGameBoy.h \
CGameBoyExecData.h \
CGameBoyMemData.h \
CGameBoyPortData.h \
\
CQZ80Dbg.h \
CQZ80Memory.h \
CQZ80Instructions.h \
CQZ80Stack.h \
CQZ80TraceBack.h \
CQZ80RegEdit.h \
CQZ80FlagCheck.h \
\
CZ80AssembleData.h \
CZ80Assemble.h \
CZ80DebugData.h \
CZ80Disassemble.h \
CZ80ExecData.h \
CZ80Execute.h \
CZ80Fn.h \
CZ80.h \
CZ80LabelData.h \
CZ80Load.h \
CZ80Macro.h \
CZ80MemData.h \
CZ80OpData.h \
CZ80Op.h \
CZ80OpP.h \
CZ80PortData.h \
CZ80Reg.h \
CZ80Registers.h \
CZ80RstData.h \
CZ80Rst.h \
CZ80Screen.h \
CZ80Snapshot.h \
CZ80SpeedData.h \
CZ80Trace.h \
CZ80Types.h \
\
CQHexdump.h \

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
