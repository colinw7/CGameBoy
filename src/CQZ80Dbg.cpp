#include <CQZ80Dbg.h>
#include <CQZ80Memory.h>
#include <CQZ80Instructions.h>
#include <CQZ80Stack.h>
#include <CQZ80TraceBack.h>
#include <CQZ80RegEdit.h>
#include <CQZ80FlagCheck.h>
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

  for (auto &regEdit : regEdits_)
    regEdit.second->setFont(getFixedFont());
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

  memoryLayout->addWidget(memoryText_);

  leftLayout->addWidget(memoryGroup_);

  //--

  instructionsGroup_ = new QGroupBox("Instructions");

  instructionsGroup_->setObjectName("instructionsGroup");
  instructionsGroup_->setCheckable(true);

  connect(instructionsGroup_, SIGNAL(toggled(bool)), this, SLOT(instructionsTraceSlot()));

  QHBoxLayout *instructionsLayout = new QHBoxLayout(instructionsGroup_);

  instructionsText_ = new CQZ80Inst(this);

  instructionsText_->setFont(getFixedFont());

  instructionsLayout->addWidget(instructionsText_);

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
  // 75643210
  // SZYHXPNC

  std::vector<CZ80Flag> flags = {{
    CZ80Flag::S, CZ80Flag::Z, CZ80Flag::Y, CZ80Flag::H,
    CZ80Flag::X, CZ80Flag::P, CZ80Flag::N, CZ80Flag::C
  }};

  int col = 0;

  for (const auto &flag : flags) {
    CQZ80FlagCheck *check = new CQZ80FlagCheck(z80_, flag);

    flagChecks_[flag] = check;

    flagsLayout_->addWidget(new QLabel(check->flagName()), 0, col);
    flagsLayout_->addWidget(check                        , 1, col);

    ++col;
  }

  flagsLayout_->setColumnStretch(col, 1);
}

void
CQZ80Dbg::
addRegistersWidgets()
{
  std::vector<CZ80Reg> registers = {{
    CZ80Reg::AF, CZ80Reg::AF1,
    CZ80Reg::BC, CZ80Reg::BC1,
    CZ80Reg::DE, CZ80Reg::DE1,
    CZ80Reg::HL, CZ80Reg::HL1,
    CZ80Reg::IX, CZ80Reg::IY ,
    CZ80Reg::SP,
    CZ80Reg::PC,
    CZ80Reg::I , CZ80Reg::R  , CZ80Reg::IFF
  }};

  for (const auto &reg : registers)
    regEdits_[reg] = new CQZ80RegEdit(z80_, reg);

  registersLayout_->addWidget(regEdits_[CZ80Reg::AF ], 0, 0);
  registersLayout_->addWidget(regEdits_[CZ80Reg::AF1], 0, 1);

  registersLayout_->addWidget(regEdits_[CZ80Reg::BC ], 1, 0);
  registersLayout_->addWidget(regEdits_[CZ80Reg::BC1], 1, 1);

  registersLayout_->addWidget(regEdits_[CZ80Reg::DE ], 2, 0);
  registersLayout_->addWidget(regEdits_[CZ80Reg::DE1], 2, 1);

  registersLayout_->addWidget(regEdits_[CZ80Reg::HL ], 3, 0);
  registersLayout_->addWidget(regEdits_[CZ80Reg::HL1], 3, 1);

  registersLayout_->addWidget(regEdits_[CZ80Reg::IX ], 4, 0);
  registersLayout_->addWidget(regEdits_[CZ80Reg::I  ], 4, 1);

  registersLayout_->addWidget(regEdits_[CZ80Reg::IY ], 5, 0);
  registersLayout_->addWidget(regEdits_[CZ80Reg::R  ], 5, 1);

  registersLayout_->addWidget(regEdits_[CZ80Reg::SP ], 6, 0);
  registersLayout_->addWidget(regEdits_[CZ80Reg::IFF], 6, 1);

  registersLayout_->addWidget(regEdits_[CZ80Reg::PC ], 7, 0);

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
  memoryText_->updateData();
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
  // update register widgets
  if (isRegistersTrace()) {
    if (reg == CZ80Reg::NONE) {
      for (auto &regEdit : regEdits_)
        regEdit.second->updateValue();
    }
    else {
      regEdits_[reg]->updateValue();
    }
  }

  // update flags widgets
  if (isFlagsTrace()) {
    if (reg == CZ80Reg::AF || reg == CZ80Reg::NONE) {
      for (auto &flagChecks : flagChecks_)
        flagChecks.second->updateState();
    }
  }

  // update breakpoints, memory and instructions widgets for PC change
  if (reg == CZ80Reg::NONE || reg == CZ80Reg::PC) {
    uint pc = z80_->getPC();

    //----

    if (isBreakpointsTrace())
      breakpointsEdit_->setText(CStrUtil::toHexString(pc, 4).c_str());

    //----

    if (isMemoryTrace()) {
      memoryText_->updatePC();
    }

    //----

    if (isInstructionsTrace()) {
      uint lineNum;

      if (! instructionsText_->getLineForPC(pc, lineNum))
        updateInstructions();

      if (instructionsText_->getLineForPC(pc, lineNum))
        instructionsText_->setVBarValue(lineNum);

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
  memoryText_->updateMemory(pos, len);

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

  if (registersGroup_->isChecked())
    regChanged(CZ80Reg::NONE);
}

void
CQZ80Dbg::
flagsTraceSlot()
{
  setFlagsTrace(flagsGroup_->isChecked());

  if (flagsGroup_->isChecked())
    regChanged(CZ80Reg::NONE);
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
