#ifndef CQZ80_DBG_H
#define CQZ80_DBG_H

#include <QWidget>
#include <CZ80.h>

#include <string>
#include <set>

class CQZ80Dbg;
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

class CQZ80MemLine {
 public:
  CQZ80MemLine(ushort pc=0, const std::string &pcStr="", const std::string &memStr="",
               const std::string textStr="") :
   pc_(pc), pcStr_(pcStr), memStr_(memStr), textStr_(textStr) {
  }

  ushort pc() const { return pc_; }

  uint num() const { return pc_ / 8; }

  const std::string &pcStr  () const { return pcStr_  ; }
  const std::string &memStr () const { return memStr_ ; }
  const std::string &textStr() const { return textStr_; }

 private:
  ushort      pc_ { 0 };
  std::string pcStr_;
  std::string memStr_;
  std::string textStr_;
};

//------

class CQZ80Mem : public QWidget {
  Q_OBJECT

 public:
  CQZ80Mem(CQZ80Dbg *dbg);

  void setFont(const QFont &font);

  void setLine(uint i, const std::string &pcStr, const std::string &memStr,
               const std::string &textStr);

  void contextMenuEvent(QContextMenuEvent *event);

 public slots:
  void sliderSlot(int y);

  void dumpSlot();

 private:
  void paintEvent(QPaintEvent *);

  void mouseDoubleClickEvent(QMouseEvent *e);

 private:
  typedef std::vector<CQZ80MemLine> LineList;

  CQZ80Dbg* dbg_        { nullptr };
  LineList  lines_;
  int       yOffset_    { 0 };
  int       charWidth_  { 8 };
  int       charHeight_ { 12 };
  int       dx_         { 2 };
};

//------

class CQZ80InstLine {
 public:
  CQZ80InstLine(uint pc=0, const std::string &pcStr="", const std::string &codeStr="",
                const std::string textStr="") :
   pc_(pc), pcStr_(pcStr), codeStr_(codeStr), textStr_(textStr) {
  }

  ushort pc() const { return pc_; }

  uint num() const { return pc_ / 8; }

  const std::string &pcStr  () const { return pcStr_  ; }
  const std::string &codeStr() const { return codeStr_; }
  const std::string &textStr() const { return textStr_; }

 private:
  uint        pc_ { 0 };
  std::string pcStr_;
  std::string codeStr_;
  std::string textStr_;
};

//------

class CQZ80Inst : public QWidget {
  Q_OBJECT

 public:
  CQZ80Inst(CQZ80Dbg *dbg);

  void setVBar(QScrollBar *vbar) { vbar_ = vbar; }

  void setFont(const QFont &font);

  void clear();

  void setLine(uint pc, const std::string &pcStr, const std::string &codeStr,
               const std::string &textStr);

  uint getNumLines() const { return lineNum_; }

  bool getLineForPC(uint pc, uint &lineNum) const;
  uint getPCForLine(uint lineNum);

  void clearBreakpoints() { breakpoints_.clear(); }

  void addBreakPoint(uint pc) { breakpoints_.insert(pc); }

  void mouseDoubleClickEvent(QMouseEvent *e);

  void contextMenuEvent(QContextMenuEvent *event);

  void reload();

 public slots:
  void sliderSlot(int y);

  void dumpSlot();

  void reloadSlot();

 private:
  void paintEvent(QPaintEvent *);

 private:
  typedef std::vector<CQZ80InstLine> LineList;
  typedef std::map<uint,uint>        PCLineMap;
  typedef std::set<uint>             BreakpointList;

  CQZ80Dbg*      dbg_        { nullptr };
  QScrollBar*    vbar_       { nullptr };
  LineList       lines_;
  int            yOffset_    { 0 };
  int            charHeight_ { 8 };
  int            lineNum_    { 0 };
  PCLineMap      pcLineMap_;
  PCLineMap      linePcMap_;
  BreakpointList breakpoints_;
};

//------

class CQZ80RegEdit : public QWidget {
  Q_OBJECT

 public:
  CQZ80RegEdit(CQZ80Dbg *dbg, CZ80Reg reg);

  void setFont(const QFont &font);

  void setValue(uint value);
  uint getValue() const;

 private slots:
  void valueChangedSlot();

 private:
  CQZ80Dbg  *dbg_   { nullptr };
  CZ80Reg    reg_;
  QLabel    *label_ { nullptr };
  QLineEdit *edit_  { nullptr };
};

//------

class CQZ80Dbg : public QWidget, public CZ80Trace {
  Q_OBJECT

  Q_PROPERTY(QFont  fixedFont         READ getFixedFont        WRITE setFixedFont)
  Q_PROPERTY(int    numMemoryLines    READ getNumMemoryLines   WRITE setNumMemoryLines)
  Q_PROPERTY(bool   memoryTrace       READ isMemoryTrace       WRITE setMemoryTrace)
  Q_PROPERTY(bool   instructionsTrace READ isInstructionsTrace WRITE setInstructionsTrace)
  Q_PROPERTY(bool   registersTrace    READ isRegistersTrace    WRITE setRegistersTrace)
  Q_PROPERTY(bool   flagsTrace        READ isFlagsTrace        WRITE setFlagsTrace)
  Q_PROPERTY(bool   stackTrace        READ isStackTrace        WRITE setStackTrace)
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

  void setInstructionsText();

  void setStackText();

  void setBreakpointText();

  //QLineEdit *createRegisterEdit();

  //----

 protected:
  void postStepProc();

  void regChangedI(CZ80Reg reg);

  void memChanged(ushort pos, ushort len) override;

  void memChangedI(ushort pos, ushort len);

  void breakpointsChanged();

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
  void breakpointsTraceSlot();

  void setTraceSlot();

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
  QTextEdit  *stackText_  { nullptr };

  QGroupBox   *breakpointsGroup_  { nullptr };
  QVBoxLayout *breakpointsLayout_ { nullptr };
  QTextEdit   *breakpointsText_   { nullptr };
  QLineEdit   *breakpointsEdit_   { nullptr };

  QCheckBox   *traceCheck_ { nullptr };

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
