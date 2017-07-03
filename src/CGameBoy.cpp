#include <CGameBoy.h>
#include <CZ80ExecData.h>
#include <CZ80MemData.h>
#include <CZ80PortData.h>
#include <CEvent.h>
#include <CFile.h>

// 256 bytes 0x0000->0x0100
uchar bios[] = {
  0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
  0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
  0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
  0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
  0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
  0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
  0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
  0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
  0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xF2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
  0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
  0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
  0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
  0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
  0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3c, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x4C,
  0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
  0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50
};

//------

class CGameBoyExecData : public CZ80ExecData {
 public:
  CGameBoyExecData(CGameBoy *gameboy);

  CGameBoy *gameboy() const { return gameboy_; }

  void preStep () override;
  void postStep() override;

  void setStop(bool b) override;
  void setHalt(bool b) override;

 private:
  CGameBoy *gameboy_ { nullptr };
  ulong     m_       { 0 };
  ulong     timer_   { 0 };
};

//------

class CGameBoyMemData : public CZ80MemData {
 public:
  CGameBoyMemData(CGameBoy *gameboy);

  CGameBoy *gameboy() const { return gameboy_; }

  bool isEnabled() const { return enabled_; }
  void setEnabled(bool b) { enabled_ = b; }

  bool memRead(uchar *data, ushort pos, ushort);

  void memWrite(const uchar *data, ushort pos, ushort);

  bool memTrigger(const uchar *data, ushort pos, ushort);

 private:
  void memReadData(ushort pos, uchar *data);

  void memWriteData(ushort pos, uchar data);

  void memTriggerData(uchar data, ushort pos);

 private:
  CGameBoy *gameboy_ { nullptr };
  bool      enabled_ { true };
};

//------

class CGameBoyPortData : public CZ80PortData {
 public:
  CGameBoyPortData(CGameBoy *gameboy) :
   CZ80PortData(*gameboy->getZ80()), gameboy_(gameboy) {
  }

  CGameBoy *gameboy() const { return gameboy_; }

  void out(uchar port, uchar value);

  uchar in(uchar port, uchar qual);

  void keyPress  (const CKeyEvent &kevent);
  void keyRelease(const CKeyEvent &kevent);

 private:
  CGameBoy *gameboy_ { nullptr };
};

//------

CGameBoy::
CGameBoy()
{
  assert(sizeof(bios) == 256);

  CZ80 *z80 = getZ80();

  z80->setExecData(execData_ = new CGameBoyExecData(this));
  z80->setMemData (memData_  = new CGameBoyMemData (this));
  z80->setPortData(portData_ = new CGameBoyPortData(this));

  //---

  ram_ = new uchar [65536];

  memset(ram_, 0, 65536);

  //---

  z80->setMemFlags(0x0000, 0x8000, uint(CZ80MemType::READ_ONLY) |
                                   uint(CZ80MemType::WRITE_TRIGGER));
  z80->setMemFlags(0x8000, 0x2000, uint(CZ80MemType::SCREEN));
}

CGameBoy::
~CGameBoy()
{
  delete [] cartridge_;
  delete [] ram_;

  delete execData_;
  delete portData_;
  delete memData_;
}

bool
CGameBoy::
loadCartridge(const std::string &filename)
{
  CFile file(filename);

  if (! file.isRegular()) {
    std::cerr << "Invalid file '" << filename << "'" << std::endl;
    return false;
  }

  cartridgeLen_ = file.getSize();

  if (cartridgeLen_ < 32768) {
    std::cerr << "Invalid file size (< 32768) '" << filename << "'" << std::endl;
    return false;
  }

  cartridge_ = new uchar [cartridgeLen_];

  if (! file.read(cartridge_, cartridgeLen_)) {
    std::cerr << "Failed to read file '" << filename << "'" << std::endl;
    return false;
  }

  static size_t cartridgeSizes[] = {
      32768, // 0 - 256Kbit =  32KByte =   2 banks
      65536, // 1 - 512Kbit =  64KByte =   4 banks
     131072, // 2 -   1Mbit = 128KByte =   8 banks
     262144, // 3 -   2Mbit = 256KByte =  16 banks
     524288, // 4 -   4Mbit = 512KByte =  32 banks
    1048576, // 5 -   8Mbit =   1MByte =  64 banks
    2097152, // 6 -  16Mbit =   2MByte = 128 banks
  };

  int isize = cartridge_[0x0148];

  if (isize > 7 || cartridgeSizes[isize] != cartridgeLen_) {
    std::cerr << "Invalid cartridge size '" << filename << "'" << std::endl;
    return false;
  }

  cartridgeType_ = cartridge_[0x0147];

  CZ80 *z80 = getZ80();

  memData_->setEnabled(false);

  z80->resetMemFlags(0x0000, 0x8000, uint(CZ80MemType::READ_ONLY) |
                                     uint(CZ80MemType::WRITE_TRIGGER));

  z80->setBytes(cartridge_, 0, 0x8000);

  z80->setMemFlags(0x0000, 0x8000, uint(CZ80MemType::READ_ONLY) |
                                   uint(CZ80MemType::WRITE_TRIGGER));

  memData_->setEnabled(true);

  //---

  return true;
}

bool
CGameBoy::
loadAsm(const std::string &filename)
{
  CZ80 *z80 = getZ80();

  memData_->setEnabled(false);

  z80->resetMemFlags(0x0000, 0x8000, uint(CZ80MemType::READ_ONLY) |
                                     uint(CZ80MemType::WRITE_TRIGGER));

  bool rc = z80->load(filename);

  z80->setMemFlags(0x0000, 0x8000, uint(CZ80MemType::READ_ONLY) |
                                   uint(CZ80MemType::WRITE_TRIGGER));

  memData_->setEnabled(true);

  //---

  return rc;
}

bool
CGameBoy::
onScreen(ushort pos, ushort len)
{
  if (pos + len >= 0x9800 && pos <= 0x9fff)
    return true;

  return false;
}

ushort
CGameBoy::
getTileAddr(int bank, int tile) const
{
  ushort memStart;

  if (bank == 0) {
    if (tile <= 127)
      memStart = 0x9000;
    else
      memStart = 0x8000;
  }
  else {
    memStart = 0x8000;
  }

  return memStart + tile*16; // 16 bytes per tile
}

bool
CGameBoy::
getLineSprites(int line, int height, std::vector<CGameBoySprite> &sprites) const
{
  sprites.clear();

  CGameBoySprite sprite;

  for (int i = 0; i < 40; ++i) {
    getSprite(i, sprite);

    int y1 = sprite.y - 16;
    int y2 = y1 + height;

    if (line < y1 || line >= y2)
      continue;

    int x1 = sprite.x - 8;
    int x2 = x1 + 8;

    if (x2 < 0 || x1 >= 144)
      continue;

    sprites.push_back(sprite);
  }

  return ! sprites.empty();
}

bool
CGameBoy::
getSprite(int i, CGameBoySprite &sprite) const
{
  const CZ80 *z80 = getZ80();

  ushort memStart = 0xfe00;

  ushort p = memStart + 4*i;

  sprite.i = i;
  sprite.y = z80->getByte(p    );
  sprite.x = z80->getByte(p + 1);
  sprite.t = z80->getByte(p + 2);

  int f = z80->getByte(p + 3);

  sprite.priority = IS_BIT(f, 7);
  sprite.yflip    = IS_BIT(f, 6);
  sprite.xflip    = IS_BIT(f, 5);
  sprite.palNum1  = IS_BIT(f, 4);
//sprite.bankNum  = IS_BIT(f, 3);
//sprite.palNum2  = (f & 0x7);
  sprite.bankNum  = 0;
  sprite.palNum2  = 0;

  return true;
}

uchar
CGameBoy::
keySel() const
{
  return z80_.getMemory(0xff00) & 0x30;
}

void
CGameBoy::
keyPress(CKeyType key)
{
  if      (key == rightKey ()) { RESET_BIT(keys_[1], 0); }
  else if (key == leftKey  ()) { RESET_BIT(keys_[1], 1); }
  else if (key == upKey    ()) { RESET_BIT(keys_[1], 2); }
  else if (key == downKey  ()) { RESET_BIT(keys_[1], 3); }
  else if (key == aKey     ()) { RESET_BIT(keys_[0], 0); }
  else if (key == bKey     ()) { RESET_BIT(keys_[0], 1); }
  else if (key == selectKey()) { RESET_BIT(keys_[0], 2); }
  else if (key == startKey ()) { RESET_BIT(keys_[0], 3); }

  z80_.setBit(0xff0f, 4);
}

void
CGameBoy::
keyRelease(CKeyType key)
{
  if      (key == rightKey ()) { SET_BIT(keys_[1], 0); }
  else if (key == leftKey  ()) { SET_BIT(keys_[1], 1); }
  else if (key == upKey    ()) { SET_BIT(keys_[1], 2); }
  else if (key == downKey  ()) { SET_BIT(keys_[1], 3); }
  else if (key == aKey     ()) { SET_BIT(keys_[0], 0); }
  else if (key == bKey     ()) { SET_BIT(keys_[0], 1); }
  else if (key == selectKey()) { SET_BIT(keys_[0], 2); }
  else if (key == startKey ()) { SET_BIT(keys_[0], 3); }

  z80_.setBit(0xff0f, 4);
}

//----------

void
CGameBoyPortData::
out(uchar port, uchar value)
{
  std::cerr << "Out: Port " << (int) port << " Value " << (int) value << std::endl;
}

uchar
CGameBoyPortData::
in(uchar port, uchar value)
{
  std::cerr << "In: Port " << (int) port << " Value " << (int) value << std::endl;

  return 0;
}

void
CGameBoyPortData::
keyPress(const CKeyEvent &e)
{
  CKeyType type = CEvent::keyTypeToUpper(e.getType());

  gameboy_->keyPress(type);
}

void
CGameBoyPortData::
keyRelease(const CKeyEvent &e)
{
  CKeyType type = CEvent::keyTypeToUpper(e.getType());

  gameboy_->keyRelease(type);
}

//----------

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

//----------

CGameBoyMemData::
CGameBoyMemData(CGameBoy *gameboy) :
 CZ80MemData(*gameboy->getZ80()), gameboy_(gameboy)
{
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
memWrite(const uchar *data, ushort pos, ushort len)
{
  if (! isEnabled())
    return;

  for (ushort i = 0; i < len; ++i)
    memWriteData(pos + i, data[i]);
}

void
CGameBoyMemData::
memReadData(ushort pos, uchar *data)
{
  // BIOS at startup
  if      (pos < 0x100) {
    if (gameboy()->isBiosEnabled())
      *data = bios[pos];
    else
      *data = z80_.getMemory(pos);
  }
  // 16kB ROM bank #0
  else if (pos < 0x4000) {
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
  // 8kB Video RAM
  else if (pos < 0xa000) {
    *data = z80_.getMemory(pos);
  }
  // 8kB switchable RAM bank
  else if (pos < 0xc000) {
    ushort pos1 = pos - 0xa000;

    *data = gameboy()->getRam(pos1 + gameboy()->ramOffset());
  }
  // 8kB Internal RAM
  else if (pos < 0xe000) {
    *data = z80_.getMemory(pos);
  }
  // Echo of 8kB Internal RAM
  else if (pos < 0xfe00) {
    *data = z80_.getMemory(pos - 0x2000);
  }
  // Sprite Attrib Memory (OAM)  (0xfe00 - 0xfeaf)
  else if (pos < 0xfea0) {
    *data = z80_.getMemory(pos);
  }
  // Empty but unusable for I/O
  else if (pos < 0xff00) {
    *data = z80_.getMemory(pos);
  }
  // I/O ports
  else if (pos < 0xff4c) {
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
  }
  // Empty but unusable for I/O
  else if (pos < 0xff80) {
    *data = z80_.getMemory(pos);
  }
  // Internal RAM
  else {
    // Interrupt Enable
    if (pos == 0xffff) {
      //std::cerr << "Read Interrupt Enabled" << std::endl;

      *data = z80_.getMemory(pos);
    }
    else {
      *data = z80_.getMemory(pos);
    }
  }
}

void
CGameBoyMemData::
memWriteData(ushort pos, uchar data)
{
  // NOTE: already written to z80 memory

  // ROM is Read Only !! handled by flags and write trigger

  // 16kB ROM bank #0
  if      (pos < 0x4000) {
    assert(false);
  }
  // 16kB switchable ROM bank (0x4000 - 0x7fff)
  else if (pos < 0x8000) {
    assert(false);
  }
  // 8kB Video RAM (0x8000 - 0x9fff)
  else if (pos < 0xa000) {
    //std::cerr << "Write Video RAM " << std::hex << pos << std::hex << int(data) << std::endl;

    if (pos < 0x9800)
      gameboy()->updateTiles();
    else
      gameboy()->updateScreen();
  }
  // 8kB switchable RAM bank
  else if (pos < 0xc000) {
    ushort pos1 = pos - 0xa000;

    gameboy()->setRam(pos1 + gameboy()->ramOffset(), data);
  }
  // 8kB Internal RAM
  else if (pos < 0xe000) {
    //std::cerr << "Write Internal RAM " << std::hex << pos << std::hex << int(data) << std::endl;
  }
  // Echo of 8kB Internal RAM
  else if (pos < 0xfe00) {
    z80_.setByte(pos - 0x2000, data);
  }
  // Sprite Attrib Memory (OAM)  (0xfe00 - 0xfeaf)
  else if (pos < 0xfea0) {
    //std::cerr << "Write Sprite data " << std::hex << pos << std::hex << int(data) << std::endl;

    gameboy()->updateSprites();
  }
  // Empty but unusable for I/O
  else if (pos < 0xff00) {
    //std::cerr << "Write I/O data " << std::hex << pos << std::hex << int(data) << std::endl;
  }
  // I/O ports
  else if (pos < 0xff4c) {
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
    // Interrupt Flag
    else if (pos == 0xff0f) {
      //std::cerr << "Write Interrupt Flag" << int(data) << std::endl;
    }
    else if (pos == 0xff40) {
      if (data != z80_.getMemory(pos)) {
        //std::cerr << "Write LCDC " << std::hex << int(data) << std::endl;
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
    else if (pos == 0xff47) {
      //std::cerr << "Write BGP " << std::hex << int(data) << std::endl;

      gameboy()->updatePalette();
    }
    else if (pos == 0xff48) {
      //std::cerr << "Write OBP0 " << std::hex << int(data) << std::endl;

      gameboy()->updatePalette();
    }
    else if (pos == 0xff49) {
      //std::cerr << "Write OBP1 " << std::hex << int(data) << std::endl;

      gameboy()->updatePalette();
    }
    else if (pos == 0xff4a) {
      if (data != z80_.getMemory(pos)) {
        //std::cerr << "Write WY " << std::hex << int(data) << std::endl;
      }
    }
    else if (pos == 0xff4b) {
      if (data != z80_.getMemory(pos)) {
        //std::cerr << "Write WX " << std::hex << int(data) << std::endl;
      }
    }
  }
  // Empty but unusable for I/O
  else if (pos < 0xff80) {
    //std::cerr << "Write I/O data " << std::hex << pos << std::hex << int(data) << std::endl;
  }
  // Internal RAM
  else {
    // Interrupt Enable
    if (pos == 0xffff) {
      //std::cerr << "Write Interrupt Enabled" << int(data) << std::endl;
    }
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
