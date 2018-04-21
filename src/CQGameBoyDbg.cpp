#include <CQGameBoyDbg.h>
#include <CQGameBoyScreen.h>
#include <CQGameBoy.h>
#include <CQZ80RegEdit.h>
#include <CQZ80FlagCheck.h>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>

CQGameBoyDbg::
CQGameBoyDbg(CQGameBoy *gameboy) :
 CQZ80Dbg(gameboy->getZ80()), gameboy_(gameboy)
{
  setObjectName("dbg");

  setWindowTitle("GameBoy CPU Debugger");
}

void
CQGameBoyDbg::
addRegistersWidgets()
{
  std::vector<CZ80Reg> registers = {{
    CZ80Reg::AF,
    CZ80Reg::BC,
    CZ80Reg::DE,
    CZ80Reg::HL,
    CZ80Reg::SP,
    CZ80Reg::PC,
    CZ80Reg::I , CZ80Reg::R  , CZ80Reg::IFF
  }};

  for (const auto &reg : registers)
    regEdits_[reg] = new CQZ80RegEdit(z80_, reg);

  registersLayout_->addWidget(regEdits_[CZ80Reg::AF ], 0, 0);
  registersLayout_->addWidget(regEdits_[CZ80Reg::BC ], 0, 1);
  registersLayout_->addWidget(regEdits_[CZ80Reg::DE ], 1, 0);
  registersLayout_->addWidget(regEdits_[CZ80Reg::HL ], 1, 1);

  registersLayout_->addWidget(regEdits_[CZ80Reg::SP ], 2, 0);
  registersLayout_->addWidget(regEdits_[CZ80Reg::PC ], 2, 1);

  registersLayout_->addWidget(regEdits_[CZ80Reg::I  ], 3, 0);
  registersLayout_->addWidget(regEdits_[CZ80Reg::R  ], 3, 1);

  registersLayout_->addWidget(regEdits_[CZ80Reg::IFF], 4, 0);

  registersLayout_->setColumnStretch(2, 1);
}

void
CQGameBoyDbg::
addFlagsWidgets()
{
  // 76543210
  // ZSHC----

  std::vector<CZ80Flag> flags = {{
    CZ80Flag::Z, CZ80Flag::S, CZ80Flag::H, CZ80Flag::C
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
CQGameBoyDbg::
addButtonsWidgets()
{
  bootButton_ = addButtonWidget("boot", "Boot");

  connect(bootButton_, SIGNAL(clicked()), this, SLOT(bootSlot()));

  CQZ80Dbg::addButtonsWidgets();
}

void
CQGameBoyDbg::
bootSlot()
{
  CZ80 *z80 = gameboy_->getZ80();

  gameboy_->setBiosEnabled(true);

  z80->setPC(0);

  z80->addBreakpoint(0x0100);

  z80->execCont();

  z80->removeBreakpoint(0x0100);

  updateAll();
}
