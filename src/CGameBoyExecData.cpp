#include <CGameBoyExecData.h>
#include <CGameBoy.h>
#include <CZ80OpData.h>

CGameBoyExecData::
CGameBoyExecData(CGameBoy *gameboy) :
 CZ80ExecData(*gameboy->getZ80()), gameboy_(gameboy), trace_(false)
{
  trace_ = getenv("CGAMEBOY_EXEC_TRACE");
}

void
CGameBoyExecData::
preStep()
{
  if (z80_.getPC() == 0x0100) {
    gameboy_->setBiosEnabled(false);
  }
}

void
CGameBoyExecData::
preExec()
{
  if (trace_) {
    //if (gameboy_->isBiosEnabled()) return;

    int pc = z80_.getPC() - z80_.opData()->op->len;
    int op = z80_.opData()->op->ind;
    if (op > 0xff) ++pc;

    std::cerr << std::hex << pc << ":" << std::hex << (op & 0xff) <<
                 " AF=" << int(z80_.getAF()) << " BC=" << int(z80_.getBC()) <<
                 " DE=" << int(z80_.getDE()) << " HL=" << int(z80_.getHL()) <<
                 " SP=" << int(z80_.getSP()) << std::endl;
  }
}

void
CGameBoyExecData::
postStep()
{
  uchar tac = z80_.getByte(0xff07); // TAC - timer control

  bool timerActive = TST_BIT(tac, 2);

  if (timerActive) {
    m_     = z80_.getT() >> 1; // M-clock: tick (t) / 2
    timer_ = m_ >> 2;          // Timer: M-clock / 4;

    //--
    // Divider Register (M-clock / 16)
    uchar div = z80_.getByte(0xff04);

    uchar newDiv = (timer_ >> 4);

    if (newDiv != div)
      z80_.setByte(0xff04, newDiv);

    //--

    // Timer counter
    uchar tac_speed = (tac & 0x3);

    uchar tima = z80_.getByte(0xff05);
    uchar tmod = z80_.getByte(0xff06);

    ushort newTima;

    if      (tac_speed == 0) // M-clock / 64
      newTima = m_ >> 6;
    else if (tac_speed == 2) // M-clock / 4
      newTima = m_ >> 2;
    else if (tac_speed == 3) // M-clock / 16
      newTima = m_ >> 4;
    else                     // M-clock / 1
      newTima = m_;

    newTima += tmod;

    if (newTima != tima) {
      // check overflow
      bool overflow = (newTima > 0xff);

      z80_.setByte(0xff05, newTima & 0xff);

      // timer overflow interrupt
      if (overflow)
        gameboy_->signalInterrupt(CGameBoy::InterruptType::TIMER);
    }
  }

  //------

  gameboy_->handleInterrupts();
}

void
CGameBoyExecData::
setStop(bool b)
{
  gameboy_->execStop(b);
}

void
CGameBoyExecData::
setHalt(bool b)
{
  gameboy_->execHalt(b);
}
