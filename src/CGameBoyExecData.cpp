#include <CGameBoyExecData.h>
#include <CGameBoy.h>

CGameBoyExecData::
CGameBoyExecData(CGameBoy *gameboy) :
 CZ80ExecData(*gameboy->getZ80()), gameboy_(gameboy)
{
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
      if (overflow && z80_.getAllowInterrupts())
        z80_.setBit(0xff0f, 2);
    }
  }

  //------

  // handle interrupt flags if enabled
  if (z80_.getIFF1()) {
    uchar iflag = z80_.getByte(0xff0f);
    uchar ie    = z80_.getByte(0xffff);

    // vertical blank (LCD has drawn a frame)
    if      (IS_BIT(iflag, 0)) {
      z80_.resetBit(0xff0f, 0);

      if (IS_BIT(ie, 0)) {
        //std::cerr << "vertical blank interrupt" << std::endl;

        z80_.setIM0(0x40);
        z80_.interrupt();
      }
    }
    // LCD controller changed
    else if (IS_BIT(iflag, 1)) {
      // TODO
      z80_.resetBit(0xff0f, 1);

      if (IS_BIT(ie, 1)) {
        //std::cerr << "LCD controller interrupt" << std::endl;

        z80_.setIM0(0x48);
        z80_.interrupt();
      }
    }
    // timer overflow
    else if (IS_BIT(iflag, 2)) {
      z80_.resetBit(0xff0f, 2);

      if (IS_BIT(ie, 2)) {
        //std::cerr << "timer interrupt" << std::endl;

        z80_.setIM0(0x50);
        z80_.interrupt();
      }
    }
    // Serial I/O transfer end
    else if (IS_BIT(iflag, 3)) {
      z80_.resetBit(0xff0f, 3);

      if (IS_BIT(ie, 3)) {
        //std::cerr << "serial interrupt" << std::endl;

        z80_.setIM0(0x58);
        z80_.interrupt();
      }
    }
    // Transition from High to Low of Pin number P10-P13 (key)
    else if (IS_BIT(iflag, 4)) {
      z80_.resetBit(0xff0f, 4);

      if (IS_BIT(ie, 4)) {
        //std::cerr << "key interrupt" << std::endl;

        z80_.setIM0(0x60);
        z80_.interrupt();
      }
    }
  }
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
