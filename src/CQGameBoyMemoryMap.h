#ifndef CQGameBoyMemoryMap_H
#define CQGameBoyMemoryMap_H

#include <QFrame>
#include <cassert>

class CQGameBoyMemoryMap;
class CQGameBoyMemoryRegions;
class CQGameBoyRegions;
class CQGameBoyRegionsCanvas;
class CQGameBoyBIOSRegions;
class CQGameBoyCartridgeRegions;
class CQGameBoyMemoryRegion;
class CQGameBoy;
class CQHexdump;
class QStackedWidget;
class QComboBox;
class QSpinBox;
class QToolButton;
class QLabel;

//---

class CQGameBoyMemoryMap : public QFrame {
  Q_OBJECT

 public:
  CQGameBoyMemoryMap(CQGameBoy *gameboy);

  CQGameBoy* gameboy() const { return gameboy_; }

  void setHexdumpRegion(CQGameBoyMemoryRegion *region);

  void updateHexdump();

 private slots:
  void regionChanged(int n);
  void bankSlot(int n);
  void asciiSlot();

 private:
  CQGameBoy*                 gameboy_     { nullptr };
  QComboBox*                 regionCombo_ { nullptr };
  QStackedWidget*            regionStack_ { nullptr };
  CQGameBoyMemoryRegions*    memory_      { nullptr };
  CQGameBoyBIOSRegions*      bios_        { nullptr };
  CQGameBoyCartridgeRegions* cartridge_   { nullptr };
  QLabel*                    hexLabel_    { nullptr };
  QSpinBox*                  memBankSpin_ { nullptr };
  QToolButton*               asciiButton_ { nullptr };
  CQHexdump*                 hexdump_     { nullptr };
};

//---

class CQGameBoyMemoryRegion {
 public:
  enum class Type {
    NONE,
    BIOS,
    CART,
    ROM0,
    ROM1,
    VRAM,
    ERAM,
    WRAM0,
    WRAM1,
    DUP,
    OAM,
    UNUSED,
    IO,
    HRAM,
    IE
  };

 public:
  CQGameBoyMemoryRegion(CQGameBoyRegions *regions, Type type, uint start, uint end,
                        const QString &name, int numBanks);

  int ind() const { return ind_; }
  void setInd(int i) { ind_ = i; }

  uint start() const { return start_; }
  uint end  () const { return end_  ; }

  const QString &name() const { return name_; }

  int numBanks() const { return numBanks_; }

  int bankNum() const { return bankNum_; }
  void setBankNum(int i) { bankNum_ = i; }

  int len() const { return end_ - start_ + 1; }

  bool contains(uint i) const { return (i >= start_ && i <= end_); }

  uchar data(uint i) const;

 private:
  CQGameBoyRegions* regions_  { nullptr };
  Type              type_     { Type::NONE };
  int               ind_      { 0 };
  uint              start_    { 0 };
  uint              end_      { 0 };
  QString           name_;
  int               numBanks_ { 1 };
  int               bankNum_  { 0 };
};

//---

class CQGameBoyRegions : public QFrame {
  Q_OBJECT

 public:
  typedef std::vector<CQGameBoyMemoryRegion *> Regions;

 public:
  CQGameBoyRegions(CQGameBoyMemoryMap *map);

  CQGameBoyMemoryMap *map() const { return map_; }

  const uint &minAddr() const { return minAddr_; }
  void setMinAddr(const uint &v) { minAddr_ = v; }

  const uint &maxAddr() const { return maxAddr_; }
  void setMaxAddr(const uint &v) { maxAddr_ = v; }

  const Regions &regions() const { return regions_; }

  CQGameBoyMemoryRegion *addRegion(CQGameBoyMemoryRegion::Type type, uint start, uint end,
                                   const QString &name, int num=1);

  CQGameBoyMemoryRegion *currentRegion() const { return region_; }
  void setCurrentRegion(CQGameBoyMemoryRegion *region);

  void setBankNum(int i);

  double map(uint addr) const {
    assert(addr >= minAddr_ && addr <= maxAddr_);

    return (1.0*(addr - minAddr_))/(maxAddr_ - minAddr_);
  }

 protected:
  void updateCurrentRegion();

 protected:
  CQGameBoyMemoryMap*     map_     { nullptr };
  uint                    minAddr_ { 0 };
  uint                    maxAddr_ { 65535 };
  Regions                 regions_;
  CQGameBoyMemoryRegion*  region_  { nullptr };
  CQGameBoyRegionsCanvas* canvas_  { nullptr };
};

//---

class CQGameBoyMemoryRegions : public CQGameBoyRegions {
  Q_OBJECT

 public:
  CQGameBoyMemoryRegions(CQGameBoyMemoryMap *map);
};

//---

class CQGameBoyBIOSRegions : public CQGameBoyRegions {
  Q_OBJECT

 public:
  CQGameBoyBIOSRegions(CQGameBoyMemoryMap *map);
};

//---

class CQGameBoyCartridgeRegions : public CQGameBoyRegions {
  Q_OBJECT

 public:
  CQGameBoyCartridgeRegions(CQGameBoyMemoryMap *map);
};

//---

class CQGameBoyRegionsCanvas : public QFrame {
  Q_OBJECT

 public:
  CQGameBoyRegionsCanvas(CQGameBoyRegions *regions);

  void paintEvent(QPaintEvent *pe);

  void mousePressEvent(QMouseEvent *e);

  void keyPressEvent(QKeyEvent *e);

  QSize sizeHint() const;

 private:
  CQGameBoyRegions *regions_ { nullptr };
};

#endif
