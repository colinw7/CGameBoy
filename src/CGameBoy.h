#ifndef CGameBoy_H
#define CGameBoy_H

#include <CZ80.h>
#include <CRGBA.h>

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
  bool palNum1  { false };
  bool bankNum  { false };
  int  palNum2  { 0 };

  void print(std::ostream &os) const {
    os << "Sprite " << i << ":";
    os << " @(" << x << "," << y << ")";
    os << " tile " << t;
    os << " priority=" << priority;
    os << " yflip=" << yflip;
    os << " xflip=" << xflip;
    os << " palNum1=" << palNum1;
    os << " bankNum=" << bankNum;
    os << " palNum2=" << palNum2;
  }
};

class CGameBoy {
 public:
  CGameBoy();
 ~CGameBoy();

  void setInvert(bool invert) { invert_ = invert; }
  bool getInvert() { return invert_; }

  void setScale(int scale) { scale_ = scale; }
  int  getScale() const { return scale_; }

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

  uchar *cartridge() const { return cartridge_; }

  size_t cartridgeLen() const { return cartridgeLen_; }

  const uchar &romBank() const { return romBank_; }
  void setRomBank(const uchar &v) { romBank_ = v; }

  ushort romOffset() const { return romOffset_; }
  void setRomOffset(ushort s) { romOffset_ = s; }

  const uchar &ramBank() const { return ramBank_; }
  void setRamBank(const uchar &v) { ramBank_ = v; }

  uchar *ram() const { return ram_; }

  uchar getRam(ushort pos) const { return ram_[pos]; }
  void setRam(ushort pos, uchar data) { ram_[pos] = data; }

  ushort ramOffset() const { return ramOffset_; }
  void setRamOffset(ushort s) { ramOffset_ = s; }

  bool loadCartridge(const std::string &fileName);

  bool onScreen(ushort pos, ushort len);

  bool getScreenPos(ushort pos, int *x, int *y);

  bool getSprite(int i, CGameBoySprite &sprite);

 private:
  CZ80              z80_;
  CGameBoyExecData *execData_      { nullptr };
  CGameBoyMemData  *memData_       { nullptr };
  CGameBoyPortData *portData_      { nullptr }; // needed ?
  bool              invert_        { false };
  int               scale_         { 1 };
  bool              biosEnabled_   { true };
  uchar*            cartridge_     { nullptr };
  size_t            cartridgeLen_  { 0 };
  uchar             cartridgeType_ { 0 };
  uchar             romBank_       { 1 };
  ushort            romOffset_     { 0x0000 };
  uchar             romMode_       { 0 }; // 0: 8k RAM, 1: 32k RAM
  uchar*            ram_           { nullptr };
  uchar             ramBank_       { 0 };
  ushort            ramOffset_     { 0x0000 };
};

#endif
