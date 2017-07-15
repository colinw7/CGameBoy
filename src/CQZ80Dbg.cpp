#include <CQZ80Dbg.h>
#include <CQZ80Memory.h>
#include <CQZ80Instructions.h>
#include <CQZ80Stack.h>
#include <CQZ80TraceBack.h>
#include <CQZ80RegEdit.h>
#include <CZ80.h>
#include <CZ80OpData.h>

#include <QApplication>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QGroupBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QScrollBar>
#include <QMouseEvent>
#include <cassert>

CQZ80Dbg::
CQZ80Dbg(CZ80 *z80) :
 CZ80Trace(*z80), z80_(z80)
{
  setObjectName("dbg");

  setWindowTitle("Z80 Emulator (Debug)");

  z80_->addTrace(this);
}

CQZ80Dbg::
~CQZ80Dbg()
{
}

void
CQZ80Dbg::
init()
{
  addWidgets();

  setMemoryText();

  updateInstructions();

  updateStack();

  updateTraceBack();

  updateBreakpoints();

  regChanged(CZ80Reg::NONE);
}

void
CQZ80Dbg::
setFixedFont(const QFont &font)
{
  fixedFont_ = font;

  memoryText_      ->setFont(getFixedFont());
  instructionsText_->setFont(getFixedFont());
  stackText_       ->setFont(getFixedFont());
  traceBack_       ->setFont(getFixedFont());
  breakpointsText_ ->setFont(getFixedFont());

  afEdit_ ->setFont(getFixedFont());
  af1Edit_->setFont(getFixedFont());
  bcEdit_ ->setFont(getFixedFont());
  bc1Edit_->setFont(getFixedFont());
  deEdit_ ->setFont(getFixedFont());
  de1Edit_->setFont(getFixedFont());
  hlEdit_ ->setFont(getFixedFont());
  hl1Edit_->setFont(getFixedFont());
  ixEdit_ ->setFont(getFixedFont());
  iEdit_  ->setFont(getFixedFont());
  iyEdit_ ->setFont(getFixedFont());
  rEdit_  ->setFont(getFixedFont());
  spEdit_ ->setFont(getFixedFont());
  iffEdit_->setFont(getFixedFont());
  pcEdit_ ->setFont(getFixedFont());
}

void
CQZ80Dbg::
setMemoryTrace(bool b)
{
  memoryTrace_ = b;

  if (memoryTrace_ && memoryDirty_) {
    memChangedI(0, 65535);
  }
}

void
CQZ80Dbg::
addWidgets()
{
  fixedFont_ = QFont("Courier", 10);

  QFontMetrics fm(getFixedFont());

  //----

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  QWidget *topFrame    = new QWidget;
  QWidget *bottomFrame = new QWidget;

  topFrame   ->setObjectName("topFrame");
  bottomFrame->setObjectName("bottomFrame");

  QHBoxLayout *topLayout    = new QHBoxLayout(topFrame);
  QVBoxLayout *bottomLayout = new QVBoxLayout(bottomFrame);

  topLayout   ->setMargin(2); topLayout   ->setSpacing(2);
  bottomLayout->setMargin(2); bottomLayout->setSpacing(2);

  layout->addWidget(topFrame);
  layout->addWidget(bottomFrame);

  //----

  QWidget *leftFrame  = new QWidget;
  QWidget *rightFrame = new QWidget;

  leftFrame ->setObjectName("leftFrame");
  rightFrame->setObjectName("rightFrame");

  topLayout->addWidget(leftFrame);
  topLayout->addWidget(rightFrame);

  QVBoxLayout *leftLayout  = new QVBoxLayout(leftFrame );
  QVBoxLayout *rightLayout = new QVBoxLayout(rightFrame);

  leftLayout ->setMargin(2); leftLayout ->setSpacing(2);
  rightLayout->setMargin(2); rightLayout->setSpacing(2);

  //----

  memoryGroup_ = new QGroupBox("Memory");

  memoryGroup_->setObjectName("memoryGroup");
  memoryGroup_->setCheckable(true);

  connect(memoryGroup_, SIGNAL(toggled(bool)), this, SLOT(memoryTraceSlot()));

  QHBoxLayout *memoryLayout = new QHBoxLayout(memoryGroup_);

  memoryText_ = new CQZ80Mem(this);

  memoryText_->setFont(getFixedFont());

  memoryVBar_ = new QScrollBar;

  memoryVBar_->setObjectName("memoryVbar");
  memoryVBar_->setPageStep  (getNumMemoryLines());
  memoryVBar_->setSingleStep(1);
  memoryVBar_->setRange     (0, 8192 - memoryVBar_->pageStep());

  connect(memoryVBar_, SIGNAL(valueChanged(int)), memoryText_, SLOT(sliderSlot(int)));

  memoryLayout->addWidget(memoryText_);
  memoryLayout->addWidget(memoryVBar_);
  memoryLayout->addStretch();

  leftLayout->addWidget(memoryGroup_);

  //--

  instructionsGroup_ = new QGroupBox("Instructions");

  instructionsGroup_->setObjectName("instructionsGroup");
  instructionsGroup_->setCheckable(true);

  connect(instructionsGroup_, SIGNAL(toggled(bool)), this, SLOT(instructionsTraceSlot()));

  QHBoxLayout *instructionsLayout = new QHBoxLayout(instructionsGroup_);

  instructionsText_ = new CQZ80Inst(this);

  instructionsText_->setFont(getFixedFont());

  instructionsVBar_ = new QScrollBar;

  instructionsVBar_->setObjectName("instructionsVbar");
  instructionsVBar_->setPageStep  (getNumMemoryLines());
  instructionsVBar_->setSingleStep(1);
  instructionsVBar_->setRange     (0, 8192 - instructionsVBar_->pageStep());

  connect(instructionsVBar_, SIGNAL(valueChanged(int)),
          instructionsText_, SLOT(sliderSlot(int)));

  instructionsLayout->addWidget(instructionsText_);
  instructionsLayout->addWidget(instructionsVBar_);
  instructionsLayout->addStretch();

  instructionsText_->setVBar(instructionsVBar_);

  leftLayout->addWidget(instructionsGroup_);

  opData_ = new QLineEdit;

  opData_->setObjectName("opData");

  opData_->setReadOnly(true);

  leftLayout->addWidget(opData_);

  //----

  registersGroup_ = new QGroupBox("Registers");

  registersGroup_->setObjectName("registersGroup");
  registersGroup_->setCheckable(true);

  connect(registersGroup_, SIGNAL(toggled(bool)), this, SLOT(registersTraceSlot()));

  registersLayout_ = new QGridLayout(registersGroup_);

  addRegistersWidgets();

  rightLayout->addWidget(registersGroup_);

  //--

  flagsGroup_ = new QGroupBox("Flags");

  flagsGroup_->setObjectName("flagsGroup");
  flagsGroup_->setCheckable(true);

  connect(flagsGroup_, SIGNAL(toggled(bool)), this, SLOT(flagsTraceSlot()));

  flagsLayout_ = new QGridLayout(flagsGroup_);

  flagsLayout_->setSpacing(6);

  addFlagsWidgets();

  rightLayout->addWidget(flagsGroup_);

  //--

  stackGroup_ = new QGroupBox("Stack");

  stackGroup_->setObjectName("stackGroup");
  stackGroup_->setCheckable(true);

  connect(stackGroup_, SIGNAL(toggled(bool)), this, SLOT(stackTraceSlot()));

  QVBoxLayout *stackLayout = new QVBoxLayout(stackGroup_);

  stackText_ = new CQZ80Stack(z80_);

  stackText_->setFixedFont(getFixedFont());

  stackLayout->addWidget(stackText_);

  rightLayout->addWidget(stackGroup_);

  //--

  traceBackGroup_ = new QGroupBox("Trace Back");

  traceBackGroup_->setObjectName("traceBackGroup");
  traceBackGroup_->setCheckable(true);

  connect(traceBackGroup_, SIGNAL(toggled(bool)), this, SLOT(traceBackTraceSlot()));

  QVBoxLayout *traceBackLayout = new QVBoxLayout(traceBackGroup_);

  traceBack_ = new CQZ80TraceBack(z80_);

  traceBack_->setFixedFont(getFixedFont());

  traceBackLayout->addWidget(traceBack_);

  rightLayout->addWidget(traceBackGroup_);

  //--

  breakpointsGroup_ = new QGroupBox("Breakpoints");

  breakpointsGroup_->setObjectName("breakpointsGroup");
  breakpointsGroup_->setCheckable(true);

  connect(breakpointsGroup_, SIGNAL(toggled(bool)), this, SLOT(breakpointsTraceSlot()));

  breakpointsLayout_ = new QVBoxLayout(breakpointsGroup_);

  addBreakpointWidgets();

  rightLayout->addWidget(breakpointsGroup_);

  //-----

  QFrame *optionsFrame = new QFrame;

  optionsFrame->setObjectName("optionsFrame");

  QHBoxLayout *optionsLayout = new QHBoxLayout(optionsFrame);

  //--

  traceCheck_ = new QCheckBox("Trace");

  traceCheck_->setObjectName("traceCheck");
  traceCheck_->setChecked(true);

  connect(traceCheck_, SIGNAL(stateChanged(int)), this, SLOT(setTraceSlot()));

  optionsLayout->addWidget(traceCheck_);

  //--

  haltCheck_ = new QCheckBox("Halt");

  haltCheck_->setObjectName("haltCheck");
  haltCheck_->setChecked(false);

  connect(haltCheck_, SIGNAL(stateChanged(int)), this, SLOT(setHaltSlot()));

  optionsLayout->addWidget(haltCheck_);

  //--

  optionsLayout->addStretch(1);

  bottomLayout->addWidget(optionsFrame);

  //---

  buttonsToolbar_ = new QFrame;

  buttonsToolbar_->setObjectName("buttonsToolbar");

  buttonsLayout_ = new QHBoxLayout(buttonsToolbar_);

  buttonsLayout_->addStretch(1);

  addButtonsWidgets();

  bottomLayout->addWidget(buttonsToolbar_);
}

void
CQZ80Dbg::
addFlagsWidgets()
{
  sFlagCheck_ = new QCheckBox("");
  zFlagCheck_ = new QCheckBox("");
  yFlagCheck_ = new QCheckBox("");
  hFlagCheck_ = new QCheckBox("");
  xFlagCheck_ = new QCheckBox("");
  pFlagCheck_ = new QCheckBox("");
  nFlagCheck_ = new QCheckBox("");
  cFlagCheck_ = new QCheckBox("");

  sFlagCheck_->setObjectName("sFlagCheck");
  zFlagCheck_->setObjectName("zFlagCheck");
  yFlagCheck_->setObjectName("yFlagCheck");
  hFlagCheck_->setObjectName("hFlagCheck");
  xFlagCheck_->setObjectName("xFlagCheck");
  pFlagCheck_->setObjectName("pFlagCheck");
  nFlagCheck_->setObjectName("nFlagCheck");
  cFlagCheck_->setObjectName("cFlagCheck");

  flagsLayout_->addWidget(new QLabel("S"), 0, 0); flagsLayout_->addWidget(sFlagCheck_, 1, 0);
  flagsLayout_->addWidget(new QLabel("Z"), 0, 1); flagsLayout_->addWidget(zFlagCheck_, 1, 1);
  flagsLayout_->addWidget(new QLabel("Y"), 0, 2); flagsLayout_->addWidget(yFlagCheck_, 1, 2);
  flagsLayout_->addWidget(new QLabel("H"), 0, 3); flagsLayout_->addWidget(hFlagCheck_, 1, 3);
  flagsLayout_->addWidget(new QLabel("X"), 0, 4); flagsLayout_->addWidget(xFlagCheck_, 1, 4);
  flagsLayout_->addWidget(new QLabel("P"), 0, 5); flagsLayout_->addWidget(pFlagCheck_, 1, 5);
  flagsLayout_->addWidget(new QLabel("N"), 0, 6); flagsLayout_->addWidget(nFlagCheck_, 1, 6);
  flagsLayout_->addWidget(new QLabel("C"), 0, 7); flagsLayout_->addWidget(cFlagCheck_, 1, 7);

  flagsLayout_->setColumnStretch(8, 1);
}

void
CQZ80Dbg::
addRegistersWidgets()
{
  afEdit_  = new CQZ80RegEdit(z80_, CZ80Reg::AF );
  af1Edit_ = new CQZ80RegEdit(z80_, CZ80Reg::AF1);
  bcEdit_  = new CQZ80RegEdit(z80_, CZ80Reg::BC );
  bc1Edit_ = new CQZ80RegEdit(z80_, CZ80Reg::BC1);
  deEdit_  = new CQZ80RegEdit(z80_, CZ80Reg::DE );
  de1Edit_ = new CQZ80RegEdit(z80_, CZ80Reg::DE1);
  hlEdit_  = new CQZ80RegEdit(z80_, CZ80Reg::HL );
  hl1Edit_ = new CQZ80RegEdit(z80_, CZ80Reg::HL1);
  ixEdit_  = new CQZ80RegEdit(z80_, CZ80Reg::IX );
  iEdit_   = new CQZ80RegEdit(z80_, CZ80Reg::I  );
  iyEdit_  = new CQZ80RegEdit(z80_, CZ80Reg::IY );
  rEdit_   = new CQZ80RegEdit(z80_, CZ80Reg::R  );
  spEdit_  = new CQZ80RegEdit(z80_, CZ80Reg::SP );
  iffEdit_ = new CQZ80RegEdit(z80_, CZ80Reg::IFF);
  pcEdit_  = new CQZ80RegEdit(z80_, CZ80Reg::PC );

  registersLayout_->addWidget(afEdit_ , 0, 0);
  registersLayout_->addWidget(af1Edit_, 0, 1);

  registersLayout_->addWidget(bcEdit_ , 1, 0);
  registersLayout_->addWidget(bc1Edit_, 1, 1);

  registersLayout_->addWidget(deEdit_ , 2, 0);
  registersLayout_->addWidget(de1Edit_, 2, 1);

  registersLayout_->addWidget(hlEdit_ , 3, 0);
  registersLayout_->addWidget(hl1Edit_, 3, 1);

  registersLayout_->addWidget(ixEdit_ , 4, 0);
  registersLayout_->addWidget(iEdit_  , 4, 1);

  registersLayout_->addWidget(iyEdit_ , 5, 0);
  registersLayout_->addWidget(rEdit_  , 5, 1);

  registersLayout_->addWidget(spEdit_ , 6, 0);
  registersLayout_->addWidget(iffEdit_, 6, 1);

  registersLayout_->addWidget(pcEdit_ , 7, 0);

  registersLayout_->setColumnStretch(2, 1);
}

void
CQZ80Dbg::
addBreakpointWidgets()
{
  breakpointsText_ = new QTextEdit;

  breakpointsText_->setObjectName("breakpointsText");
  breakpointsText_->setReadOnly(true);

  breakpointsText_->setFont(getFixedFont());

  breakpointsLayout_->addWidget(breakpointsText_);

  QFrame *breakpointEditFrame = new QFrame;

  breakpointEditFrame->setObjectName("breakpointEditFrame");

  breakpointsLayout_->addWidget(breakpointEditFrame);

  QHBoxLayout *breakpointEditLayout = new QHBoxLayout(breakpointEditFrame);
  breakpointEditLayout->setMargin(0); breakpointEditLayout->setSpacing(0);

  breakpointsEdit_ = new QLineEdit;

  breakpointEditLayout->addWidget(new QLabel("Addr"));
  breakpointEditLayout->addWidget(breakpointsEdit_);
  breakpointEditLayout->addStretch(1);

  QFrame *breakpointToolbar = new QFrame;

  breakpointToolbar->setObjectName("breakpointToolbar");

  QHBoxLayout *breakpointToolbarLayout = new QHBoxLayout(breakpointToolbar);
  breakpointToolbarLayout->setMargin(0); breakpointToolbarLayout->setSpacing(0);

  QPushButton *addBreakpointButton    = new QPushButton("Add"   );
  QPushButton *deleteBreakpointButton = new QPushButton("Delete");
  QPushButton *clearBreakpointButton  = new QPushButton("Clear" );

  addBreakpointButton   ->setObjectName("addBreakpointButton");
  deleteBreakpointButton->setObjectName("deleteBreakpointButton");
  clearBreakpointButton ->setObjectName("clearBreakpointButton");

  connect(addBreakpointButton   , SIGNAL(clicked()), this, SLOT(addBreakpointSlot   ()));
  connect(deleteBreakpointButton, SIGNAL(clicked()), this, SLOT(deleteBreakpointSlot()));
  connect(clearBreakpointButton , SIGNAL(clicked()), this, SLOT(clearBreakpointSlot ()));

  breakpointToolbarLayout->addWidget(addBreakpointButton);
  breakpointToolbarLayout->addWidget(deleteBreakpointButton);
  breakpointToolbarLayout->addWidget(clearBreakpointButton);
  breakpointToolbarLayout->addStretch(1);

  breakpointsLayout_->addWidget(breakpointToolbar);
}

void
CQZ80Dbg::
addButtonsWidgets()
{
  runButton_      = addButtonWidget("run"     , "Run");
  nextButton_     = addButtonWidget("next"    , "Next");
  stepButton_     = addButtonWidget("step"    , "Step");
  continueButton_ = addButtonWidget("continue", "Continue");
  stopButton_     = addButtonWidget("stop"    , "Stop");
  restartButton_  = addButtonWidget("restart" , "Restart");
  exitButton_     = addButtonWidget("exit"    , "Exit");

  connect(runButton_     , SIGNAL(clicked()), this, SLOT(runSlot()));
  connect(nextButton_    , SIGNAL(clicked()), this, SLOT(nextSlot()));
  connect(stepButton_    , SIGNAL(clicked()), this, SLOT(stepSlot()));
  connect(continueButton_, SIGNAL(clicked()), this, SLOT(continueSlot()));
  connect(stopButton_    , SIGNAL(clicked()), this, SLOT(stopSlot()));
  connect(restartButton_ , SIGNAL(clicked()), this, SLOT(restartSlot()));
  connect(exitButton_    , SIGNAL(clicked()), this, SLOT(exitSlot()));
}

QPushButton *
CQZ80Dbg::
addButtonWidget(const QString &name, const QString &label)
{
  QPushButton *button = new QPushButton(label);

  button->setObjectName(name);

  buttonsLayout_->addWidget(button);

  return button;
}

void
CQZ80Dbg::
setMemoryText()
{
  uint len = 65536;

  ushort numLines = len / 8;

  if ((len % 8) != 0) ++numLines;

  uint pos = z80_->getPC();

  z80_->setPC(0);

  std::string str;

  uint pos1 = 0;

  for (ushort i = 0; i < numLines; ++i) {
    setMemoryLine(pos1);

    pos1 += 8;
  }

  z80_->setPC(pos);
}

void
CQZ80Dbg::
setMemoryLine(uint pos)
{
  std::string pcStr = CStrUtil::toHexString(pos, 4);

  //-----

  std::string memStr;

  for (ushort j = 0; j < 8; ++j) {
    if (j > 0) memStr += " ";

    memStr += CStrUtil::toHexString(z80_->getByte(pos + j), 2);
  }

  std::string textStr;

  for (ushort j = 0; j < 8; ++j) {
    uchar c = z80_->getByte(pos + j);

    textStr += getByteChar(c);
  }

  memoryText_->setLine(pos, pcStr, memStr, textStr);
}

std::string
CQZ80Dbg::
getByteChar(uchar c)
{
  std::string str;

  if (c >= 0x20 && c < 0x7f)
    str += c;
  else
    str += '.';

  return str;
}

void
CQZ80Dbg::
updateInstructions()
{
  instructionsText_->reload();
}

void
CQZ80Dbg::
updateStack()
{
  stackText_->update();
}

void
CQZ80Dbg::
updateTraceBack()
{
  traceBack_->update();
}

void
CQZ80Dbg::
updateBreakpoints()
{
  breakpointsText_->clear();

  instructionsText_->clearBreakpoints();

  //----

  std::vector<ushort> addrs;

  z80_->getBreakpoints(addrs);

  std::string str;

  for (uint i = 0; i < addrs.size(); ++i) {
    str = CStrUtil::toHexString(addrs[i], 4);

    breakpointsText_->append(str.c_str());

    instructionsText_->addBreakPoint(addrs[i]);
  }
}

void
CQZ80Dbg::
postStepProc()
{
/*
  if (memoryGroup_      ->isChecked() ||
      instructionsGroup_->isChecked() ||
      registersGroup_   ->isChecked() ||
      flagsGroup_       ->isChecked() ||
      stackGroup_       ->isChecked() ||
      traceBackGroup_   ->isChecked() ||
      breakpointsGroup_ ->isChecked()) {
*/
  while (qApp->hasPendingEvents())
    qApp->processEvents();
}

void
CQZ80Dbg::
regChanged(CZ80Reg reg)
{
  if (reg == CZ80Reg::AF || reg == CZ80Reg::NONE) {
    if (reg == CZ80Reg::NONE || isRegistersTrace())
      afEdit_->setValue(z80_->getAF());

    if (reg == CZ80Reg::NONE || isFlagsTrace()) {
      cFlagCheck_->setChecked(z80_->tstCFlag());
      nFlagCheck_->setChecked(z80_->tstNFlag());
      pFlagCheck_->setChecked(z80_->tstPFlag());
      xFlagCheck_->setChecked(z80_->tstXFlag());
      hFlagCheck_->setChecked(z80_->tstHFlag());
      yFlagCheck_->setChecked(z80_->tstYFlag());
      zFlagCheck_->setChecked(z80_->tstZFlag());
      sFlagCheck_->setChecked(z80_->tstSFlag());
    }
  }

  if (reg == CZ80Reg::BC || reg == CZ80Reg::NONE) {
    if (reg == CZ80Reg::NONE || isRegistersTrace())
      bcEdit_->setValue(z80_->getBC());
  }

  if (reg == CZ80Reg::DE || reg == CZ80Reg::NONE) {
    if (reg == CZ80Reg::NONE || isRegistersTrace())
      deEdit_->setValue(z80_->getDE());
  }

  if (reg == CZ80Reg::HL || reg == CZ80Reg::NONE) {
    if (reg == CZ80Reg::NONE || isRegistersTrace())
      hlEdit_->setValue(z80_->getHL());
  }

  if (reg == CZ80Reg::IX || reg == CZ80Reg::NONE) {
    if (reg == CZ80Reg::NONE || isRegistersTrace())
      ixEdit_->setValue(z80_->getIX());
  }

  if (reg == CZ80Reg::IY || reg == CZ80Reg::NONE) {
    if (reg == CZ80Reg::NONE || isRegistersTrace())
      iyEdit_->setValue(z80_->getIY());
  }

  if (reg == CZ80Reg::SP || reg == CZ80Reg::NONE) {
    if (reg == CZ80Reg::NONE || isRegistersTrace())
      spEdit_->setValue(z80_->getSP());

    if (reg == CZ80Reg::NONE || isStackTrace())
      updateStack();
  }

  if (reg == CZ80Reg::PC || reg == CZ80Reg::NONE) {
    uint pc = z80_->getPC();

    if (reg == CZ80Reg::NONE || isRegistersTrace())
      pcEdit_->setValue(pc);

    if (reg == CZ80Reg::NONE || isBreakpointsTrace())
      breakpointsEdit_->setText(CStrUtil::toHexString(pc, 4).c_str());

    //----

    int mem1 = memoryVBar_->value();
    int mem2 = mem1 + 20;
    int mem  = pc / 8;

    if (reg == CZ80Reg::NONE || isMemoryTrace()) {
      if (mem < mem1 || mem > mem2) {
        memoryVBar_->setValue(mem);
      }
      else {
        memoryText_->update();
      }
    }

    //----

    if (reg == CZ80Reg::NONE || isInstructionsTrace()) {
      uint lineNum;

      if (! instructionsText_->getLineForPC(pc, lineNum))
        updateInstructions();

      if (instructionsText_->getLineForPC(pc, lineNum))
        instructionsVBar_->setValue(lineNum);

      //----

      // instruction at PC
      CZ80OpData opData;

      z80_->readOpData(pc, &opData);

      if (opData.op)
        opData_->setText(opData.getOpString(pc).c_str());
      else
        opData_->setText("");
    }
  }

  if (reg == CZ80Reg::I || reg == CZ80Reg::NONE) {
    if (reg == CZ80Reg::NONE || isRegistersTrace())
      iEdit_->setValue(z80_->getI  ());
  }

  if (reg == CZ80Reg::R || reg == CZ80Reg::NONE) {
    if (reg == CZ80Reg::NONE || isRegistersTrace())
      rEdit_->setValue(z80_->getR  ());
  }

  if (reg == CZ80Reg::AF1 || reg == CZ80Reg::NONE) {
    if (reg == CZ80Reg::NONE || isRegistersTrace())
      af1Edit_->setValue(z80_->getAF1());
  }

  if (reg == CZ80Reg::BC1 || reg == CZ80Reg::NONE) {
    if (reg == CZ80Reg::NONE || isRegistersTrace())
      bc1Edit_->setValue(z80_->getBC1());
  }

  if (reg == CZ80Reg::DE1 || reg == CZ80Reg::NONE) {
    if (reg == CZ80Reg::NONE || isRegistersTrace())
      de1Edit_->setValue(z80_->getDE1());
  }

  if (reg == CZ80Reg::HL1 || reg == CZ80Reg::NONE) {
    if (reg == CZ80Reg::NONE || isRegistersTrace())
      hl1Edit_->setValue(z80_->getHL1());
  }

  if (reg == CZ80Reg::IFF || reg == CZ80Reg::NONE) {
    if (reg == CZ80Reg::NONE || isRegistersTrace())
      iffEdit_->setValue(z80_->getIFF());
  }
}

void
CQZ80Dbg::
memChanged(ushort pos, ushort len)
{
  if (! isMemoryTrace()) {
    memoryDirty_ = true;
    return;
  }

  //if (! debug_) return;

  memChangedI(pos, len);
}

void
CQZ80Dbg::
memChangedI(ushort pos, ushort len)
{
  ushort pos1 = pos;
  ushort pos2 = pos + len;

  uint lineNum1 = pos1/8;
  uint lineNum2 = pos2/8;

  for (uint lineNum = lineNum1; lineNum <= lineNum2; ++lineNum)
    setMemoryLine(8*lineNum);

  memoryText_->update();

  memoryDirty_ = false;
}

void
CQZ80Dbg::
breakpointsChanged()
{
  if (isBreakpointsTrace())
    updateBreakpoints();
}

void
CQZ80Dbg::
traceBackChanged()
{
  if (isTraceBackTrace())
    updateTraceBack();
}

void
CQZ80Dbg::
setStop(bool)
{
}

void
CQZ80Dbg::
setHalt(bool b)
{
  haltCheck_->setChecked(b);
}

void
CQZ80Dbg::
addBreakpointSlot()
{
  uint value;

  if (! CStrUtil::decodeHexString(breakpointsEdit_->text().toStdString(), &value))
    value = z80_->getPC();

  if (! z80_->isBreakpoint(value))
    z80_->addBreakpoint(value);
}

void
CQZ80Dbg::
deleteBreakpointSlot()
{
  uint value;

  if (! CStrUtil::decodeHexString(breakpointsEdit_->text().toStdString(), &value))
    value = z80_->getPC();

  if (z80_->isBreakpoint(value))
    z80_->removeBreakpoint(value);
}

void
CQZ80Dbg::
clearBreakpointSlot()
{
  z80_->removeAllBreakpoints();
}

void
CQZ80Dbg::
memoryTraceSlot()
{
  setMemoryTrace(memoryGroup_->isChecked());
}

void
CQZ80Dbg::
instructionsTraceSlot()
{
  setInstructionsTrace(instructionsGroup_->isChecked());
}

void
CQZ80Dbg::
registersTraceSlot()
{
  setRegistersTrace(registersGroup_->isChecked());
}

void
CQZ80Dbg::
flagsTraceSlot()
{
  setFlagsTrace(flagsGroup_->isChecked());
}

void
CQZ80Dbg::
stackTraceSlot()
{
  setStackTrace(stackGroup_->isChecked());

  if (stackGroup_->isChecked())
    updateStack();
}

void
CQZ80Dbg::
traceBackTraceSlot()
{
  setTraceBackTrace(traceBackGroup_->isChecked());

  if (traceBackGroup_->isChecked())
    updateTraceBack();
}

void
CQZ80Dbg::
breakpointsTraceSlot()
{
  setBreakpointsTrace(breakpointsGroup_->isChecked());
}

void
CQZ80Dbg::
setTraceSlot()
{
  bool checked = traceCheck_->isChecked();

  memoryGroup_      ->setChecked(checked);
  instructionsGroup_->setChecked(checked);
  registersGroup_   ->setChecked(checked);
  flagsGroup_       ->setChecked(checked);
  stackGroup_       ->setChecked(checked);
  traceBackGroup_   ->setChecked(checked);
  breakpointsGroup_ ->setChecked(checked);
}

void
CQZ80Dbg::
setHaltSlot()
{
  z80_->setHalt(haltCheck_->isChecked());
}

void
CQZ80Dbg::
runSlot()
{
  z80_->execute();

  updateAll();
}

void
CQZ80Dbg::
nextSlot()
{
  z80_->execNext();

  updateAll();
}

void
CQZ80Dbg::
stepSlot()
{
  z80_->execStep();

  updateAll();
}

void
CQZ80Dbg::
continueSlot()
{
  z80_->execCont();

  updateAll();
}

void
CQZ80Dbg::
stopSlot()
{
  z80_->execStop();

  updateAll();
}

void
CQZ80Dbg::
restartSlot()
{
  z80_->reset();

  z80_->setPC(z80_->getLoadPos());

  updateAll();
}

void
CQZ80Dbg::
exitSlot()
{
  exit(0);
}

void
CQZ80Dbg::
updateAll()
{
  regChanged(CZ80Reg::NONE);

  memChangedI(0, 65535);

  update();
}
