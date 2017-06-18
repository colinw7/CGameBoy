#include <CQZ80Dbg.h>
#include <CZ80.h>
#include <CStrUtil.h>

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
#include <QMenu>
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

void
CQZ80Dbg::
init()
{
  addWidgets();

  setMemoryText();

  setInstructionsText();

  setStackText();

  setBreakpointText();

  regChangedI(CZ80_REG_NONE);
}

CQZ80Dbg::
~CQZ80Dbg()
{
}

void
CQZ80Dbg::
setFixedFont(const QFont &font)
{
  fixed_font_ = font;

  memoryText_      ->setFont(getFixedFont());
  instructionsText_->setFont(getFixedFont());
  stackText_      ->setFont(getFixedFont());
  breakpointsText_->setFont(getFixedFont());

  af_edit_ ->setFont(getFixedFont());
  af1_edit_->setFont(getFixedFont());
  bc_edit_ ->setFont(getFixedFont());
  bc1_edit_->setFont(getFixedFont());
  de_edit_ ->setFont(getFixedFont());
  de1_edit_->setFont(getFixedFont());
  hl_edit_ ->setFont(getFixedFont());
  hl1_edit_->setFont(getFixedFont());
  ix_edit_ ->setFont(getFixedFont());
  i_edit_  ->setFont(getFixedFont());
  iy_edit_ ->setFont(getFixedFont());
  r_edit_  ->setFont(getFixedFont());
  sp_edit_ ->setFont(getFixedFont());
  iff_edit_->setFont(getFixedFont());
  pc_edit_ ->setFont(getFixedFont());
}

void
CQZ80Dbg::
addWidgets()
{
  fixed_font_ = QFont("Courier", 10);

  QFontMetrics fm(getFixedFont());

  //----

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  QWidget *top_frame    = new QWidget;
  QWidget *bottom_frame = new QWidget;

  top_frame   ->setObjectName("top_frame");
  bottom_frame->setObjectName("bottom_frame");

  QHBoxLayout *top_layout    = new QHBoxLayout(top_frame);
  QVBoxLayout *bottom_layout = new QVBoxLayout(bottom_frame);

  top_layout   ->setMargin(2); top_layout   ->setSpacing(2);
  bottom_layout->setMargin(2); bottom_layout->setSpacing(2);

  layout->addWidget(top_frame);
  layout->addWidget(bottom_frame);

  //----

  QWidget *left_frame  = new QWidget;
  QWidget *right_frame = new QWidget;

  left_frame ->setObjectName("left_frame");
  right_frame->setObjectName("right_frame");

  top_layout->addWidget(left_frame);
  top_layout->addWidget(right_frame);

  QVBoxLayout *left_layout  = new QVBoxLayout(left_frame );
  QVBoxLayout *right_layout = new QVBoxLayout(right_frame);

  left_layout ->setMargin(2); left_layout ->setSpacing(2);
  right_layout->setMargin(2); right_layout->setSpacing(2);

  //----

  memoryGroup_ = new QGroupBox("Memory");

  memoryGroup_->setCheckable(true);

  connect(memoryGroup_, SIGNAL(toggled(bool)), this, SLOT(memoryTraceSlot()));

  memoryGroup_->setObjectName("memoryGroup");

  QHBoxLayout *memory_layout = new QHBoxLayout(memoryGroup_);

  memoryText_ = new CQZ80Mem(this);

  memoryText_->setFont(getFixedFont());

  memoryVBar_ = new QScrollBar;

  memoryVBar_->setObjectName("memory_vbar");
  memoryVBar_->setPageStep  (getNumMemoryLines());
  memoryVBar_->setSingleStep(1);
  memoryVBar_->setRange     (0, 8192 - memoryVBar_->pageStep());

  connect(memoryVBar_, SIGNAL(valueChanged(int)), memoryText_, SLOT(sliderSlot(int)));

  memory_layout->addWidget(memoryText_);
  memory_layout->addWidget(memoryVBar_);
  memory_layout->addStretch();

  left_layout->addWidget(memoryGroup_);

  //--

  instructionsGroup_ = new QGroupBox("Instructions");

  instructionsGroup_->setCheckable(true);

  connect(instructionsGroup_, SIGNAL(toggled(bool)), this, SLOT(instructionsTraceSlot()));

  instructionsGroup_->setObjectName("instructionsGroup");

  QHBoxLayout *instructions_layout = new QHBoxLayout(instructionsGroup_);

  instructionsText_ = new CQZ80Inst(this);

  instructionsText_->setFont(getFixedFont());

  instructionsVBar_ = new QScrollBar;

  instructionsVBar_->setObjectName("instructions_vbar");
  instructionsVBar_->setPageStep  (getNumMemoryLines());
  instructionsVBar_->setSingleStep(1);
  instructionsVBar_->setRange     (0, 8192 - instructionsVBar_->pageStep());

  connect(instructionsVBar_, SIGNAL(valueChanged(int)),
          instructionsText_, SLOT(sliderSlot(int)));

  instructions_layout->addWidget(instructionsText_);
  instructions_layout->addWidget(instructionsVBar_);
  instructions_layout->addStretch();

  left_layout->addWidget(instructionsGroup_);

  op_data_ = new QLineEdit;

  op_data_->setObjectName("op_data");

  op_data_->setReadOnly(true);

  left_layout->addWidget(op_data_);

  //----

  registersGroup_ = new QGroupBox("Registers");

  registersGroup_->setCheckable(true);

  connect(registersGroup_, SIGNAL(toggled(bool)), this, SLOT(registersTraceSlot()));

  registersGroup_->setObjectName("registersGroup");

  registersLayout_ = new QGridLayout(registersGroup_);

  addRegistersWidgets();

  right_layout->addWidget(registersGroup_);

  //--

  flagsGroup_ = new QGroupBox("Flags");

  flagsGroup_->setCheckable(true);

  connect(flagsGroup_, SIGNAL(toggled(bool)), this, SLOT(flagsTraceSlot()));

  flagsGroup_->setObjectName("flagsGroup");

  flagsLayout_ = new QGridLayout(flagsGroup_);

  flagsLayout_->setSpacing(6);

  addFlagsWidgets();

  right_layout->addWidget(flagsGroup_);

  //--

  stackGroup_ = new QGroupBox("Stack");

  stackGroup_->setCheckable(true);

  connect(stackGroup_, SIGNAL(toggled(bool)), this, SLOT(stackTraceSlot()));

  stackGroup_->setObjectName("stackGroup");

  QVBoxLayout *stack_layout = new QVBoxLayout(stackGroup_);

  stackText_ = new QTextEdit;

  stackText_->setObjectName("stack_text");

  stackText_->setReadOnly(true);

  stackText_->setFont(getFixedFont());

  stack_layout->addWidget(stackText_);

  right_layout->addWidget(stackGroup_);

  //--

  breakpointsGroup_ = new QGroupBox("Breakpoints");

  breakpointsGroup_->setCheckable(true);

  connect(breakpointsGroup_, SIGNAL(toggled(bool)), this, SLOT(breakpointsTraceSlot()));

  breakpointsGroup_->setObjectName("breakpointsGroup");

  breakpointsLayout_ = new QVBoxLayout(breakpointsGroup_);

  addBreakpointWidgets();

  right_layout->addWidget(breakpointsGroup_);

  //-----

  QFrame *optionsFrame = new QFrame;

  optionsFrame->setObjectName("optionsFrame");

  QHBoxLayout *optionsLayout = new QHBoxLayout(optionsFrame);

  traceCheck_ = new QCheckBox("Trace");

  traceCheck_->setObjectName("traceCheck");

  traceCheck_->setChecked(true);

  connect(traceCheck_, SIGNAL(stateChanged(int)), this, SLOT(setTraceSlot()));

  optionsLayout->addWidget(traceCheck_);

#if 0
  QCheckBox *memTraceCheck = new QCheckBox("Mem Trace");

  memTraceCheck->setObjectName("memTraceCheck");

  connect(memTraceCheck, SIGNAL(stateChanged(int)), this, SLOT(setMemTraceSlot(int)));

  optionsLayout->addWidget(memTraceCheck);
#endif

  optionsLayout->addStretch(1);

  bottom_layout->addWidget(optionsFrame);

  //---

  buttonsToolbar_ = new QFrame;

  buttonsToolbar_->setObjectName("buttonsToolbar");

  buttonsLayout_ = new QHBoxLayout(buttonsToolbar_);

  buttonsLayout_->addStretch(1);

  addButtonsWidgets();

  bottom_layout->addWidget(buttonsToolbar_);
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

  sFlagCheck_->setObjectName("s_flag_checkbox");
  zFlagCheck_->setObjectName("z_flag_checkbox");
  yFlagCheck_->setObjectName("y_flag_checkbox");
  hFlagCheck_->setObjectName("h_flag_checkbox");
  xFlagCheck_->setObjectName("x_flag_checkbox");
  pFlagCheck_->setObjectName("p_flag_checkbox");
  nFlagCheck_->setObjectName("n_flag_checkbox");
  cFlagCheck_->setObjectName("c_flag_checkbox");

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
  af_edit_  = new CQZ80RegEdit(this, CZ80_REG_AF );
  af1_edit_ = new CQZ80RegEdit(this, CZ80_REG_AF1);
  bc_edit_  = new CQZ80RegEdit(this, CZ80_REG_BC );
  bc1_edit_ = new CQZ80RegEdit(this, CZ80_REG_BC1);
  de_edit_  = new CQZ80RegEdit(this, CZ80_REG_DE );
  de1_edit_ = new CQZ80RegEdit(this, CZ80_REG_DE1);
  hl_edit_  = new CQZ80RegEdit(this, CZ80_REG_HL );
  hl1_edit_ = new CQZ80RegEdit(this, CZ80_REG_HL1);
  ix_edit_  = new CQZ80RegEdit(this, CZ80_REG_IX );
  i_edit_   = new CQZ80RegEdit(this, CZ80_REG_I  );
  iy_edit_  = new CQZ80RegEdit(this, CZ80_REG_IY );
  r_edit_   = new CQZ80RegEdit(this, CZ80_REG_R  );
  sp_edit_  = new CQZ80RegEdit(this, CZ80_REG_SP );
  iff_edit_ = new CQZ80RegEdit(this, CZ80_REG_IFF);
  pc_edit_  = new CQZ80RegEdit(this, CZ80_REG_PC );

  registersLayout_->addWidget(af_edit_ , 0, 0);
  registersLayout_->addWidget(af1_edit_, 0, 1);

  registersLayout_->addWidget(bc_edit_ , 1, 0);
  registersLayout_->addWidget(bc1_edit_, 1, 1);

  registersLayout_->addWidget(de_edit_ , 2, 0);
  registersLayout_->addWidget(de1_edit_, 2, 1);

  registersLayout_->addWidget(hl_edit_ , 3, 0);
  registersLayout_->addWidget(hl1_edit_, 3, 1);

  registersLayout_->addWidget(ix_edit_ , 4, 0);
  registersLayout_->addWidget(i_edit_  , 4, 1);

  registersLayout_->addWidget(iy_edit_ , 5, 0);
  registersLayout_->addWidget(r_edit_  , 5, 1);

  registersLayout_->addWidget(sp_edit_ , 6, 0);
  registersLayout_->addWidget(iff_edit_, 6, 1);

  registersLayout_->addWidget(pc_edit_ , 7, 0);

  registersLayout_->setColumnStretch(2, 1);
}

void
CQZ80Dbg::
addBreakpointWidgets()
{
  breakpointsText_ = new QTextEdit;

  breakpointsText_->setObjectName("breakpoints_text");
  breakpointsText_->setReadOnly(true);

  breakpointsText_->setFont(getFixedFont());

  breakpointsLayout_->addWidget(breakpointsText_);

  QWidget *breakpoint_toolbar = new QWidget;

  breakpoint_toolbar->setObjectName("breakpoint_toolbar");

  QHBoxLayout *breakpoint_toolbar_layout = new QHBoxLayout(breakpoint_toolbar);

  QPushButton *addBreakpointButton    = new QPushButton("Add"   );
  QPushButton *deleteBreakpointButton = new QPushButton("Delete");
  QPushButton *clearBreakpointButton  = new QPushButton("Clear" );

  addBreakpointButton   ->setObjectName("addBreakpointButton");
  deleteBreakpointButton->setObjectName("deleteBreakpointButton");
  clearBreakpointButton ->setObjectName("clearBreakpointButton");

  connect(addBreakpointButton   , SIGNAL(clicked()), this, SLOT(addBreakpointSlot   ()));
  connect(deleteBreakpointButton, SIGNAL(clicked()), this, SLOT(deleteBreakpointSlot()));
  connect(clearBreakpointButton , SIGNAL(clicked()), this, SLOT(clearBreakpointSlot ()));

  breakpoint_toolbar_layout->addWidget(addBreakpointButton);
  breakpoint_toolbar_layout->addWidget(deleteBreakpointButton);
  breakpoint_toolbar_layout->addWidget(clearBreakpointButton);
  breakpoint_toolbar_layout->addStretch(1);

  breakpointsLayout_->addWidget(breakpoint_toolbar);
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

  ushort num_lines = len / 8;

  if ((len % 8) != 0) ++num_lines;

  uint pos = z80_->getPC();

  z80_->setPC(0);

  std::string str;

  uint pos1 = 0;

  for (ushort i = 0; i < num_lines; ++i) {
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
setInstructionsText()
{
  uint init_pc = z80_->getPC();

  uint pos1 = 0;
  uint pos2 = 65536;

  instructionsText_->clear();

  uint pc       = pos1;
  bool pc_found = false;

  while (pc < pos2) {
    if (! pc_found && pc >= init_pc) {
      pc       = init_pc;
      pc_found = true;
    }

    //-----

    std::string pcStr = CStrUtil::toHexString(pc, 4);

    //-----

    uint last_pc = pc;

    z80_->setPC(pc);

    CZ80OpData op_data;

    z80_->readOpData(&op_data);

    pc = z80_->getPC();

    if (pc < last_pc) pc = pos2;

    //-----

    std::string codeStr;

    ushort len1 = 0;

    for (uint i = last_pc; i < pc; ++i) {
      if (i > last_pc) codeStr += " ";

      codeStr += CStrUtil::toHexString(z80_->getByte(i), 2);

      len1 += 3;
    }

    for ( ; len1 < 12; ++len1)
      codeStr += " ";

    //-----

    std::string textStr = "; ";

    if (op_data.op != NULL)
      textStr += op_data.getOpString();
    else
      textStr += "??";

    instructionsText_->setLine(last_pc, pcStr, codeStr, textStr);
  }

  uint numLines = instructionsText_->getNumLines();

  instructionsVBar_->setRange(0, numLines - instructionsVBar_->pageStep());

  instructionsVBar_->setValue(0);

  z80_->setPC(init_pc);

  instructionsText_->update();
}

void
CQZ80Dbg::
setStackText()
{
  stackText_->clear();

  ushort sp = z80_->getSP();

  ushort sp1 = sp - 4;

  std::string str;

  for (ushort i = 0; i < 16; ++i) {
    ushort sp2 = sp1 + i;

    str = "";

    if (sp2 == sp)
      str += "<b><font color=\"red\">&gt;</font></b>";
    else
      str += " ";

    str += CStrUtil::toHexString(sp2, 4);

    str += " ";

    str += CStrUtil::toHexString(z80_->getByte(sp2), 2);

    stackText_->append(str.c_str());
  }
}

void
CQZ80Dbg::
setBreakpointText()
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
  while (qApp->hasPendingEvents())
    qApp->processEvents();
}

void
CQZ80Dbg::
regChanged(CZ80Reg reg)
{
  regChangedI(reg);
}

void
CQZ80Dbg::
regChangedI(CZ80Reg reg)
{
  if (reg == CZ80_REG_AF || reg == CZ80_REG_NONE) {
    if (reg == CZ80_REG_NONE || isRegistersTrace())
      af_edit_->setValue(z80_->getAF());

    if (reg == CZ80_REG_NONE || isFlagsTrace()) {
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

  if (reg == CZ80_REG_BC || reg == CZ80_REG_NONE) {
    if (reg == CZ80_REG_NONE || isRegistersTrace())
      bc_edit_->setValue(z80_->getBC());
  }

  if (reg == CZ80_REG_DE || reg == CZ80_REG_NONE) {
    if (reg == CZ80_REG_NONE || isRegistersTrace())
      de_edit_->setValue(z80_->getDE());
  }

  if (reg == CZ80_REG_HL || reg == CZ80_REG_NONE) {
    if (reg == CZ80_REG_NONE || isRegistersTrace())
      hl_edit_->setValue(z80_->getHL());
  }

  if (reg == CZ80_REG_IX || reg == CZ80_REG_NONE) {
    if (reg == CZ80_REG_NONE || isRegistersTrace())
      ix_edit_->setValue(z80_->getIX());
  }

  if (reg == CZ80_REG_IY || reg == CZ80_REG_NONE) {
    if (reg == CZ80_REG_NONE || isRegistersTrace())
      iy_edit_->setValue(z80_->getIY());
  }

  if (reg == CZ80_REG_SP || reg == CZ80_REG_NONE) {
    if (reg == CZ80_REG_NONE || isRegistersTrace())
      sp_edit_->setValue(z80_->getSP());

    if (reg == CZ80_REG_NONE || isStackTrace())
      setStackText();
  }

  if (reg == CZ80_REG_PC || reg == CZ80_REG_NONE) {
    uint pc = z80_->getPC();

    if (reg == CZ80_REG_NONE || isRegistersTrace())
      pc_edit_->setValue(pc);

    //----

    int mem1 = memoryVBar_->value();
    int mem2 = mem1 + 20;
    int mem  = pc / 8;

    if (reg == CZ80_REG_NONE || isMemoryTrace()) {
      if (mem < mem1 || mem > mem2) {
        memoryVBar_->setValue(mem);
      }
      else {
        memoryText_->update();
      }
    }

    //----

    if (reg == CZ80_REG_NONE || isInstructionsTrace()) {
      uint line_num;

      if (! instructionsText_->getLineForPC(pc, line_num))
        setInstructionsText();

      if (instructionsText_->getLineForPC(pc, line_num))
        instructionsVBar_->setValue(line_num);

      //----

      CZ80OpData op_data;

      z80_->readOpData(&op_data);

      if (op_data.op != NULL)
        op_data_->setText(op_data.getOpString().c_str());
      else
        op_data_->setText("");
    }

    z80_->setPC(pc);
  }

  if (reg == CZ80_REG_I   || reg == CZ80_REG_NONE) {
    if (reg == CZ80_REG_NONE || isRegistersTrace())
      i_edit_  ->setValue(z80_->getI  ());
  }

  if (reg == CZ80_REG_R   || reg == CZ80_REG_NONE) {
    if (reg == CZ80_REG_NONE || isRegistersTrace())
      r_edit_  ->setValue(z80_->getR  ());
  }

  if (reg == CZ80_REG_AF1 || reg == CZ80_REG_NONE) {
    if (reg == CZ80_REG_NONE || isRegistersTrace())
      af1_edit_->setValue(z80_->getAF1());
  }

  if (reg == CZ80_REG_BC1 || reg == CZ80_REG_NONE) {
    if (reg == CZ80_REG_NONE || isRegistersTrace())
      bc1_edit_->setValue(z80_->getBC1());
  }

  if (reg == CZ80_REG_DE1 || reg == CZ80_REG_NONE) {
    if (reg == CZ80_REG_NONE || isRegistersTrace())
      de1_edit_->setValue(z80_->getDE1());
  }

  if (reg == CZ80_REG_HL1 || reg == CZ80_REG_NONE) {
    if (reg == CZ80_REG_NONE || isRegistersTrace())
      hl1_edit_->setValue(z80_->getHL1());
  }

  if (reg == CZ80_REG_IFF || reg == CZ80_REG_NONE) {
    if (reg == CZ80_REG_NONE || isRegistersTrace())
      iff_edit_->setValue(z80_->getIFF());
  }
}

void
CQZ80Dbg::
memChanged(ushort pos, ushort len)
{
  if (! isMemoryTrace())
    return;

  //if (! debug_) return;

  memChangedI(pos, len);
}

void
CQZ80Dbg::
memChangedI(ushort pos, ushort len)
{
  ushort pos1 = pos;
  ushort pos2 = pos + len;

  uint line_num1 = pos1/8;
  uint line_num2 = pos2/8;

  for (uint line_num = line_num1; line_num <= line_num2; ++line_num)
    setMemoryLine(8*line_num);

  memoryText_->update();
}

void
CQZ80Dbg::
breakpointsChanged()
{
  if (isBreakpointsTrace())
    setBreakpointText();
}

#if 0
QLineEdit *
CQZ80Dbg::
createRegisterEdit()
{
  QLineEdit *edit = new QLineEdit;

  edit->setObjectName("edit");

  edit->setFont(getFixedFont());

  QFontMetrics fm(edit->font());

  edit->setFixedWidth(fm.width("0000") + 16);

  return edit;
}
#endif

void
CQZ80Dbg::
addBreakpointSlot()
{
  if (! z80_->isBreakpoint(z80_->getPC()))
    z80_->addBreakpoint(z80_->getPC());
}

void
CQZ80Dbg::
deleteBreakpointSlot()
{
  if (z80_->isBreakpoint(z80_->getPC()))
    z80_->removeBreakpoint(z80_->getPC());
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
  breakpointsGroup_ ->setChecked(checked);
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
  z80_->next();

  updateAll();
}

void
CQZ80Dbg::
stepSlot()
{
  z80_->step();

  updateAll();
}

void
CQZ80Dbg::
continueSlot()
{
  z80_->cont();

  updateAll();
}

void
CQZ80Dbg::
stopSlot()
{
  z80_->stop();

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
  regChangedI(CZ80_REG_NONE);

  memChangedI(0, 65535);

  update();
}

//-----------

CQZ80Mem::
CQZ80Mem(CQZ80Dbg *dbg) :
 QWidget(nullptr), dbg_(dbg)
{
  setObjectName("mem");

  lines_.resize(8192);
}

void
CQZ80Mem::
setFont(const QFont &font)
{
  QWidget::setFont(font);

  QFontMetrics fm(font);

  int memory_width = fm.width("0000  00 00 00 00 00 00 00 00  XXXXXXXX");
  int char_height  = fm.height();

  setFixedWidth (memory_width + 32);
  setFixedHeight(char_height*dbg_->getNumMemoryLines());
}

void
CQZ80Mem::
setLine(uint pc, const std::string &pcStr, const std::string &memStr, const std::string &textStr)
{
  uint line_num = pc / 8;

  lines_[line_num] = CQZ80MemLine(pc, pcStr, memStr, textStr);
}

void
CQZ80Mem::
contextMenuEvent(QContextMenuEvent *event)
{
  QMenu *menu = new QMenu;

  QAction *action = menu->addAction("Dump");

  connect(action, SIGNAL(triggered()), this, SLOT(dumpSlot()));

  menu->exec(event->globalPos());

  delete menu;
}

void
CQZ80Mem::
paintEvent(QPaintEvent *)
{
  CZ80 *z80 = dbg_->getZ80();

  uint pc = z80->getPC();

  QPainter p(this);

  p.fillRect(rect(), Qt::white);

  QFontMetrics fm(font());

  char_height_ = fm.height();
  char_width_  = fm.width(" ");

  int char_ascent = fm.ascent();

  int w1 =  4*char_width_; // address (4 digits)
  int w2 =    char_width_; // spacer (1 char)
  int w3 = 23*char_width_; // data (16 digits + 7 spaces)
  int w4 =    char_width_; // spacer (1 char)

  int y  = -y_offset_*char_height_;
  int ya = y + char_ascent;

  int ymin = -char_height_;
  int ymax = height() + char_height_;

  for (const auto &line : lines_) {
    if (y >= ymin && y <= ymax) {
      int x = dx_;

      uint pc1 = line.pc();
      uint pc2 = pc1 + 8;

      if      (z80->isReadOnly(pc1, 8))
        p.fillRect(QRect(x + w1 + w2, y, w3, char_height_), dbg_->readOnlyBgColor());
      else if (z80->isScreen(pc1, 8))
        p.fillRect(QRect(x + w1 + w2, y, w3, char_height_), dbg_->screenBgColor());

      p.setPen(dbg_->addrColor());

      p.drawText(x, ya, line.pcStr().c_str());

      x += w1 + w2;

      p.setPen(dbg_->memDataColor());

      if (pc >= pc1 && pc < pc2) {
        int i1 = 3*(pc - pc1);
        int i2 = i1 + 2;

        std::string lhs = line.memStr().substr(0, i1);
        std::string mid = line.memStr().substr(i1, 2);
        std::string rhs = line.memStr().substr(i2);

        p.drawText(x                 , ya, lhs.c_str());
        p.drawText(x + char_width_*i2, ya, rhs.c_str());

        p.setPen(dbg_->currentColor());

        p.drawText(x + char_width_*i1, ya, mid.c_str());
      }
      else {
        p.drawText(x, ya, line.memStr().c_str());
      }

      x += w3 + w4;

      p.setPen(dbg_->memCharsColor());

      p.drawText(x, ya, line.textStr().c_str());
    }

    y  += char_height_;
    ya += char_height_;
  }
}

void
CQZ80Mem::
mouseDoubleClickEvent(QMouseEvent *e)
{
  int ix = (e->pos().x() - dx_                   )/char_width_ ;
  int iy = (e->pos().y() + y_offset_*char_height_)/char_height_;

  if (ix < 4 || ix >= 28  ) return;
  if (iy < 0 || iy >= 8192) return;

  uint pc = int((ix - 4)/3) + iy*8;

  CZ80 *z80 = dbg_->getZ80();

  z80->setPC(pc);

  dbg_->regChanged(CZ80_REG_PC);
}

void
CQZ80Mem::
sliderSlot(int y)
{
  y_offset_ = y;

  update();
}

void
CQZ80Mem::
dumpSlot()
{
  FILE *fp = fopen("memory.txt", "w");
  if (! fp) return;

  for (const auto &line : lines_) {
    fprintf(fp, "%s %s %s\n", line.pcStr().c_str(), line.memStr().c_str(),
            line.textStr().c_str());
  }

  fclose(fp);
}

//-----------

CQZ80Inst::
CQZ80Inst(CQZ80Dbg *dbg) :
 QWidget(nullptr), dbg_(dbg)
{
  setObjectName("inst");

  lines_.resize(65536);
}

void
CQZ80Inst::
setFont(const QFont &font)
{
  QWidget::setFont(font);

  QFontMetrics fm(font);

  int instructions_width = fm.width("0000  123456789012  AAAAAAAAAAAAAAAAAAAAAA");
  int char_height        = fm.height();

  setFixedWidth (instructions_width + 32);
  setFixedHeight(char_height*dbg_->getNumMemoryLines());
}

void
CQZ80Inst::
clear()
{
  line_num_ = 0;

  pc_line_map_.clear();
  line_pc_map_.clear();
}

void
CQZ80Inst::
setLine(uint pc, const std::string &pcStr, const std::string &codeStr, const std::string &textStr)
{
  lines_[line_num_] = CQZ80InstLine(pc, pcStr, codeStr, textStr);

  pc_line_map_[pc       ] = line_num_;
  line_pc_map_[line_num_] = pc;

  ++line_num_;
}

bool
CQZ80Inst::
getLineForPC(uint pc, uint &line_num) const
{
  PCLineMap::const_iterator p = pc_line_map_.find(pc);

  if (p == pc_line_map_.end())
    return false;

  line_num = (*p).second;

  return true;
}

uint
CQZ80Inst::
getPCForLine(uint line_num)
{
  return line_pc_map_[line_num];
}

void
CQZ80Inst::
contextMenuEvent(QContextMenuEvent *event)
{
  QMenu *menu = new QMenu;

  QAction *action = menu->addAction("Dump");

  connect(action, SIGNAL(triggered()), this, SLOT(dumpSlot()));

  menu->exec(event->globalPos());

  delete menu;
}

void
CQZ80Inst::
paintEvent(QPaintEvent *)
{
  CZ80 *z80 = dbg_->getZ80();

  uint pc = z80->getPC();

  QPainter p(this);

  p.fillRect(rect(), Qt::white);

  QFontMetrics fm(font());

  char_height_ = fm.height();

  int char_width  = fm.width(" ");
  int char_ascent = fm.ascent();

  int w1 =  4*char_width;
  int w2 = 12*char_width;

  int y = -y_offset_*char_height_ + char_ascent;

  int ymin = -char_height_;
  int ymax = height() + char_height_;

  for (const auto &line : lines_) {
    if (y >= ymin && y <= ymax) {
      int x = 2;

      if (line.pc() == pc) {
        p.setPen(dbg_->currentColor());

        p.drawText(x, y, ">");
      }

      x += char_width;

      p.setPen(dbg_->addrColor());

      p.drawText(x, y, line.pcStr().c_str());

      x += w1 + char_width;

      p.setPen(dbg_->memDataColor());

      p.drawText(x, y, line.codeStr().c_str());

      x += w2 + char_width;

      p.setPen(dbg_->memCharsColor());

      p.drawText(x, y, line.textStr().c_str());
    }

    y += char_height_;
  }
}

void
CQZ80Inst::
mouseDoubleClickEvent(QMouseEvent *e)
{
  int iy = (e->pos().y() + y_offset_*char_height_)/char_height_;

  CZ80 *z80 = dbg_->getZ80();

  z80->setPC(getPCForLine(iy));

  dbg_->regChanged(CZ80_REG_PC);
}

void
CQZ80Inst::
sliderSlot(int y)
{
  y_offset_ = y;

  update();
}

void
CQZ80Inst::
dumpSlot()
{
  FILE *fp = fopen("inst.txt", "w");
  if (! fp) return;

  for (const auto &line : lines_) {
    fprintf(fp, "%s %s %s\n", line.pcStr().c_str(), line.codeStr().c_str(),
            line.textStr().c_str());
  }

  fclose(fp);
}

//------

CQZ80RegEdit::
CQZ80RegEdit(CQZ80Dbg *dbg, CZ80Reg reg) :
 QWidget(nullptr), dbg_(dbg), reg_(reg)
{
  QHBoxLayout *layout = new QHBoxLayout(this);

  layout->setMargin(2); layout->setMargin(2);

  QString str;

  switch (reg) {
    case CZ80_REG_AF : str = "AF" ; break;
    case CZ80_REG_AF1: str = "AF'"; break;
    case CZ80_REG_BC : str = "BC" ; break;
    case CZ80_REG_BC1: str = "BC'"; break;
    case CZ80_REG_DE : str = "DE" ; break;
    case CZ80_REG_DE1: str = "DE'"; break;
    case CZ80_REG_HL : str = "HL" ; break;
    case CZ80_REG_HL1: str = "HL'"; break;
    case CZ80_REG_IX : str = "IX" ; break;
    case CZ80_REG_I  : str = "I"  ; break;
    case CZ80_REG_IY : str = "IY" ; break;
    case CZ80_REG_R  : str = "R"  ; break;
    case CZ80_REG_SP : str = "SP" ; break;
    case CZ80_REG_PC : str = "PC" ; break;
    case CZ80_REG_IFF: str = "IFF"; break;
    default          : assert(false);
  }

  setObjectName(str);

  label_ = new QLabel(str);

  label_->setObjectName("label");

  edit_ = new QLineEdit;

  edit_->setObjectName("edit");

  layout->addWidget(label_);
  layout->addWidget(edit_);

  connect(edit_, SIGNAL(returnPressed()), this, SLOT(valueChangedSlot()));

  //---

  setFont(dbg_->getFixedFont());
}

void
CQZ80RegEdit::
setFont(const QFont &font)
{
  QWidget::setFont(font);

  label_->setFont(font);
  edit_ ->setFont(font);

  QFontMetrics fm(font);

  label_->setFixedWidth(fm.width("XXX") + 4);
  edit_ ->setFixedWidth(fm.width("0000") + 16);
}

void
CQZ80RegEdit::
setValue(uint value)
{
  int len = 4;

  if (reg_ == CZ80_REG_I || reg_ == CZ80_REG_R || reg_ == CZ80_REG_IFF) len = 2;

  edit_->setText(CStrUtil::toHexString(value, len).c_str());
}

void
CQZ80RegEdit::
valueChangedSlot()
{
  uint value;

  if (! CStrUtil::decodeHexString(edit_->text().toStdString(), &value))
    return;

  CZ80 *z80 = dbg_->getZ80();

  switch (reg_) {
    case CZ80_REG_AF : z80->setAF (value); break;
    case CZ80_REG_AF1: z80->setAF1(value); break;
    case CZ80_REG_BC : z80->setBC (value); break;
    case CZ80_REG_BC1: z80->setBC1(value); break;
    case CZ80_REG_DE : z80->setDE (value); break;
    case CZ80_REG_DE1: z80->setDE1(value); break;
    case CZ80_REG_HL : z80->setHL (value); break;
    case CZ80_REG_HL1: z80->setHL1(value); break;
    case CZ80_REG_IX : z80->setIX (value); break;
    case CZ80_REG_I  : z80->setI  (value); break;
    case CZ80_REG_IY : z80->setIY (value); break;
    case CZ80_REG_R  : z80->setR  (value); break;
    case CZ80_REG_SP : z80->setSP (value); break;
    case CZ80_REG_PC : z80->setPC (value); break;
    case CZ80_REG_IFF: z80->setIFF(value); break;
    default          : assert(false);
  }

  dbg_->regChanged(reg_);
}
