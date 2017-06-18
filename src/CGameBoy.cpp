#include <CGameBoy.h>
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

  void preStep ();
  void postStep();

  int timerOverflow();

 private:
  CGameBoy *gameboy_ { nullptr };
};

//------

class CGameBoyMemData : public CZ80MemData {
 public:
  CGameBoyMemData(CGameBoy *gameboy);

  bool isEnabled() const { return enabled_; }
  void setEnabled(bool b) { enabled_ = b; }

  bool memRead(uchar *data, ushort pos, ushort);

  void memWrite(const uchar *data, ushort pos, ushort);

  bool memTrigger(const uchar *data, ushort pos, ushort);

 private:
  void memReadData(uchar *data, ushort pos);

  void memWriteData(uchar data, ushort pos);

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

  void out(uchar port, uchar value);

  uchar in(uchar port, uchar qual);

  void keyPress  (const CKeyEvent &kevent);
  void keyRelease(const CKeyEvent &kevent);

  uchar key0() const { return keys_[0]; }
  uchar key1() const { return keys_[1]; }

 private:
  CGameBoy *gameboy_    { nullptr };
  bool      keyHandled_ { false };
  uchar     keys_[2]    { 0x0f, 0x0f };
};

//------

CGameBoy::
CGameBoy()
{
  assert(sizeof(bios) == 256);

  z80_.setExecData(execData_ = new CGameBoyExecData(this));
  z80_.setMemData (memData_  = new CGameBoyMemData (this));
  z80_.setPortData(portData_ = new CGameBoyPortData(this));

  //---

  ram_ = new uchar [65536];

  //---

  z80_.setMemFlags(0x0000, 0x8000, uint(CZ80MemType::READ_ONLY) |
                                   uint(CZ80MemType::WRITE_TRIGGER));
  z80_.setMemFlags(0x8000, 0x2000, uint(CZ80MemType::SCREEN));
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

  z80_.resetMemFlags(0x0000, 0x8000, uint(CZ80MemType::READ_ONLY) |
                                     uint(CZ80MemType::WRITE_TRIGGER));

  z80->setBytes(cartridge_, 0, 0x8000);

  z80_.setMemFlags(0x0000, 0x8000, uint(CZ80MemType::READ_ONLY) |
                                   uint(CZ80MemType::WRITE_TRIGGER));

  memData_->setEnabled(true);

  //---

  return true;
}

bool
CGameBoy::
onScreen(ushort pos, ushort len)
{
  if (pos + len >= 0x9800 && pos <= 0x9fff)
    return true;

  return false;
}

bool
CGameBoy::
getScreenPos(ushort, int *x, int *y)
{
  *x = 0;
  *y = 0;

  return false;
}

bool
CGameBoy::
getSprite(int i, CGameBoySprite &sprite)
{
  CZ80 *z80 = getZ80();

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
  sprite.bankNum  = IS_BIT(f, 3);
  sprite.palNum2  = (f & 0x7);

  return true;
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

  // Start : Home, Select : Return, A : A, B : B
  if      (type == CKEY_TYPE_Right ) { keys_[1] &= 0x0e; }
  else if (type == CKEY_TYPE_Left  ) { keys_[1] &= 0x0d; }
  else if (type == CKEY_TYPE_Up    ) { keys_[1] &= 0x0b; }
  else if (type == CKEY_TYPE_Down  ) { keys_[1] &= 0x07; }
  else if (type == CKEY_TYPE_A     ) { keys_[0] &= 0x0e; }
  else if (type == CKEY_TYPE_B     ) { keys_[0] &= 0x0d; }
  else if (type == CKEY_TYPE_Return) { keys_[0] &= 0x0b; }
  else if (type == CKEY_TYPE_Home  ) { keys_[0] &= 0x07; }
}

void
CGameBoyPortData::
keyRelease(const CKeyEvent &e)
{
  CKeyType type = CEvent::keyTypeToUpper(e.getType());

  // Start : Home, Select : Return, A : A, B : B
  if      (type == CKEY_TYPE_Right ) { keys_[1] |= 0x01; }
  else if (type == CKEY_TYPE_Left  ) { keys_[1] |= 0x02; }
  else if (type == CKEY_TYPE_Up    ) { keys_[1] |= 0x04; }
  else if (type == CKEY_TYPE_Down  ) { keys_[1] |= 0x08; }
  else if (type == CKEY_TYPE_A     ) { keys_[0] |= 0x01; }
  else if (type == CKEY_TYPE_B     ) { keys_[0] |= 0x02; }
  else if (type == CKEY_TYPE_Return) { keys_[0] |= 0x04; }
  else if (type == CKEY_TYPE_Home  ) { keys_[0] |= 0x08; }
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
  // vertical blank
  uchar c = z80_.getByte(0xff0f);

  if      (IS_BIT(c, 0)) {
    z80_.resetBit(0xff0f, 0);

    z80_.setIM0(0x40);

    z80_.interrupt();

    //std::cerr << "vertical blank interrupt" << std::endl;
  }
  // timer overflow
  else if (IS_BIT(c, 2)) {
    z80_.resetBit(0xff0f, 2);

    z80_.setIM0(0x50);

    z80_.interrupt();

    //std::cerr << "timer interrupt" << std::endl;
  }
}

int
CGameBoyExecData::
timerOverflow()
{
  z80_.setBit(0xff0f, 2);

  return 0;
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
    memReadData(&data[i], pos + i);

  return true;
}

void
CGameBoyMemData::
memReadData(uchar *data, ushort pos)
{
  if (pos < 0x100) {
    if (gameboy_->isBiosEnabled())
      *data = bios[pos];
    else
      *data = z80_.getMemory(pos);

    return;
  }

  // 16kB switchable ROM bank
  if      (pos <= 0x4000 && pos < 0x8000) {
    if (gameboy_->cartridge())
      *data = gameboy_->readCartridge(pos + gameboy_->romOffset());
    else
      *data = z80_.getMemory(pos);
  }
  // 8kB switchable RAM bank
  else if (pos <= 0xA000 && pos < 0xC000) {
    int pos1 = pos - 0xA000;

    *data = gameboy_->getRam(pos1 + gameboy_->ramOffset());
  }
  // Echo of 8kB Internal RAM
  else if (pos >= 0xe000 && pos < 0xfe00) {
    *data = z80_.getMemory(pos - 0x2000);
  }
  // Key Data
  else if (pos == 0xff00) {
    int data1 = z80_.getMemory(pos) & 0x30;

    if      (data1 == 0x10)
      *data = gameboy_->portData()->key0();
    else if (data1 == 0x20)
      *data = gameboy_->portData()->key1();
    else
      *data = 0;
  }
  else
    *data = z80_.getMemory(pos);
}

void
CGameBoyMemData::
memWrite(const uchar *data, ushort pos, ushort len)
{
  if (! isEnabled())
    return;

  for (ushort i = 0; i < len; ++i)
    memWriteData(data[i], pos + i);
}

void
CGameBoyMemData::
memWriteData(uchar data, ushort pos)
{
  // ROM is Read Only !! handled by flags and write trigger

  // 8kB switchable RAM bank
  if      (pos <= 0xA000 && pos < 0xC000) {
    int pos1 = pos - 0xA000;

    gameboy_->setRam(pos1 + gameboy_->ramOffset(), data);
  }
  // Echo of 8kB Internal RAM
  else if (pos >= 0xe000 && pos < 0xfe00) {
    z80_.setByte(pos - 0x2000, data);
  }

  // NOTE: will still write to z80 memory
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
  if      (pos < 0x2000) {
    // enable external ram

    uchar enable = data & 0x0a;

    if (enable) {
      //std::cerr << "Enable external RAM" << std::endl;
    }
  }
  else if (pos >= 0x2000 && pos < 0x4000) {
    // set bank

    uchar bank = data & 0x1f;

    int maxBanks = gameboy_->cartridgeLen()/0x4000;

    if (bank > maxBanks)
      bank %= maxBanks;

    if (bank < 1)
      bank = 1;

    gameboy_->setRomBank(bank);

    gameboy_->setRomOffset((bank - 1)*0x4000);

    std::cerr << "Set ROM bank : " << std::hex << int(bank) <<
                 "  Offset : "     << std::hex << gameboy_->romOffset() << std::endl;
  }
  else if (pos >= 0x4000 && pos < 0x6000) {
    // set bank

    uchar bank = data & 0x03;

    gameboy_->setRamBank(bank);

    gameboy_->setRamOffset(bank*0x2000);

    std::cerr << "Set RAM bank : " << std::hex << int(bank) << std::endl;
  }
  else if (pos >= 0x6000 && pos < 0x8000) {
    // set mode

    std::cerr << "Set Mode : " << int(data) << std::endl;
  }
}
