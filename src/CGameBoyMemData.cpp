#include <CGameBoyMemData.h>
#include <CGameBoy.h>
#include <CGameBoyBios.h>

CGameBoyMemData::
CGameBoyMemData(CGameBoy *gameboy) :
 CZ80MemData(*gameboy->getZ80()), gameboy_(gameboy)
{
  assert(sizeof(CGameBoyBios::gbData ) == CGameBoyBios::gbSize );
  assert(sizeof(CGameBoyBios::cgbData) == CGameBoyBios::cgbSize);
}

bool
CGameBoyMemData::
memRead(uchar *data, ushort pos, ushort len)
{
  if (! isEnabled())
    return false;

  for (ushort i = 0; i < len; ++i)
    memReadData(pos + i, &data[i]);

  return true;
}

void
CGameBoyMemData::
memReadData(ushort pos, uchar *data)
{
  // BIOS at startup
  if (gameboy()->isBiosEnabled()) {
    if (gameboy_->isGBC()) {
      if (pos < 0x100) {
        *data = CGameBoyBios::cgbData[pos];
        return;
      }

      if (pos >= 0x200 && pos < 0x900) {
        *data = CGameBoyBios::cgbData[pos - 0x100];
        return;
      }
    }
    else {
      if      (pos < 0x100) {
        *data = CGameBoyBios::gbData[pos];
        return;
      }
    }
  }

  //------

  // 16kB ROM bank #0 (0x0000 - 0x3fff)
  if      (pos < 0x4000) {
    *data = z80_.getMemory(pos);
  }
  // 16kB switchable ROM bank (0x4000 - 0x7fff)
  else if (pos < 0x8000) {
    if (gameboy()->cartridge()) {
      //uint pos1 = pos - 0x4000;
      uint pos1 = pos;

      *data = gameboy()->readCartridge(pos1 + gameboy()->romOffset());
    }
    else
      *data = z80_.getMemory(pos);
  }
  // 8kB Video RAM (0x8000 - 0x9fff)
  else if (pos < 0xa000) {
    if (gameboy_->isGBC()) {
      *data = gameboy()->getVRam(gameboy_->vramBank(), pos - 0x8000);
    }
    else {
      *data = z80_.getMemory(pos);
    }
  }
  // 8kB switchable RAM bank (0xa000 - 0xbfff)
  else if (pos < 0xc000) {
    ushort pos1 = pos - 0xa000;

    *data = gameboy()->getRam(pos1 + gameboy()->ramOffset());
  }
  // 4kB Internal RAM (0xc000 - 0xcfff)
  else if (pos < 0xd000) {
    *data = z80_.getMemory(pos);
  }
  // 4kB Internal RAM (0xd000 - 0xdfff)
  else if (pos < 0xe000) {
    if (gameboy_->isGBC()) {
      // WRAM (1-7)
      ushort pos1 = pos - 0xd000;

      ushort offset = (gameboy()->wramBank() - 1)*0x1000;

      *data = gameboy()->getWRam(pos1 + offset);
    }
    else {
      *data = z80_.getMemory(pos);
    }
  }
  // Echo of 8kB Internal RAM (0xe000 - 0xfdff)
  else if (pos < 0xfe00) {
    *data = z80_.getMemory(pos - 0x2000);
  }
  // Sprite Attrib Memory (OAM) (0xfe00 - 0xfe9f)
  else if (pos < 0xfea0) {
    *data = z80_.getMemory(pos);
  }
  // Empty but unusable for I/O (0xfea0 - 0xfeff)
  else if (pos < 0xff00) {
    *data = z80_.getMemory(pos);
  }
  // I/O ports (0xff00 - 0xff7f)
  else if (pos < 0xff80) {
    // Key Data
    if      (pos == 0xff00) {
      int data1 = gameboy()->keySel();

      if      (data1 == 0x10)
        *data = gameboy()->key0();
      else if (data1 == 0x20)
        *data = gameboy()->key1();
      else
        *data = 0;
    }
    // SB - Serial transfer data
    else if (pos == 0xff01) {
      //std::cerr << "Read Serial transfer data" << std::endl;

      *data = z80_.getMemory(pos);
    }
    // SC - Serial transfer control
    else if (pos == 0xff02) {
      *data = z80_.getMemory(pos);
    }
    // Divider Register
    else if (pos == 0xff04) {
      *data = z80_.getMemory(pos);
    }
    // Timer counter
    else if (pos == 0xff05) {
      *data = z80_.getMemory(pos);
    }
    // Timer Modulo
    else if (pos == 0xff06) {
      *data = z80_.getMemory(pos);
    }
    // Timer Control
    else if (pos == 0xff07) {
      *data = z80_.getMemory(pos);
    }
    // Interrupt Flag
    else if (pos == 0xff0f) {
      //std::cerr << "Read Interrupt Flag" << std::endl;

      *data = z80_.getMemory(pos);
    }
    // Sound Data
    else if (pos >= 0xff10 && pos <= 0xff3f) {
      *data = z80_.getMemory(pos);
    }
    // LCD Control
    else if (pos == 0xff40) {
      *data = z80_.getMemory(pos);
    }
    // LCDC Status (STAT)
    else if (pos == 0xff41) {
      *data = z80_.getMemory(pos);
    }
    // Scroll Y
    else if (pos == 0xff42) {
      *data = z80_.getMemory(pos);
    }
    // Scroll X
    else if (pos == 0xff43) {
      *data = z80_.getMemory(pos);
    }
    // LCDC Y-Coordinate
    else if (pos == 0xff44) {
      *data = z80_.getMemory(pos);
    }
    // LYC
    else if (pos == 0xff45) {
      //std::cerr << "Read LYC" << std::endl;

      *data = z80_.getMemory(pos);
    }
    // DMA Transfer and Start Address
    else if (pos == 0xff46) {
      *data = z80_.getMemory(pos);
    }
    // BG Palette Data
    else if (pos == 0xff47) {
      *data = z80_.getMemory(pos);
    }
    // Object Palette 0 Data
    else if (pos == 0xff48) {
      *data = z80_.getMemory(pos);
    }
    // Object Palette 1 Data
    else if (pos == 0xff49) {
      *data = z80_.getMemory(pos);
    }
    // Window Y Position
    else if (pos == 0xff4a) {
      *data = z80_.getMemory(pos);
    }
    // Window X Position
    else if (pos == 0xff4b) {
      *data = z80_.getMemory(pos);
    }
    // KEY1 - Prepare Speed Switch
    else if (pos == 0xff4d) {
      *data = (gameboy()->isDoubleSpeed() ? 0x80 : 0x00);

      // *data = z80_.getMemory(pos);
    }
    // VBK - VRAM Bank (CBG)
    else if (pos == 0xff4f) {
      *data = z80_.getMemory(pos);
    }
    // HDMA1 - New DMA Source, High (CBG)
    else if (pos == 0xff51) {
      *data = z80_.getMemory(pos);
    }
    // HDMA2 - New DMA Source, Low (CBG)
    else if (pos == 0xff52) {
      *data = z80_.getMemory(pos);
    }
    // HDMA3 - New DMA Destination, High (CBG)
    else if (pos == 0xff53) {
      *data = z80_.getMemory(pos);
    }
    // HDMA4 - New DMA Destination, Low (CBG)
    else if (pos == 0xff54) {
      *data = z80_.getMemory(pos);
    }
    // HDMA5 - New DMA Length/Mode/Start (CBG)
    else if (pos == 0xff55) {
      *data = 0; // always return zero (DMA complete)

      //*data = z80_.getMemory(pos);
    }
    // BCPS/BGPI - Background Palette Index (CGB)
    else if (pos == 0xff68) {
      *data = z80_.getMemory(pos);
    }
    // BCPD/BGPD - Background Palette Data (CGB)
    else if (pos == 0xff69) {
      *data = z80_.getMemory(pos);
    }
    // OCPS/OBPI - Sprite Palette Index
    else if (pos == 0xff6a) {
      *data = z80_.getMemory(pos);
    }
    // OCPD/OBPD - Sprite Palette Data
    else if (pos == 0xff6b) {
      *data = z80_.getMemory(pos);
    }
    // SVBK - WRAM Bank
    else if (pos == 0xff70) {
      *data = z80_.getMemory(pos);
    }
    else {
      *data = z80_.getMemory(pos);
    }
  }
  // High RAM (HRAM) (0xff80->0xfffe)
  else if (pos < 0xffff) {
    *data = z80_.getMemory(pos);
  }
  // Interrupt Enable Register
  else if (pos == 0xffff) {
    //std::cerr << "Read Interrupt Enabled" << std::endl;

    *data = z80_.getMemory(pos);
  }
  else {
    assert(false);

    *data = z80_.getMemory(pos);
  }
}

void
CGameBoyMemData::
memPreWrite(const uchar *data, ushort pos, ushort len)
{
  if (! isEnabled())
    return;

  for (ushort i = 0; i < len; ++i)
    memPreWriteData(pos + i, data[i]);
}

void
CGameBoyMemData::
memPreWriteData(ushort pos, uchar /*data*/)
{
  if (pos == 0xff40) {
    lastLCDC_ = z80_.getMemory(pos);
  }
}

void
CGameBoyMemData::
memPostWrite(const uchar *data, ushort pos, ushort len)
{
  if (! isEnabled())
    return;

  for (ushort i = 0; i < len; ++i)
    memPostWriteData(pos + i, data[i]);
}

void
CGameBoyMemData::
memPostWriteData(ushort pos, uchar data)
{
  // NOTE: already written to z80 memory

  // ROM is Read Only !! handled by flags and write trigger

  // 16kB ROM bank #0 (0x0000 - 0x3fff)
  if      (pos < 0x4000) {
    assert(false);
  }
  // 16kB switchable ROM bank (0x4000 - 0x7fff)
  else if (pos < 0x8000) {
    assert(false);
  }
  // 8kB Video RAM (0x8000 - 0x9fff)
  else if (pos < 0xa000) {
    //std::cerr << "Write Video RAM " << std::hex << pos << " " <<
    //             std::hex << int(data) << std::endl;

    if (gameboy_->isGBC()) {
      gameboy()->setVRam(gameboy_->vramBank(), pos - 0x8000, data);
    }

    if (pos < 0x9800)
      gameboy()->updateTiles();
    else
      gameboy()->updateScreen();
  }
  // 8kB switchable RAM bank (0xa000 - 0xbfff)
  else if (pos < 0xc000) {
    ushort pos1 = pos - 0xa000;

    gameboy()->setRam(pos1 + gameboy()->ramOffset(), data);
  }
  // 4kB Internal RAM (0xc000 - 0xcfff)
  else if (pos < 0xd000) {
    //std::cerr << "Write Internal RAM " << std::hex << pos << " " <<
    //             std::hex << int(data) << std::endl;
  }
  // 4kB Internal RAM (0xd000 - 0xdfff)
  else if (pos < 0xe000) {
    //std::cerr << "Write Internal RAM " << std::hex << pos << " " <<
    //             std::hex << int(data) << std::endl;
    if (gameboy_->isGBC()) {
      ushort pos1 = pos - 0xd000;

      ushort offset = (gameboy()->wramBank() - 1)*0x1000;

      gameboy()->setWRam(pos1 + offset, data);
    }
  }
  // Echo of 8kB Internal RAM (0xe000 - 0xfdff)
  else if (pos < 0xfe00) {
    z80_.setByte(pos - 0x2000, data);
  }
  // Sprite Attrib Memory (OAM) (0xfe00 - 0xfe9f)
  else if (pos < 0xfea0) {
    //std::cerr << "Write Sprite data " << std::hex << pos << " " <<
    //             std::hex << int(data) << std::endl;

    gameboy()->updateSprites();
  }
  // Empty but unusable for I/O (0xfea0 - 0xfeff)
  else if (pos < 0xff00) {
    //std::cerr << "Write I/O data " << std::hex << pos << " " <<
    //                                  std::hex << int(data) << std::endl;
  }
  // I/O ports (0xff00 - 0xff7f)
  else if (pos < 0xff80) {
    if      (pos == 0xff00) {
      //std::cerr << "Write Key data " << int(data) << std::endl;

      gameboy()->updateKeys();
    }
    // SB - Serial transfer data
    else if (pos == 0xff01) {
      //std::cerr << "Write Serial transfer data " << int(data) << std::endl;
    }
    // SC - Serial transfer control
    else if (pos == 0xff02) {
      //std::cerr << "Serial transfer control " << int(data) << std::endl;

      bool transfer = TST_BIT(data, 7);
    //bool speed    = TST_BIT(data, 1);
    //bool shift    = TST_BIT(data, 0);

      if (transfer) {
        char c = z80_.getMemory(0xff01);

        if (isprint(c) || isspace(c))
          std::cerr << c;
        else
          std::cerr << std::hex << int(c);
      }
    }
    // DIV - Divider Register
    else if (pos == 0xff04) {
    }
    // TIMA - Timer counter
    else if (pos == 0xff05) {
    }
    // TMA - Timer Modulo
    else if (pos == 0xff06) {
    }
    // TAC - Timer Control
    else if (pos == 0xff07) {
    }
    // Interrupt Flag
    else if (pos == 0xff0f) {
      //std::cerr << "Write Interrupt Flag" << int(data) << std::endl;
    }
    // NR10 - Channel 1 Sweep register
    else if (pos == 0xff10) {
    }
    // NR11 - Channel 1 Sound length/Wave pattern duty
    else if (pos == 0xff11) {
    }
    // NR12 - Channel 1 Volume Envelope
    else if (pos == 0xff12) {
    }
    // NR13 - Channel 1 Frequency lo
    else if (pos == 0xff13) {
    }
    // NR14 - Channel 1 Frequency hi
    else if (pos == 0xff14) {
    }
    // NR21 - Channel 2 Sound length/Wave pattern duty
    else if (pos == 0xff16) {
    }
    // NR22 - Channel 2 Volume Envelope
    else if (pos == 0xff17) {
    }
    // NR23 - Channel 2 Frequency lo
    else if (pos == 0xff18) {
    }
    // NR24 - Channel 2 Frequency hi
    else if (pos == 0xff19) {
    }
    // NR30 - Channel 3 Sound on/off (R/W)
    else if (pos == 0xff1a) {
    }
    // NR31 - Channel 3 Sound length/Wave pattern duty
    else if (pos == 0xff1b) {
    }
    // NR32 - Channel 3 Volume Envelope
    else if (pos == 0xff1c) {
    }
    // NR33 - Channel 3 Frequency lo
    else if (pos == 0xff1d) {
    }
    // NR34 - Channel 3 Frequency hi
    else if (pos == 0xff1e) {
    }
    // NR41 - Channel 4 Sound length/Wave pattern duty
    else if (pos == 0xff20) {
    }
    // NR42 - Channel 4 Volume Envelope
    else if (pos == 0xff21) {
    }
    // NR43 - Channel 4 Polynomial Counter
    else if (pos == 0xff22) {
    }
    // NR44 - Channel 4 Counter/consecutive; Initial
    else if (pos == 0xff23) {
    }
    // NR50 - Channel control / ON-OFF / Volume
    else if (pos == 0xff24) {
    }
    // NR51 - Selection of Sound output terminal
    else if (pos == 0xff25) {
    }
    // NR52 - Sound on/off
    else if (pos == 0xff26) {
    }
    // FF30-FF3F - Wave Pattern RAM
    else if (pos >= 0xff30 && pos <= 0xff3f) {
    }
    // LCDC
    else if (pos == 0xff40) {
      if (data != lastLCDC_) {
        std::cerr << "Write LCDC " << std::hex << int(data) << std::endl;
      }
    }
    // LCDC Status (STAT)
    else if (pos == 0xff41) {
      //std::cerr << "Write STAT " << std::hex << int(data) << std::endl;
    }
    else if (pos == 0xff42) {
      //std::cerr << "Write SCY " << std::hex << int(data) << std::endl;
    }
    else if (pos == 0xff43) {
      //std::cerr << "Write SCX " << std::hex << int(data) << std::endl;
    }
    else if (pos == 0xff44) {
      //std::cerr << "Write LY " << std::hex << int(data) << std::endl;
    }
    else if (pos == 0xff45) {
      //std::cerr << "Write LYC " << std::hex << int(data) << std::endl;
    }
    // DMA Transfer and Start Address
    else if (pos == 0xff46) {
      ushort addr = (data << 8);
      ushort len  = 0xa0;

      //std::cerr << "Write DMA (0xa0 bytes) from " << std::hex << int(addr) << std::endl;

      for (ushort i = 0; i < len; ++i) {
        z80_.setByte(0xFE00 + i, z80_.getByte(addr + i));
      }

      // DMA takes 160 ms (runs in parallel with Z80)
      //ushort cycles = z80_.msCycles(160);
    }
    // BGP - BG Palette Data
    else if (pos == 0xff47) {
      //std::cerr << "Write BGP " << std::hex << int(data) << std::endl;

      gameboy()->updatePalette();
    }
    // OBP0 - Object Palette 0 Data
    else if (pos == 0xff48) {
      //std::cerr << "Write OBP0 " << std::hex << int(data) << std::endl;

      gameboy()->updatePalette();
    }
    // OBP1 - Object Palette 1 Data
    else if (pos == 0xff49) {
      //std::cerr << "Write OBP1 " << std::hex << int(data) << std::endl;

      gameboy()->updatePalette();
    }
    // WY - Window Y Position
    else if (pos == 0xff4a) {
      if (data != z80_.getMemory(pos)) {
        //std::cerr << "Write WY " << std::hex << int(data) << std::endl;
      }
    }
    // WX - Window X Position
    else if (pos == 0xff4b) {
      if (data != z80_.getMemory(pos)) {
        //std::cerr << "Write WX " << std::hex << int(data) << std::endl;
      }
    }
    // KEY1 - Prepare Speed Switch
    else if (pos == 0xff4d) {
      std::cerr << "Prepare Speed Switch " << std::hex << int(data) << std::endl;

      gameboy()->setDoubleSpeed(data & 0x01);
    }
    // VBK - VRAM Bank (CBG)
    else if (pos == 0xff4f) {
      int bank = (data & 0x01 ? 1 : 0);

      if (bank != gameboy()->vramBank()) {
        std::cerr << "Set VRAM Bank " << std::hex << int(data) << std::endl;

        gameboy()->setVRamBank(bank);
      }
    }
    // HDMA1 - New DMA Source, High (CBG)
    else if (pos == 0xff51) {
    }
    // HDMA2 - New DMA Source, Low (CBG)
    else if (pos == 0xff52) {
    }
    // HDMA3 - New DMA Destination, High (CBG)
    else if (pos == 0xff53) {
    }
    // HDMA4 - New DMA Destination, Low (CBG)
    else if (pos == 0xff54) {
    }
    // HDMA5 - New DMA Length/Mode/Start (CBG)
    else if (pos == 0xff55) {
      ushort src  = ((z80_.getMemory(0xff51) << 8) || z80_.getMemory(0xff52) & 0xFFF0);
      ushort dst  = ((z80_.getMemory(0xff53) << 8) || z80_.getMemory(0xff54) & 0xFFF0);
      ushort len  = ((data & 0x7F) + 1)*0x10;
      int    mode = (data & 0x80 ? 1 : 0);

      std::cerr << "Write HDMA (" << std::hex << int(len) << " bytes) from " <<
                   std::hex << int(src) << " to " << std::hex << int(dst) <<
                   " Mode " << mode << std::endl;

      for (ushort i = 0; i < len; ++i) {
        z80_.setByte(dst + i, z80_.getByte(src + i));
      }

      // DMA takes 160 ms (runs in parallel with Z80)
      //ushort cycles = z80_.msCycles(160);
    }
    // BCPS/BGPI - Background Palette Index (CGB)
    else if (pos == 0xff68) {
      std::cerr << "Set Background Palette Index " << std::hex << int(data) << std::endl;
    }
    // BCPD/BGPD - Background Palette Data (CGB)
    else if (pos == 0xff69) {
      uchar ind = z80_.getMemory(0xff68);

      uchar ind1 = (ind & 0x3F);

      std::cerr << "Set Background Palette Data " << std::hex << int(ind1) << "=" <<
                                                     std::hex << int(data) << std::endl;

      gameboy()->setBgPaleteData(ind1, data);

      bool autoIncr = (ind & 0x80);

      if (autoIncr) {
        z80_.setMemory(0xff68, (ind1 + 1) | 0x80);
      }

      gameboy()->updatePalette();
    }
    // OCPS/OBPI - Sprite Palette Index
    else if (pos == 0xff6a) {
      std::cerr << "Set Sprite Palette Index " << std::hex << int(data) << std::endl;
    }
    // OCPD/OBPD - Sprite Palette Data
    else if (pos == 0xff6b) {
      uchar ind = z80_.getMemory(0xff6a);

      uchar ind1 = (ind & 0x3F);

      std::cerr << "Set Background Palette Data " << std::hex << int(ind1) << "=" <<
                                                     std::hex << int(data) << std::endl;

      gameboy()->setSpritePaleteData(ind1, data);

      bool autoIncr = (ind & 0x80);

      if (autoIncr) {
        z80_.setMemory(0xff6a, (ind1 + 1) | 0x80);
      }

      gameboy()->updatePalette();
    }
    // SVBK - WRAM Bank
    else if (pos == 0xff70) {
      int bank = (data & 0x07);

      if (bank == 0)
        bank = 1;

      if (bank != gameboy()->wramBank()) {
        std::cerr << "Set WRAM Bank " << std::hex << int(data) << std::endl;

        gameboy()->setWRamBank(bank);
      }
    }
    else {
      std::cerr << "I/O Write " << std::hex << pos << " : " <<
                                   std::hex << int(data) << std::endl;
    }
  }
  // High RAM (HRAM) (0xff80->0xfffe)
  else if (pos < 0xffff) {
    //std::cerr << "Write High RAM" << std::hex << pos << " : " <<
    //             std::hex << int(data) << std::endl;
  }
  // Interrupt Enable Register
  else if (pos == 0xffff) {
    //std::cerr << "Write Interrupt Enabled" << int(data) << std::endl;
  }
  else {
    assert(false);
  }
}

bool
CGameBoyMemData::
memTrigger(const uchar *data, ushort pos, ushort len)
{
  if (! isEnabled())
    return false;

  for (ushort i = 0; i < len; ++i)
    memTriggerData(data[i], pos + i);

  return true;
}

void
CGameBoyMemData::
memTriggerData(uchar data, ushort pos)
{
  if (pos >= 0x8000)
    return;

  //---

  uchar type = z80_.getByte(0x147);

  uchar mbc = 0;

  if      (type == 1 || type == 2 || type == 3)
    mbc = 1;
  else if (type == 5 || type == 6)
    mbc = 2;

  //---

  if      (mbc == 1) {
    // write 0x0000 -> 0x1fff enable/disable ram
    if      (pos < 0x2000) {
      // enable external ram

      bool enabled = ((data & 0xf) == 0x0a);

      if (enabled != gameboy()->isRamEnabled()) {
        gameboy()->setRamEnabled(enabled);

        if (enabled) {
          //std::cerr << "Enable external RAM" << std::endl;
        }
        else {
          //std::cerr << "Disable external RAM" << std::endl;
        }
      }
    }
    // write 0x2000 -> 0x3fff select ROM bank:
    else if (pos < 0x4000) {
      // set rom bank

      assert(type == 1 || type == 2 || type == 3);

      uchar bank = data & 0x1f;

      int maxBanks = gameboy()->cartridgeLen()/0x4000;

      if (bank > maxBanks)
        bank %= maxBanks;

      if (bank < 1)
        bank = 1;

      if (bank != gameboy()->romBank()) {
        gameboy()->setRomBank(bank);

        gameboy()->setRomOffset((bank - 1)*0x4000);

        //std::cerr << "MBC1: Set ROM bank (Low Bits) : " << std::hex << int(bank) <<
        //             "  Offset : "     << std::hex << gameboy()->romOffset() << std::endl;
      }
    }
    // write 0x4000 -> 0x5fff select RAM bank
    else if (pos < 0x6000) {
      // set ram bank

      assert(type == 1 || type == 2 || type == 3);

      if      (gameboy()->memoryModel() == 0) {
        uchar bank1 = (data & 0x03) << 5;
        uchar bank2 = (gameboy()->romBank() & 0x1f);

        uchar bank = bank1 | bank2;

        int maxBanks = gameboy()->cartridgeLen()/0x4000;

        if (bank > maxBanks)
          bank %= maxBanks;

        if (bank < 1)
          bank = 1;

        if (bank != gameboy()->romBank()) {
          gameboy()->setRomBank(bank);

          gameboy()->setRomOffset((bank - 1)*0x4000);

          // std::cerr << "MBC1: Set ROM bank (High Bits) : " << std::hex << int(bank) <<
          //            "  Offset : "     << std::hex << gameboy()->romOffset() << std::endl;
        }
      }
      else if (gameboy()->memoryModel() == 1) {
        uchar bank = data & 0x03;

        // TODO: check max ram banks

        if (bank != gameboy()->ramBank()) {
          gameboy()->setRamBank(bank);

          gameboy()->setRamOffset(bank*0x2000);

          // std::cerr << "MBC1: Set RAM bank : " << std::hex << int(bank) <<
          //              "  Offset : "     << std::hex << gameboy()->ramOffset() << std::endl;
        }
      }
    }
    // write to 0x6000 -> 0x7ffff selects mode:
    //   mode 0 : 16/8 mode : ROM mode (no RAM banks, uo to 2MB ROM)
    //   mode 1 : 4/32 mode : RAM mode (4 RAM banks, up to 512kb ROM)
    else if (pos < 0x8000) {
      // set mode

      uchar memoryModel = data & 0x01;

      if (gameboy()->memoryModel() != memoryModel) {
        //std::cerr << "Set Memory Model : " << int(memoryModel) << std::endl;

        gameboy()->setMemoryModel(memoryModel);
      }
    }
  }
  else if (mbc == 2) {
    if (pos >= 0x2100 && pos < 0x2200) {
      // set rom bank

      uchar bank = data & 0x1f;

      int maxBanks = gameboy()->cartridgeLen()/0x4000;

      if (bank > maxBanks)
        bank %= maxBanks;

      if (bank < 1)
        bank = 1;

      gameboy()->setRomBank(bank);

      gameboy()->setRomOffset((bank - 1)*0x4000);

      // std::cerr << "MBC2: Set ROM bank : " << std::hex << int(bank) <<
      //              "  Offset : "     << std::hex << gameboy()->romOffset() << std::endl;
    }
  }
}
