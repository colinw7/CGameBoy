#include <CGameBoy.h>
#include <CGameBoyExecData.h>
#include <CGameBoyMemData.h>
#include <CGameBoyPortData.h>
#include <CEvent.h>
#include <CFile.h>

namespace {

template<typename T>
T *allocMem(ulong size) {
  T *mem = new T [size];

  memset(mem, 0, size*sizeof(T));

  return mem;
}

}

//------

CGameBoy::
CGameBoy()
{
  CZ80 *z80 = getZ80();

  z80->setExecData(execData_ = new CGameBoyExecData(this));
  z80->setMemData (memData_  = new CGameBoyMemData (this));
  z80->setPortData(portData_ = new CGameBoyPortData(this));

  //---

  ram_ = allocMem<uchar>(0x10000);

  //---

  z80->setMemFlags(0x8000, 0x2000, uint(CZ80MemType::SCREEN));

  enableMemFlags(true);
}

CGameBoy::
~CGameBoy()
{
  delete [] cartridge_;
  delete [] ram_;
  delete [] vram_;
  delete [] wram_;

  delete execData_;
  delete portData_;
  delete memData_;
}

void
CGameBoy::
setDoubleSpeed(bool b)
{
  doubleSpeed_ = b;

  if (doubleSpeed_)
    getZ80()->setCPUHz(8*1024*1024);
  else
    getZ80()->setCPUHz(4*1024*1024);
}

uchar
CGameBoy::
biosData(ushort pos)
{
  return memData_->biosData(pos);
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

  cartridge_ = allocMem<uchar>(cartridgeLen_);

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

  //---

  // init state from cartridge
  if      (readCartridge(0x0143) == 0xc0)
    setGBC(true);
  else if (readCartridge(0x0143) == 0x80)
    setGBC(true); // and original GB

  if (readCartridge(0x0146) == 0x03)
    setSGB(true);

  setCartridgeType(readCartridge(0x0147));

  //---

  CZ80 *z80 = getZ80();

  memData_->setEnabled(false);

  enableMemFlags(false);

  // copy cartridge into Z80 mem
  z80->setBytes(cartridge_, 0, 0x8000);

  enableMemFlags(true);

  memData_->setEnabled(true);

  return true;
}

bool
CGameBoy::
loadAsm(const std::string &filename)
{
  CZ80 *z80 = getZ80();

  memData_->setEnabled(false);

  enableMemFlags(false);

  bool rc = z80->load(filename);

  enableMemFlags(true);

  memData_->setEnabled(true);

  //---

  return rc;
}

void
CGameBoy::
enableMemFlags(bool enable)
{
  CZ80 *z80 = getZ80();

  if (enable)
    z80->setMemFlags(0x0000, 0x8000, uint(CZ80MemType::READ_ONLY) |
                                     uint(CZ80MemType::WRITE_TRIGGER));
  else
    z80->resetMemFlags(0x0000, 0x8000, uint(CZ80MemType::READ_ONLY) |
                                       uint(CZ80MemType::WRITE_TRIGGER));
}

void
CGameBoy::
init()
{
  CZ80 *z80 = getZ80();

  z80->setSP(0xfffe);

  if (isGBC())
    z80->setAF(0x11b0);
  else
    z80->setAF(0x01b0);

  z80->setBC(0x0013);
  z80->setDE(0x00d8);
  z80->setHL(0x014d);
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

uchar
CGameBoy::
getTileNum(int tileMap, int tx, int ty) const
{
  ushort tileInd = ty*32 + tx; // 32 bytes per line, 1 byte per tile

  ushort tileMem = (tileMap == 0 ? 0x9800 : 0x9C00) + tileInd;

  const CZ80 *z80 = getZ80();

  //return z80->getByte(tileMem);

  if (isGBC()) {
    // get tile map index (from VRAM 0)
    return getVRam(0, tileMem - 0x8000);
  }
  else {
    return z80->getMemory(tileMem);
  }
}

void
CGameBoy::
getTileAttr(int tileMap, int tx, int ty, CGameBoyTileAttr &attr) const
{
  ushort tileInd = ty*32 + tx; // 32 bytes per line, 1 byte per tile

  ushort tileMem = (tileMap == 0 ? 0x9800 : 0x9C00) + tileInd;

  // get color attributes for tile (from VRAM 1)
  uchar cp = getVRam(1, tileMem - 0x8000);

  // Bit 0-2  Background Palette number (BGP0-7)
  // Bit 3    Tile VRAM Bank number     (0=Bank 0, 1=Bank 1)
  // Bit 4    Not used
  // Bit 5    Horizontal Flip           (0=Normal, 1=Mirror horizontally)
  // Bit 6    Vertical Flip             (0=Normal, 1=Mirror vertically)
  // Bit 7    BG-to-OAM Priority        (0=Use OAM priority bit, 1=BG Priority)

  attr.pnum     = cp & 0x7;
  attr.bank     = TST_BIT(cp, 3);
  attr.hflip    = TST_BIT(cp, 5);
  attr.vflip    = TST_BIT(cp, 6);
  attr.priority = TST_BIT(cp, 7);
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

    if (x2 < 0 || x1 >= int(getScreenPixelWidth()))
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

  if (isGBC()) {
    sprite.bankNum = IS_BIT(f, 3);
    sprite.palNum  = (f & 0x7);
  }
  else {
    sprite.bankNum = 0;
    sprite.palNum  = IS_BIT(f, 4);
  }

  return true;
}

void
CGameBoy::
setGBC(bool b)
{
  gbc_ = b;

  if (gbc_) {
    if (! vram_)
      vram_ = allocMem<uchar>(0x4000);

    if (! wram_)
      wram_ = allocMem<uchar>(0x8000);
  }
}

void
CGameBoy::
setSGB(bool b)
{
  sgb_ = b;
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
