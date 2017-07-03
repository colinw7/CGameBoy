#include <CQGameBoyPalette.h>
#include <CQGameBoySpriteList.h>
#include <CQGameBoyVideo.h>
#include <CQGameBoyScreen.h>
#include <CQGameBoy.h>

#include <QLabel>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>

CQGameBoyPalette::
CQGameBoyPalette(CQGameBoyVideo *video) :
 video_(video)
{
  setObjectName("palette");

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  set1_ = new CQGameBoyPaletteSet(this, "BGP" , 0xff47);
  set2_ = new CQGameBoyPaletteSet(this, "OBP0", 0xff48);
  set3_ = new CQGameBoyPaletteSet(this, "OBP1", 0xff49);

  layout->addWidget(set1_);
  layout->addWidget(set2_);
  layout->addWidget(set3_);

  layout->addStretch(1);
}

CQGameBoyPalette::
~CQGameBoyPalette()
{
}

void
CQGameBoyPalette::
update()
{
  set1_->update();
  set2_->update();
  set3_->update();
}

//------

CQGameBoyPaletteSet::
CQGameBoyPaletteSet(CQGameBoyPalette *palette, const QString &name, ushort addr) :
 palette_(palette), name_(name), addr_(addr)
{
}

void
CQGameBoyPaletteSet::
paintEvent(QPaintEvent *)
{
  QFontMetrics fm(font());

  int tw = fm.width("XXXXX");

  CQGameBoy *gameboy = palette_->video()->screen()->gameboy();

  CZ80 *z80 = gameboy->getZ80();

  uchar data = z80->getByte(addr_);

  uchar data00 = (data & 0x03);
  uchar data01 = (data & 0x0c) >> 2;
  uchar data10 = (data & 0x30) >> 4;
  uchar data11 = (data & 0xc0) >> 6;

  QPainter painter(this);

  painter.fillRect(rect(), bg_);

  const QColor &c1 = gameboy->paletteColor(data00);
  const QColor &c2 = gameboy->paletteColor(data01);
  const QColor &c3 = gameboy->paletteColor(data10);
  const QColor &c4 = gameboy->paletteColor(data11);

  int x = 0;
  int y = 0;

  painter.drawText(x, fm.ascent(), name_);

  x += tw;

  painter.fillRect(QRect(x, y, size_, size_), c1); x += size_;
  painter.fillRect(QRect(x, y, size_, size_), c2); x += size_;
  painter.fillRect(QRect(x, y, size_, size_), c3); x += size_;
  painter.fillRect(QRect(x, y, size_, size_), c4);
}

QSize
CQGameBoyPaletteSet::
sizeHint() const
{
  QFontMetrics fm(font());

  int tw = fm.width("XXXX");

  return QSize(tw + size_*4, size_);
}
