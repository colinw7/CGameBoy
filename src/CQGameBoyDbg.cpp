#include <CQGameBoyDbg.h>
#include <CQGameBoyScreen.h>
#include <CQGameBoy.h>
#include <QPushButton>

CQGameBoyDbg::
CQGameBoyDbg(CQGameBoy *gameboy) :
 CQZ80Dbg(gameboy->getZ80()), gameboy_(gameboy)
{
  setWindowTitle("GameBoy CPU Debugger");
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

  z80->addBreakpoint(0x0100);

  z80->cont();

  z80->removeBreakpoint(0x0100);

  updateAll();
}
