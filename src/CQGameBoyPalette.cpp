#include <CQGameBoyPalette.h>
#include <CQGameBoySpriteList.h>
#include <CQGameBoyVideo.h>
#include <CQGameBoyScreen.h>
#include <CQGameBoy.h>

#include <QColorDialog>
#include <QSpinBox>
#include <QToolButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QPainter>

CQGameBoyPalette::
CQGameBoyPalette(CQGameBoyVideo *video) :
 video_(video)
{
  setObjectName("palette");

  CQGameBoy *gameboy = video->screen()->gameboy();

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  //---

  QVBoxLayout *paletteLayout = new QVBoxLayout;
  paletteLayout->setMargin(0); paletteLayout->setSpacing(0);

  if (gameboy->isGBC()) {
    for (int i = 0; i < 8; ++i) {
      CQGameBoyColorPalette *palette = new CQGameBoyColorPalette(this, i);

      connect(palette, SIGNAL(colorPressed(int, int)), this, SLOT(colorPalettePressed(int, int)));

      palettes_.push_back(palette);
    }

    for (auto &palette : palettes_)
      paletteLayout->addWidget(palette);
  }
  else {
    sets_.push_back(new CQGameBoyPaletteSet(this, "BGP" , 0xff47));
    sets_.push_back(new CQGameBoyPaletteSet(this, "OBP0", 0xff48));
    sets_.push_back(new CQGameBoyPaletteSet(this, "OBP1", 0xff49));

    for (auto &set : sets_) {
      paletteLayout->addWidget(set);

      connect(set, SIGNAL(colorPressed(int, int)), this, SLOT(colorSetPressed(int, int)));
    }
  }

  layout->addLayout(paletteLayout);

  //---

  QHBoxLayout *colorLayout = new QHBoxLayout;

  layout->addLayout(colorLayout);

  colorLabel_  = new QLabel;
  colorButton_ = new QToolButton;

  colorButton_->setAutoFillBackground(true);

  connect(colorButton_, SIGNAL(clicked()), this, SLOT(colorButtonSlot()));

  colorLayout->addWidget(colorLabel_);
  colorLayout->addWidget(colorButton_);
  colorLayout->addStretch(1);

  //---

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

void
CQGameBoyPalette::
colorPalettePressed(int color, int palette)
{
  palette_ = palette;
  addr_    = 0;
  color_   = color;

  CQGameBoy *gameboy = video_->screen()->gameboy();

  QColor c = gameboy->vramBgPaletteColor(palette_, color_);

  colorLabel_->setText(QString("%1:%2").arg(palette_).arg(color_));

  setButtonColor(c);
}

void
CQGameBoyPalette::
colorSetPressed(int color, int addr)
{
  palette_ = 0;
  addr_    = addr;
  color_   = color;

  CQGameBoy *gameboy = video_->screen()->gameboy();

  CZ80 *z80 = gameboy->getZ80();

  const QColor &c = gameboy->mappedPaletteColor(z80->getByte(addr_), color_);

  colorLabel_->setText(QString("%1:%2").arg(CZ80::hexString((ushort) addr_).c_str()).arg(color));

  setButtonColor(c);
}

void
CQGameBoyPalette::
colorButtonSlot()
{
  CQGameBoy *gameboy = video_->screen()->gameboy();

  if (gameboy->isGBC()) {
    QColor c = gameboy->vramBgPaletteColor(palette_, color_);

    QColor c1 = QColorDialog::getColor(c, this);

    if (c1.isValid()) {
      gameboy->setVRamBgPaletteColor(palette_, color_, c1);

      setButtonColor(c1);
    }
  }
  else {
    CZ80 *z80 = gameboy->getZ80();

    const QColor &c = gameboy->mappedPaletteColor(z80->getByte(addr_), color_);

    QColor c1 = QColorDialog::getColor(c, this);

    if (c1.isValid()) {
      gameboy->setMappedPaletteColor(z80->getByte(addr_), color_, c1);

      setButtonColor(c1);
    }
  }

  update();
}

void
CQGameBoyPalette::
setButtonColor(const QColor &c)
{
  QPalette palette = colorButton_->palette();

  palette.setColor(colorButton_->backgroundRole(), c);

  colorButton_->setPalette(palette);
}

//------

CQGameBoyColorPalette::
CQGameBoyColorPalette(CQGameBoyPalette *palette, uchar ind) :
 palette_(palette), ind_(ind)
{
}

void
CQGameBoyColorPalette::
mousePressEvent(QMouseEvent *e)
{
  int n = e->x()/size_;

  emit colorPressed(n, ind_);
}

void
CQGameBoyColorPalette::
paintEvent(QPaintEvent *)
{
  CQGameBoy *gameboy = palette_->video()->screen()->gameboy();

  QPainter painter(this);

  painter.fillRect(rect(), bg_);

  uchar r, g, b;

  gameboy->bgPaletteColor(ind_, 0, r, g, b); QColor c1 = QColor(r, g, b);
  gameboy->bgPaletteColor(ind_, 1, r, g, b); QColor c2 = QColor(r, g, b);
  gameboy->bgPaletteColor(ind_, 2, r, g, b); QColor c3 = QColor(r, g, b);
  gameboy->bgPaletteColor(ind_, 3, r, g, b); QColor c4 = QColor(r, g, b);

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
mousePressEvent(QMouseEvent *e)
{
  QFontMetrics fm(font());

  int tw = fm.width("XXXXX");

  if (e->x() >= tw && e->x() <= int(tw + 4*size_)) {
    int n = (e->x() - tw)/size_;

    emit colorPressed(n, addr_);
  }
}

void
CQGameBoyPaletteSet::
paintEvent(QPaintEvent *)
{
  QFontMetrics fm(font());

  int tw = fm.width("XXXXX");

  CQGameBoy *gameboy = palette_->video()->screen()->gameboy();

  CZ80 *z80 = gameboy->getZ80();

  QPainter painter(this);

  painter.fillRect(rect(), bg_);

  uchar data = z80->getByte(addr_);

  const QColor &c1 = gameboy->mappedPaletteColor(data, 0);
  const QColor &c2 = gameboy->mappedPaletteColor(data, 1);
  const QColor &c3 = gameboy->mappedPaletteColor(data, 2);
  const QColor &c4 = gameboy->mappedPaletteColor(data, 3);

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
