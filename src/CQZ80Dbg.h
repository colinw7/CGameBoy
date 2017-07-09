#ifndef CQZ80_DBG_H
#define CQZ80_DBG_H

#include <CZ80Trace.h>
#include <CZ80.h>
#include <QFrame>

#include <string>
#include <set>

class CQZ80Dbg;
class CQZ80Mem;
class CQZ80Inst;
class CQZ80Stack;
class CQZ80TraceBack;
class CQZ80RegEdit;
class CZ80;

class QGroupBox;
class QFrame;
class QTextEdit;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QLabel;
class QScrollBar;
class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;

//------

class CQZ80Dbg : public QFrame, public CZ80Trace {
  Q_OBJECT

  Q_PROPERTY(QFont  fixedFont         READ getFixedFont        WRITE setFixedFont)
  Q_PROPERTY(int    numMemoryLines    READ getNumMemoryLines   WRITE setNumMemoryLines)
  Q_PROPERTY(bool   memoryTrace       READ isMemoryTrace       WRITE setMemoryTrace)
  Q_PROPERTY(bool   instructionsTrace READ isInstructionsTrace WRITE setInstructionsTrace)
  Q_PROPERTY(bool   registersTrace    READ isRegistersTrace    WRITE setRegistersTrace)
  Q_PROPERTY(bool   flagsTrace        READ isFlagsTrace        WRITE setFlagsTrace)
  Q_PROPERTY(bool   stackTrace        READ isStackTrace        WRITE setStackTrace)
  Q_PROPERTY(bool   traceBackTrace    READ isTraceBackTrace    WRITE setTraceBackTrace)
  Q_PROPERTY(bool   breakpointsTrace  READ isBreakpointsTrace  WRITE setBreakpointsTrace)
  Q_PROPERTY(QColor addrColor         READ addrColor           WRITE setAddrColor)
  Q_PROPERTY(QColor memDataColor      READ memDataColor        WRITE setMemDataColor)
  Q_PROPERTY(QColor memCharsColor     READ memCharsColor       WRITE setMemCharsColor)
  Q_PROPERTY(QColor currentColor      READ currentColor        WRITE setCurrentColor)
  Q_PROPERTY(QColor readOnlyBgColor   READ readOnlyBgColor     WRITE setReadOnlyBgColor)
  Q_PROPERTY(QColor screenBgColor     READ screenBgColor       WRITE setScreenBgColor)

 public:
  CQZ80Dbg(CZ80 *z80);

  virtual ~CQZ80Dbg();

  virtual void init();

  CZ80 *getZ80() const { return z80_; }

  void regChanged(CZ80Reg reg) override;

  const QFont &getFixedFont() const { return fixedFont_; }
  void setFixedFont(const QFont &font);

  int getNumMemoryLines() const { return numMemoryLines_; }
  void setNumMemoryLines(int i) { numMemoryLines_ = i; }

  bool isMemoryTrace() const { return memoryTrace_; }
  void setMemoryTrace(bool b);

  bool isInstructionsTrace() const { return instructionsTrace_; }
  void setInstructionsTrace(bool b) { instructionsTrace_ = b; }

  bool isRegistersTrace() const { return registersTrace_; }
  void setRegistersTrace(bool b) { registersTrace_ = b; }

  bool isFlagsTrace() const { return flagsTrace_; }
  void setFlagsTrace(bool b) { flagsTrace_ = b; }

  bool isStackTrace() const { return stackTrace_; }
  void setStackTrace(bool b) { stackTrace_ = b; }

  bool isTraceBackTrace() const { return traceBackTrace_; }
  void setTraceBackTrace(bool b) { traceBackTrace_ = b; }

  bool isBreakpointsTrace() const { return breakpointsTrace_; }
  void setBreakpointsTrace(bool b) { breakpointsTrace_ = b; }

  const QColor &addrColor() const { return addrColor_; }
  void setAddrColor(const QColor &c) { addrColor_ = c; }

  const QColor &memDataColor() const { return memDataColor_; }
  void setMemDataColor(const QColor &c) { memDataColor_ = c; }

  const QColor &memCharsColor() const { return memCharsColor_; }
  void setMemCharsColor(const QColor &c) { memCharsColor_ = c; }

  const QColor &currentColor() const { return currentColor_; }
  void setCurrentColor(const QColor &c) { currentColor_ = c; }

  const QColor &readOnlyBgColor() const { return readOnlyBgColor_; }
  void setReadOnlyBgColor(const QColor &c) { readOnlyBgColor_ = c; }

  const QColor &screenBgColor() const { return screenBgColor_; }
  void setScreenBgColor(const QColor &c) { screenBgColor_ = c; }

 protected:
  virtual void addWidgets();

  virtual void addFlagsWidgets();

  virtual void addRegistersWidgets();

  virtual void addBreakpointWidgets();

  virtual void addButtonsWidgets();

  QPushButton *addButtonWidget(const QString &name, const QString &label);

  void setMemoryText();
  void setMemoryLine(uint pos);

  std::string getByteChar(uchar c);

  void updateInstructions();

  void updateStack();

  void updateTraceBack();

  void updateBreakpoints();

  //QLineEdit *createRegisterEdit();

  //----

 protected:
  void postStepProc() override;

  void memChanged(ushort pos, ushort len) override;

  void memChangedI(ushort pos, ushort len);

  void traceBackChanged() override;

  void breakpointsChanged() override;

  void setStop(bool b) override;
  void setHalt(bool b) override;

  void updateAll();

 protected slots:
  void addBreakpointSlot();
  void deleteBreakpointSlot();
  void clearBreakpointSlot();

  void memoryTraceSlot();
  void instructionsTraceSlot();
  void registersTraceSlot();
  void flagsTraceSlot();
  void stackTraceSlot();
  void traceBackTraceSlot();
  void breakpointsTraceSlot();

  void setTraceSlot();
  void setHaltSlot();

  void runSlot();
  void nextSlot();
  void stepSlot();
  void continueSlot();
  void stopSlot();
  void restartSlot();
  void exitSlot();

 protected:
  CZ80  *z80_ { nullptr };
  QFont  fixedFont_;

  int numMemoryLines_ { 20 };

  bool follow_    { false };
  bool followMem_ { false };

  bool memoryTrace_       { true };
  bool instructionsTrace_ { true };
  bool registersTrace_    { true };
  bool flagsTrace_        { true };
  bool stackTrace_        { true };
  bool traceBackTrace_    { true };
  bool breakpointsTrace_  { true };

  bool memoryDirty_ { false };

  QColor addrColor_       {   0,   0, 220 };
  QColor memDataColor_    {   0,   0,   0 };
  QColor memCharsColor_   {   0, 220,   0 };
  QColor currentColor_    { 220,   0,   0 };
  QColor readOnlyBgColor_ { 216, 180, 180 };
  QColor screenBgColor_   { 180, 216, 180 };

  QGroupBox  *memoryGroup_ { nullptr };
  CQZ80Mem   *memoryText_  { nullptr };
  QScrollBar *memoryVBar_  { nullptr };

  QGroupBox  *instructionsGroup_ { nullptr };
  CQZ80Inst  *instructionsText_  { nullptr };
  QScrollBar *instructionsVBar_  { nullptr };
  QLineEdit  *opData_            { nullptr };

  QGroupBox    *registersGroup_  { nullptr };
  QGridLayout  *registersLayout_ { nullptr };
  CQZ80RegEdit *afEdit_          { nullptr };
  CQZ80RegEdit *af1Edit_         { nullptr };
  CQZ80RegEdit *bcEdit_          { nullptr };
  CQZ80RegEdit *bc1Edit_         { nullptr };
  CQZ80RegEdit *deEdit_          { nullptr };
  CQZ80RegEdit *de1Edit_         { nullptr };
  CQZ80RegEdit *hlEdit_          { nullptr };
  CQZ80RegEdit *hl1Edit_         { nullptr };
  CQZ80RegEdit *ixEdit_          { nullptr };
  CQZ80RegEdit *iEdit_           { nullptr };
  CQZ80RegEdit *iyEdit_          { nullptr };
  CQZ80RegEdit *rEdit_           { nullptr };
  CQZ80RegEdit *spEdit_          { nullptr };
  CQZ80RegEdit *iffEdit_         { nullptr };
  CQZ80RegEdit *pcEdit_          { nullptr };

  QGroupBox   *flagsGroup_  { nullptr };
  QGridLayout *flagsLayout_ { nullptr };
  QCheckBox   *sFlagCheck_  { nullptr };
  QCheckBox   *zFlagCheck_  { nullptr };
  QCheckBox   *yFlagCheck_  { nullptr };
  QCheckBox   *hFlagCheck_  { nullptr };
  QCheckBox   *xFlagCheck_  { nullptr };
  QCheckBox   *pFlagCheck_  { nullptr };
  QCheckBox   *nFlagCheck_  { nullptr };
  QCheckBox   *cFlagCheck_  { nullptr };

  QGroupBox  *stackGroup_ { nullptr };
  CQZ80Stack *stackText_  { nullptr };

  QGroupBox      *traceBackGroup_ { nullptr };
  CQZ80TraceBack *traceBack_      { nullptr };

  QGroupBox   *breakpointsGroup_  { nullptr };
  QVBoxLayout *breakpointsLayout_ { nullptr };
  QTextEdit   *breakpointsText_   { nullptr };
  QLineEdit   *breakpointsEdit_   { nullptr };

  QCheckBox   *traceCheck_ { nullptr };
  QCheckBox   *haltCheck_  { nullptr };

  QFrame      *buttonsToolbar_ { nullptr };
  QHBoxLayout *buttonsLayout_  { nullptr };
  QPushButton *runButton_      { nullptr };
  QPushButton *nextButton_     { nullptr };
  QPushButton *stepButton_     { nullptr };
  QPushButton *continueButton_ { nullptr };
  QPushButton *stopButton_     { nullptr };
  QPushButton *restartButton_  { nullptr };
  QPushButton *exitButton_     { nullptr };
};

#endif
