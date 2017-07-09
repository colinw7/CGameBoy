#ifndef CGameBoy_H
#define CGameBoy_H

#include <CZ80.h>
#include <CRGBA.h>
#include <CEvent.h>

class CGameBoyExecData;
class CGameBoyMemData;
class CGameBoyPortData;

struct CGameBoySprite {
  int i { 0 };
  int x { 0 };
  int y { 0 };
  int t { 0 };

  bool priority { false };
  bool yflip    { false };
  bool xflip    { false };
  int  bankNum  { 0 };
  int  palNum   { 0 };

  void print(std::ostream &os) const {
    os << "Sprite " << i << ":";
    os << " @(" << x << "," << y << ")";
    os << " tile " << t;
    os << " priority=" << priority;
    os << " yflip=" << yflip;
    os << " xflip=" << xflip;
    os << " bankNum=" << bankNum;
    os << " palNum=" << palNum;
  }
};

class CGameBoy {
 public:
  // LCDC - LCD Control (0xff40) (Bits Low to High 0-7)
  struct LCDC {
    uchar bgDisplay:1;         // Bit 0 - BG Display (for CGB see below) (0=Off, 1=On)
    uchar spriteDispEnable:1;  // Bit 1 - OBJ (Sprite) Display Enable    (0=Off, 1=On)
    uchar spriteSize:1;        // Bit 2 - OBJ (Sprite) Size              (0=8x8, 1=8x16)
    uchar bgTileMapDispSel:1;  // Bit 3 - BG Tile Map Display Select     (0=9800-9BFF, 1=9C00-9FFF)
    uchar bgWinTileDataSel:1;  // Bit 4 - BG & Window Tile Data Select   (0=8800-97FF, 1=8000-8FFF)
    uchar winDispEnable:1;     // Bit 5 - Window Display Enable          (0=Off, 1=On)
    uchar winTileMapDispSel:1; // Bit 6 - Window Tile Map Display Select (0=9800-9BFF, 1=9C00-9FFF)
    uchar lcdDispEnable:1;     // Bit 7 - LCD Display Enable             (0=Off, 1=On)
  };

  // IF - Interrupt Flag (0xffff)
  struct InterruptFlag {
    uchar vblank:1;
    uchar lcdc  :1;
    uchar timer :1;
    uchar serial:1;
    uchar key   :1;
    uchar pad   :3;
  };

  // IE - Interrupt Enable (0xffff)
  struct InterruptEnable {
    uchar vblank:1;
    uchar lcdc  :1;
    uchar timer :1;
    uchar serial:1;
    uchar key   :1;
    uchar pad   :3;
  };

 public:
  CGameBoy();

  virtual ~CGameBoy();

  void setInvert(bool invert) { invert_ = invert; }
  bool getInvert() { return invert_; }

  void setScale(int scale) { scale_ = scale; }
  int  getScale() const { return scale_; }

  const CZ80 *getZ80() const { return &z80_; }

  CZ80 *getZ80() { return &z80_; }

  CGameBoyPortData *portData() const { return portData_; }

  uint mainRamSize () const { return 8096; }
  uint videoRamSize() const { return 8096; }

  double screenWidth () const { return 2.6; /* inches */ }
  double screenHeight() const { return 2.6; /* inches */ }

  uint getScreenPixelWidth () const { return 160; }
  uint getScreenPixelHeight() const { return 144; }

  uint getScreenTilesWidth () const { return 20; }
  uint getScreenTilesHeight() const { return 18; }

  uint maxNumSprites () const { return 40; }
  uint maxLineSprites() const { return 10; }

  uint maxSpriteWidth () const { return 8; }
  uint maxSpriteHeight() const { return 16; }

  uint minSpriteWidth () const { return 8; }
  uint minSpriteHeight() const { return 8; }

  uint getHBlank() const { return 32; }
  uint getVBlank() const { return 10; }

  double clockSpeed() const { return 4.194304; /* MHz */ }

  bool isBiosEnabled() const { return biosEnabled_; }
  void setBiosEnabled(bool b) { biosEnabled_ = b; }

  //---

  uchar *cartridge() const { return cartridge_; }

  uchar readCartridge(uint pos) const { return cartridge_[pos]; }

  size_t cartridgeLen() const { return cartridgeLen_; }

  uchar romBank() const { return romBank_; }
  void setRomBank(uchar v) { romBank_ = v; }

  uint romOffset() const { return romOffset_; }
  void setRomOffset(uint s) { romOffset_ = s; }

  //---

  bool isRamEnabled() const { return ramEnabled_; }
  void setRamEnabled(bool b) { ramEnabled_ = b; }

  uchar ramBank() const { return ramBank_; }
  void setRamBank(uchar v) { ramBank_ = v; }

  uchar *ram() const { return ram_; }

  uchar getRam(ushort pos) const { return ram_[pos]; }
  void setRam(ushort pos, uchar data) { ram_[pos] = data; }

  ushort ramOffset() const { return ramOffset_; }
  void setRamOffset(ushort s) { ramOffset_ = s; }

  //---

  uchar vramBank() const { return vramBank_; }
  void setVRamBank(uchar i) { vramBank_ = i; }

  uchar *vram() const { return vram_; }

  uchar getVRam(uchar bank, ushort pos) const {
    assert(bank < 2 && pos < 0x2000);
    return (bank == 0 ? vram_[pos] : vram_[pos + 0x2000]);
  }
  void setVRam(uchar bank, ushort pos, uchar data) {
    assert(bank < 2 && pos < 0x2000);
    uchar *p = (bank == 0 ? &vram_[pos] : &vram_[pos + 0x2000]);
    *p = data;
  }

  //---

  uchar wramBank() const { return wramBank_; }
  void setWRamBank(uchar i) { wramBank_ = i; }

  uchar *wram() const { return wram_; }

  uchar getWRam(ushort pos) const { assert(pos < 0x8000); return wram_[pos]; }
  void setWRam(ushort pos, uchar data) { assert(pos < 0x8000); wram_[pos] = data; }

  //---

  uchar memoryModel() const { return memoryModel_; }
  void setMemoryModel(uchar v) { memoryModel_ = v; }

  bool isDoubleSpeed() const { return doubleSpeed_; }
  void setDoubleSpeed(bool b);

  //---

  bool loadCartridge(const std::string &fileName);
  bool loadAsm(const std::string &fileName);

  bool onScreen(ushort pos, ushort len);

  ushort getTileAddr(int bank, int tile) const;

  bool getLineSprites(int line, int height, std::vector<CGameBoySprite> &sprites) const;

  bool getSprite(int i, CGameBoySprite &sprite) const;

  uchar interruptFlag  () const { return z80_.getMemory(0xff0f); }
  uchar interruptEnable() const { return z80_.getMemory(0xffff); }

  bool isGBC() const { return gbc_; }
  void setGBC(bool b);

  uchar keySel() const;

  uchar key0() const { return keys_[0] & 0x0F; }
  uchar key1() const { return keys_[1] & 0x0F; }

  virtual void updateKeys   () { }
  virtual void updateTiles  () { }
  virtual void updateScreen () { }
  virtual void updateSprites() { }
  virtual void updatePalette() { }

  virtual void keyPress  (CKeyType key);
  virtual void keyRelease(CKeyType key);

  virtual void execStop(bool) { }
  virtual void execHalt(bool) { }

  // Start : Home, Select : Return, A : A, B : B
  CKeyType aKey     () const { return CKEY_TYPE_A;      }
  CKeyType bKey     () const { return CKEY_TYPE_B;      }
  CKeyType leftKey  () const { return CKEY_TYPE_Left;   }
  CKeyType rightKey () const { return CKEY_TYPE_Right;  }
  CKeyType upKey    () const { return CKEY_TYPE_Up;     }
  CKeyType downKey  () const { return CKEY_TYPE_Down;   }
  CKeyType selectKey() const { return CKEY_TYPE_Return; }
  CKeyType startKey () const { return CKEY_TYPE_Home;   }

  //---

  void setBgPaleteData(uchar ind, uchar data) {
    bgPalette_.setData(ind, data);
  }

  void setSpritePaleteData(uchar ind, uchar data) {
    spritePalette_.setData(ind, data);
  }

  void bgPaletteColor(uchar palette, uchar color, uchar &r, uchar &g, uchar &b) const {
    bgPalette_.paletteColor(palette, color, r, g, b);
  }

  void spritePaletteColor(uchar palette, uchar color, uchar &r, uchar &g, uchar &b) const {
    spritePalette_.paletteColor(palette, color, r, g, b);
  }

  //---

 private:
  struct ColorPalette {
    // 8 palettes, 4 colors per palette, 2 bytes per color
    uchar c[64];

    uchar getData(uchar ind) const {
      assert(ind < 64); return c[ind];
    }

    void setData(uchar ind, uchar data) {
      assert(ind < 64); c[ind] = data;
    }

    void paletteColor(uchar palette, uchar color, uchar &r, uchar &g, uchar &b) const {
      uchar c1 = getData(palette*8 + color*2 + 0);
      uchar c2 = getData(palette*8 + color*2 + 1);

      r = (( c1 & 0x1f      )                  ) << 3;
      g = (((c1 & 0xe0) >> 5) | (c2 & 0x3 << 3)) << 3;
      b = (((c2 & 0x7c) >> 2)                  ) << 3;
    }
  };

  //---

 private:
  CZ80              z80_;
  CGameBoyExecData *execData_      { nullptr };
  CGameBoyMemData  *memData_       { nullptr };
  CGameBoyPortData *portData_      { nullptr }; // needed ?
  bool              invert_        { false };
  bool              gbc_           { false };
  int               scale_         { 1 };
  bool              biosEnabled_   { true };
  uchar*            cartridge_     { nullptr };
  size_t            cartridgeLen_  { 0 };
  uchar             cartridgeType_ { 0 };
  uchar             romBank_       { 1 };
  uint              romOffset_     { 0x0000 };
  uchar             romMode_       { 0 }; // 0: 8k RAM, 1: 32k RAM
  uchar*            ram_           { nullptr };
  uchar             ramBank_       { 0 };
  ushort            ramOffset_     { 0x0000 };
  bool              ramEnabled_    { true };
  uchar             vramBank_      { 0 };
  uchar*            vram_          { nullptr };
  uchar             wramBank_      { 1 };
  uchar*            wram_          { nullptr };
  uchar             memoryModel_   { 0 };
  bool              doubleSpeed_   { false };
  uchar             keys_[2]       { 0x0f, 0x0f };
  ColorPalette      bgPalette_;
  ColorPalette      spritePalette_;
};

#endif
