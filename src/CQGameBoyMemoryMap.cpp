#include <CQGameBoyMemoryMap.h>
#include <CQGameBoy.h>
#include <CQHexdump.h>
#include <QStackedWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QToolButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>

#include <svg/ascii_svg.h>

class CQGameBoyMemoryRegionDump : public CQHexdumpDataIFace {
 public:
  CQGameBoyMemoryRegionDump(CQGameBoyMemoryMap *map, CQGameBoyMemoryRegion *region) :
   map_(map), region_(region) {
  }

  std::size_t offset() const override { return region_->start(); }

  std::size_t size() const override { return region_->len(); }

  uchar data(uint i) const override {
    return region_->data(i);
  }

 private:
  CQGameBoyMemoryMap*    map_    { nullptr };
  CQGameBoyMemoryRegion* region_ { nullptr };
};

//------

CQGameBoyMemoryMap::
CQGameBoyMemoryMap(CQGameBoy *gameboy) :
 gameboy_(gameboy)
{
  setObjectName("memoryMap");

  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setMargin(2); layout->setSpacing(2);

  //---

  QFrame *regionFrame = new QFrame;

  regionFrame->setObjectName("regionFrame");
  regionFrame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  QVBoxLayout *regionLayout = new QVBoxLayout(regionFrame);
  regionLayout->setMargin(2); regionLayout->setSpacing(2);

  regionCombo_ = new QComboBox;

  regionCombo_->setObjectName("regionCombo");

  regionCombo_->addItem("Memory");
  regionCombo_->addItem("BIOS");

  if (gameboy->cartridge())
    regionCombo_->addItem("Cartridge");

  connect(regionCombo_, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this, &CQGameBoyMemoryMap::regionChanged);

  regionLayout->addWidget(regionCombo_);

  regionStack_ = new QStackedWidget;

  regionStack_->setObjectName("regionStack");

  memory_ = new CQGameBoyMemoryRegions   (this);
  bios_   = new CQGameBoyBIOSRegions     (this);

  regionStack_->addWidget(memory_);
  regionStack_->addWidget(bios_);

  if (gameboy->cartridge()) {
    cartridge_ = new CQGameBoyCartridgeRegions(this);

    regionStack_->addWidget(cartridge_);
  }

  regionLayout->addWidget(regionStack_);

  layout->addWidget(regionFrame);

  //---

  QVBoxLayout *hexLayout = new QVBoxLayout;
  hexLayout->setMargin(2); hexLayout->setSpacing(2);

  QHBoxLayout *controlLayout = new QHBoxLayout;
  controlLayout->setMargin(2); controlLayout->setSpacing(2);

  hexLabel_ = new QLabel;

  hexLabel_->setObjectName("hexLabel");

  memBankSpin_ = new QSpinBox;

  memBankSpin_->setObjectName("memBankSpin");
  memBankSpin_->setRange(0, 100);

  connect(memBankSpin_, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
          this, &CQGameBoyMemoryMap::bankSlot);

  controlLayout->addWidget(hexLabel_);
  controlLayout->addWidget(memBankSpin_);

  asciiButton_ = new QToolButton;

  asciiButton_->setObjectName("ascii");
  asciiButton_->setCheckable(true);
  asciiButton_->setIcon(CQPixmapCacheInst->getIcon("ASCII"));

  connect(asciiButton_, &QToolButton::clicked, this, &CQGameBoyMemoryMap::asciiSlot);

  controlLayout->addStretch(1);
  controlLayout->addWidget(asciiButton_);

  hexLayout->addLayout(controlLayout);

  //---

  hexdump_ = new CQHexdump;

  hexdump_->setShowAddress(true);
  hexdump_->setShowAscii(false);

  hexLayout->addWidget(hexdump_);

  layout->addLayout(hexLayout);
}

void
CQGameBoyMemoryMap::
regionChanged(int n)
{
  regionStack_->setCurrentIndex(n);
}

void
CQGameBoyMemoryMap::
bankSlot(int n)
{
  memory_->setBankNum(n);
}

void
CQGameBoyMemoryMap::
asciiSlot()
{
  hexdump_->setShowAscii(asciiButton_->isChecked());
}

void
CQGameBoyMemoryMap::
setHexdumpRegion(CQGameBoyMemoryRegion *region)
{
  if (region) {
    //std::cerr << "Region: " << region->ind() << std::endl;

    CQGameBoyMemoryRegionDump *dump = new CQGameBoyMemoryRegionDump(this, region);

    hexdump_->setData(dump);

    //---

    auto text = QString::asprintf("%04x - %04x : ", region->start(), region->end());

    text += region->name();

    if (region->numBanks() > 1) {
      text += QString("(%1 of %2)").arg(region->bankNum()).arg(region->numBanks() - 1);
    }

    hexLabel_->setText(text);

    if (region->numBanks() > 1) {
      memBankSpin_->setVisible(true);

      memBankSpin_->setRange(0, region->numBanks() - 1);
      memBankSpin_->setValue(region->bankNum());
    }
    else {
      memBankSpin_->setVisible(false);
    }
  }
  else {
    memBankSpin_->setVisible(false);
  }
}

void
CQGameBoyMemoryMap::
updateHexdump()
{
  hexdump_->updateText();
}

//------

CQGameBoyMemoryRegions::
CQGameBoyMemoryRegions(CQGameBoyMemoryMap *map) :
 CQGameBoyRegions(map)
{
  CQGameBoy *gameboy = map_->gameboy();

  addRegion(CQGameBoyMemoryRegion::Type::ROM0  , 0x0000, 0x3FFF, "16KB ROM Bank #0");
  addRegion(CQGameBoyMemoryRegion::Type::ROM1  , 0x4000, 0x7FFF, "16KB ROM Bank #1",
            gameboy->maxRomBanks());
  addRegion(CQGameBoyMemoryRegion::Type::VRAM  , 0x8000, 0x9FFF, "8KB Video RAM",
            gameboy->isGBC() ? 2 : 1);
  addRegion(CQGameBoyMemoryRegion::Type::ERAM  , 0xA000, 0xBFFF, "8KB External RAM", 8);
  addRegion(CQGameBoyMemoryRegion::Type::WRAM0 , 0xC000, 0xCFFF, "4KB Work RAM Bank #0");
  addRegion(CQGameBoyMemoryRegion::Type::WRAM1 , 0xD000, 0xDFFF, "4KB Work RAM Bank #1", 7);
  addRegion(CQGameBoyMemoryRegion::Type::DUP   , 0xE000, 0xFDFF, "Duplicate of 0xC000-0xDDFF");
  addRegion(CQGameBoyMemoryRegion::Type::OAM   , 0xFE00, 0xFE9F, "Sprite Attribute Table (OAM)");
  addRegion(CQGameBoyMemoryRegion::Type::UNUSED, 0xFEA0, 0xFEFF, "Not Usable");
  addRegion(CQGameBoyMemoryRegion::Type::IO    , 0xFF00, 0xFF7F, "I/O Ports");
  addRegion(CQGameBoyMemoryRegion::Type::HRAM  , 0xFF80, 0xFFFE, "High RAM");
  addRegion(CQGameBoyMemoryRegion::Type::IE    , 0xFFFF, 0xFFFF, "Interrupt Enable Register");
}

//------

CQGameBoyBIOSRegions::
CQGameBoyBIOSRegions(CQGameBoyMemoryMap *map) :
 CQGameBoyRegions(map)
{
  CQGameBoy *gameboy = map_->gameboy();

  if (gameboy->isGBC()) {
    addRegion(CQGameBoyMemoryRegion::Type::BIOS, 0x0000, 0x00FF, "BIOS");
    addRegion(CQGameBoyMemoryRegion::Type::BIOS, 0x0200, 0x08FF, "BIOS");

    setMaxAddr(0x08FF);
  }
  else {
    addRegion(CQGameBoyMemoryRegion::Type::BIOS, 0x0000, 0x00FF, "BIOS");

    setMaxAddr(0x00FF);
  }
}

//------

CQGameBoyCartridgeRegions::
CQGameBoyCartridgeRegions(CQGameBoyMemoryMap *map) :
 CQGameBoyRegions(map)
{
  CQGameBoy *gameboy = map_->gameboy();

  addRegion(CQGameBoyMemoryRegion::Type::CART, 0x0000, gameboy->cartridgeLen() - 1, "Cartridge");

  setMaxAddr(gameboy->cartridgeLen() - 1);
}

//------

CQGameBoyRegions::
CQGameBoyRegions(CQGameBoyMemoryMap *map) :
 map_(map)
{
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setMargin(2); layout->setSpacing(2);

  canvas_ = new CQGameBoyRegionsCanvas(this);

  layout->addWidget(canvas_);
}

CQGameBoyMemoryRegion *
CQGameBoyRegions::
addRegion(CQGameBoyMemoryRegion::Type type, uint start, uint end,
          const QString &name, int numBanks)
{
  CQGameBoyMemoryRegion *region =
    new CQGameBoyMemoryRegion(this, type, start, end, name, numBanks);

  region->setInd(regions_.size());

  regions_.push_back(region);

  return region;
}

void
CQGameBoyRegions::
setCurrentRegion(CQGameBoyMemoryRegion *region)
{
  if (region_ != region) {
    region_ = region;

    updateCurrentRegion();
  }
  else {
    map_->updateHexdump();
  }
}

void
CQGameBoyRegions::
updateCurrentRegion()
{
  map_->setHexdumpRegion(currentRegion());

  canvas_->update();
}

void
CQGameBoyRegions::
setBankNum(int num)
{
  if (currentRegion()) {
    currentRegion()->setBankNum(num);

    updateCurrentRegion();
  }
}

//------

CQGameBoyRegionsCanvas::
CQGameBoyRegionsCanvas(CQGameBoyRegions *regions) :
 regions_(regions)
{
  setObjectName("canvas");

  setFocusPolicy(Qt::StrongFocus);
}

void
CQGameBoyRegionsCanvas::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  QFontMetrics fm(font());

  double b  = 2;
  double mw = width() - 2*b;

  double y = 0;

  for (const auto &region : regions_->regions()) {
    bool isCurrent = (regions_->currentRegion() == region);

    double f1 = regions_->map(region->start());
    double f2 = regions_->map(region->end  ());

    double mh = (f2 - f1)*height();

    QRectF r(b, y, mw - 1, mh - 1);

    if (isCurrent)
      painter.setBrush(QColor(200,200,255));
    else
      painter.setBrush(QColor(200,200,200));

    painter.setPen(Qt::black);

    painter.drawRect(r);

    if (r.height() >= 3*fm.height()) {
      QString ts = QString::asprintf("0x%04x", region->start());
      QString te = QString::asprintf("0x%04x", region->end  ());

      painter.drawText(r.left() + b, r.top   () + b + fm.ascent (), ts);
      painter.drawText(r.left() + b, r.bottom() - b - fm.descent(), te);
    }

    if (r.height() >= fm.height()) {
      int dt = fm.height()/2 - fm.descent();

      QString text = region->name();

      if (region->numBanks() > 1) {
        text += QString("(%1 of %2)").arg(region->bankNum()).arg(region->numBanks() - 1);
      }

      painter.drawText(r.left() + b, r.center().y() + dt, text);
    }

    y += mh;
  }
}

void
CQGameBoyRegionsCanvas::
mousePressEvent(QMouseEvent *e)
{
  double b  = 2;
  double mw = width() - 2*b;

  double y = 0;

  for (const auto &region : regions_->regions()) {
    double f1 = regions_->map(region->start());
    double f2 = regions_->map(region->end  ());

    double mh = (f2 - f1)*height();

    QRectF r(b, y, mw, mh);

    if (r.contains(e->pos())) {
      regions_->setCurrentRegion(region);
    }

    y += mh;
  }
}

void
CQGameBoyRegionsCanvas::
keyPressEvent(QKeyEvent *e)
{
  // Next Region
  if      (e->key() == Qt::Key_Down) {
    bool isCurrent = false;

    for (const auto &region : regions_->regions()) {
      if (isCurrent) {
        regions_->setCurrentRegion(region);
        break;
      }

      isCurrent = (regions_->currentRegion() == region);
    }
  }
  // Prev Region
  else if (e->key() == Qt::Key_Up) {
    CQGameBoyMemoryRegion *prevRegion = { nullptr };

    for (const auto &region : regions_->regions()) {
      bool isCurrent = (regions_->currentRegion() == region);

      if (isCurrent) {
        if (prevRegion)
          regions_->setCurrentRegion(prevRegion);

        break;
      }

      prevRegion = region;
    }
  }
}

QSize
CQGameBoyRegionsCanvas::
sizeHint() const
{
  return QSize(250, 750);
}

//------

CQGameBoyMemoryRegion::
CQGameBoyMemoryRegion(CQGameBoyRegions *regions, Type type, uint start, uint end,
                      const QString &name, int numBanks) :
 regions_(regions), type_(type), start_(start), end_(end), name_(name), numBanks_(numBanks)
{
}

uchar
CQGameBoyMemoryRegion::
data(uint i) const
{
  CQGameBoy *gameboy = regions_->map()->gameboy();

  if      (type_ == Type::BIOS) {
    return gameboy->biosData(start() + i);
  }
  else if (type_ == Type::CART) {
    return gameboy->readCartridge(i);
  }
  else if (type_ == Type::VRAM) {
    if (gameboy->isGBC())
      return gameboy->getVRam(bankNum(), i);
    else {
      CZ80 *z80 = gameboy->getZ80();

      return z80->getByte(start() + i);
    }
  }
  else {
    CZ80 *z80 = gameboy->getZ80();

    return z80->getByte(start() + i);
  }
}
