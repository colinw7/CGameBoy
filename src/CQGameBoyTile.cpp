#include <CQGameBoyTile.h>
#include <CQGameBoyVideo.h>
#include <CQGameBoyScreen.h>
#include <CQGameBoy.h>
#include <CQUtil.h>
#include <CQUtilEvent.h>

#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>

CQGameBoyTile::
CQGameBoyTile(CQGameBoyVideo *video) :
 video_(video)
{
  setObjectName("tile");

  CQGameBoy *gameboy = video->screen()->gameboy();

  //---

  QVBoxLayout *vlayout = new QVBoxLayout(this);
  vlayout->setMargin(0); vlayout->setSpacing(0);

  //---

  QHBoxLayout *controlLayout = new QHBoxLayout;
  controlLayout->setMargin(2); controlLayout->setSpacing(2);

  bankSpin_ = new QSpinBox;

  bankSpin_->setObjectName("bank");
  bankSpin_->setRange(0, 1);
  bankSpin_->setValue(getBank());
  bankSpin_->setToolTip("Bank");

  connect(bankSpin_, SIGNAL(valueChanged(int)), this, SLOT(bankSlot()));

  //---

  vbankSpin_ = new QSpinBox;

  vbankSpin_->setObjectName("vbank");
  vbankSpin_->setRange(0, 1);
  vbankSpin_->setValue(getVBank());
  vbankSpin_->setToolTip("VRAM Bank");

  connect(vbankSpin_, SIGNAL(valueChanged(int)), this, SLOT(vbankSlot()));

  //---

  tileSpin_ = new QSpinBox;

  tileSpin_->setObjectName("tile");
  tileSpin_->setValue(getTile());
  tileSpin_->setToolTip("Tile");

  connect(tileSpin_, SIGNAL(valueChanged(int)), this, SLOT(tileSlot()));

  //---

  if (gameboy->isGBC()) {
    paletteSpin_ = new QSpinBox;

    paletteSpin_->setObjectName("palette");
    paletteSpin_->setValue(getPalette());
    paletteSpin_->setRange(0, 7);
    paletteSpin_->setToolTip("Palette");

    connect(paletteSpin_, SIGNAL(valueChanged(int)), this, SLOT(paletteSlot()));
  }

  //---

  xFlipCheck_ = new QCheckBox("X Flip");

  xFlipCheck_->setObjectName("xflip");
  xFlipCheck_->setChecked(getXFlip());

  connect(xFlipCheck_, SIGNAL(stateChanged(int)), this, SLOT(xflipSlot()));

  yFlipCheck_ = new QCheckBox("Y Flip");

  yFlipCheck_->setObjectName("yflip");
  yFlipCheck_->setChecked(getXFlip());

  connect(yFlipCheck_, SIGNAL(stateChanged(int)), this, SLOT(yflipSlot()));

  //---

  scaleSpin_ = new QSpinBox;

  scaleSpin_->setObjectName("scale");
  scaleSpin_->setValue(getScale());
  scaleSpin_->setToolTip("Scale");

  connect(scaleSpin_, SIGNAL(valueChanged(int)), this, SLOT(scaleSlot()));

  //---

  controlLayout->addWidget (new QLabel("Bank"));
  controlLayout->addWidget (bankSpin_);
  controlLayout->addWidget (new QLabel("VBank"));
  controlLayout->addWidget (vbankSpin_);
  controlLayout->addWidget (new QLabel("Tile"));
  controlLayout->addWidget (tileSpin_);

  if (gameboy->isGBC()) {
    controlLayout->addWidget(new QLabel("Palette"));
    controlLayout->addWidget(paletteSpin_);
  }

  controlLayout->addWidget(xFlipCheck_);
  controlLayout->addWidget(yFlipCheck_);

  controlLayout->addWidget (new QLabel("Scale"));
  controlLayout->addWidget (scaleSpin_);
  controlLayout->addStretch(1);

  vlayout->addLayout(controlLayout);

  //---

  QHBoxLayout *canvasLayout = new QHBoxLayout;
  canvasLayout->setMargin(2); canvasLayout->setSpacing(2);

  canvas_ = new CQGameBoyTileCanvas(this);

  canvasLayout->addWidget(canvas_);

  vlayout->addLayout(canvasLayout);
}

CQGameBoyTile::
~CQGameBoyTile()
{
}

void
CQGameBoyTile::
bankSlot()
{
  setBank(bankSpin_->value());

  canvas_->update();
}

void
CQGameBoyTile::
vbankSlot()
{
  setVBank(vbankSpin_->value());

  canvas_->update();
}

void
CQGameBoyTile::
tileSlot()
{
  setTile(tileSpin_->value());

  canvas_->update();
}

void
CQGameBoyTile::
paletteSlot()
{
  setPalette(paletteSpin_->value());

  canvas_->update();
}

void
CQGameBoyTile::
xflipSlot()
{
  setXFlip(xFlipCheck_->isChecked());

  canvas_->update();
}

void
CQGameBoyTile::
yflipSlot()
{
  setYFlip(yFlipCheck_->isChecked());

  canvas_->update();
}

void
CQGameBoyTile::
scaleSlot()
{
  setScale(scaleSpin_->value());

  canvas_->update();
}

void
CQGameBoyTile::
update()
{
  bankSpin_ ->setValue(getBank ());
  vbankSpin_->setValue(getVBank());
  tileSpin_ ->setValue(getTile ());
  scaleSpin_->setValue(getScale());

  canvas_->update();
}

//------

CQGameBoyTileCanvas::
CQGameBoyTileCanvas(CQGameBoyTile *tile) :
 tile_(tile)
{
  setFocusPolicy(Qt::StrongFocus);
}

CQGameBoyTileCanvas::
~CQGameBoyTileCanvas()
{
}

void
CQGameBoyTileCanvas::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.fillRect(rect(), Qt::white);

  CQGameBoyVideo *video = tile_->video();

  CQGameBoy *gameboy = video->screen()->gameboy();

  int  bank  = tile_->getBank();
  int  vbank = tile_->getVBank();
  int  tile  = tile_->getTile();
  int  scale = tile_->getScale();
  bool xflip = tile_->getXFlip();
  bool yflip = tile_->getYFlip();

  uchar palette;

  if (gameboy->isGBC()) {
    palette = tile_->getPalette();
  }
  else {
    palette = 0xe4; // standard, 0, 1, 2, 3 (R->L) (11100100)
  }

  video->drawTile(&painter, 2, 2, vbank, bank, tile, palette, xflip, yflip,
                  /*isSprite*/false, scale);

  QSize s = sizeHint();

  painter.drawRect(QRect(0, 0, s.width(), s.height()));
}

void
CQGameBoyTileCanvas::
keyPressEvent(QKeyEvent *e)
{
  CKeyEvent *kevent = CQUtil::convertEvent(e);

  CKeyType type = kevent->getType();

  if      (type == CKEY_TYPE_Left) {
    if (tile_->getTile() > 0)
      tile_->setTile(tile_->getTile() - 1);
  }
  else if (type == CKEY_TYPE_Right) {
    if (tile_->getTile() < 255)
      tile_->setTile(tile_->getTile() + 1);
  }
  else if (type == CKEY_TYPE_Up) {
    if (tile_->getBank() > 0)
      tile_->setBank(tile_->getBank() - 1);
  }
  else if (type == CKEY_TYPE_Down) {
    if (tile_->getBank() < 1)
      tile_->setBank(tile_->getBank() + 1);
  }

  update();
}

QSize
CQGameBoyTileCanvas::
sizeHint() const
{
  int scale = tile_->getScale();

  return QSize(8*scale + 4, 8*scale + 4);
}
