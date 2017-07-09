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

  CQGameBoy *gameboy = video->screen()->gameboy();

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  if (gameboy->isGBC()) {
    for (int i = 0; i < 8; ++i) {
      palettes_.push_back(new CQGameBoyColorPalette(this, i));
    }

    for (auto &palette : palettes_)
      layout->addWidget(palette);
  }
  else {
    sets_.push_back(new CQGameBoyPaletteSet(this, "BGP" , 0xff47));
    sets_.push_back(new CQGameBoyPaletteSet(this, "OBP0", 0xff48));
    sets_.push_back(new CQGameBoyPaletteSet(this, "OBP1", 0xff49));

    for (auto &set : sets_)
      layout->addWidget(set);
  }

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
  CQGameBoy *gameboy = video_->screen()->gameboy();

  if (gameboy->isGBC()) {
    for (auto &palette : palettes_)
      palette->update();
  }
  else {
    for (auto &set : sets_)
      set->update();
  }
}

//------

CQGameBoyColorPalette::
CQGameBoyColorPalette(CQGameBoyPalette *palette, uchar ind) :
 palette_(palette), ind_(ind)
{
}

void
CQGameBoyColorPalette::
paintEvent(QPaintEvent *)
{
  CQGameBoy *gameboy = palette_->video()->screen()->gameboy();

  QPainter painter(this);

  painter.fillRect(rect(), bg_);

  uchar r, g, b;

  gameboy->bgPaletteColor(ind_, 0, r, b, g); QColor c1 = QColor(r, g, b);
  gameboy->bgPaletteColor(ind_, 1, r, b, g); QColor c2 = QColor(r, g, b);
  gameboy->bgPaletteColor(ind_, 2, r, b, g); QColor c3 = QColor(r, g, b);
  gameboy->bgPaletteColor(ind_, 3, r, b, g); QColor c4 = QColor(r, g, b);

  int x = 0;
  int y = 0;

  painter.fillRect(QRect(x, y, size_, size_), c1); x += size_;
  painter.fillRect(QRect(x, y, size_, size_), c2); x += size_;
  painter.fillRect(QRect(x, y, size_, size_), c3); x += size_;
  painter.fillRect(QRect(x, y, size_, size_), c4);
}

QSize
CQGameBoyColorPalette::
sizeHint() const
{
  return QSize(size_*4, size_);
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
