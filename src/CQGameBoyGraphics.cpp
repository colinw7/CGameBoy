#include <CQGameBoyGraphics.h>
#include <CQGameBoyVideo.h>
#include <CQGameBoyScreen.h>
#include <CQGameBoy.h>
#include <QGroupBox>
#include <QSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QMouseEvent>

CQGameBoyGraphics::
CQGameBoyGraphics(CQGameBoyVideo *video) :
 video_(video)
{
  setObjectName("graphics");

  QVBoxLayout *vlayout = new QVBoxLayout(this);
  vlayout->setMargin(0); vlayout->setSpacing(0);

  //---

  QHBoxLayout *controlLayout = new QHBoxLayout;
  controlLayout->setMargin(2); controlLayout->setSpacing(2);

  scaleSpin_ = new QSpinBox;

  scaleSpin_->setValue(getScale());
  scaleSpin_->setToolTip("Scale");
  scaleSpin_->setRange(1, 16);

  connect(scaleSpin_, SIGNAL(valueChanged(int)), this, SLOT(scaleSlot()));

  controlLayout->addWidget(new QLabel("Scale"));
  controlLayout->addWidget(scaleSpin_);

  tileDataSpin_ = new QSpinBox;

  tileDataSpin_->setValue(getTileData());
  tileDataSpin_->setToolTip("Scale");
  tileDataSpin_->setRange(0, 1);

  connect(tileDataSpin_, SIGNAL(valueChanged(int)), this, SLOT(tileDataSlot()));

  controlLayout->addWidget(new QLabel("Tile Data"));
  controlLayout->addWidget(tileDataSpin_);

  controlLayout->addStretch(1);

  vlayout->addLayout(controlLayout);

  //---

  QHBoxLayout *groupLayout = new QHBoxLayout;
  groupLayout->setMargin(2); groupLayout->setSpacing(2);

  QGroupBox *canvas1Group = new QGroupBox("Tile Map 1");
  QGroupBox *canvas2Group = new QGroupBox("Tile Map 2");

  canvas1Group->setObjectName("canvas1Group1");
  canvas2Group->setObjectName("canvas1Group2");

  QHBoxLayout *canvas1Layout = new QHBoxLayout(canvas1Group);
  canvas1Layout->setMargin(2); canvas1Layout->setSpacing(2);

  QHBoxLayout *canvas2Layout = new QHBoxLayout(canvas2Group);
  canvas2Layout->setMargin(2); canvas2Layout->setSpacing(2);

  canvas1_ = new CQGameBoyGraphicsCanvas(this, 0);
  canvas2_ = new CQGameBoyGraphicsCanvas(this, 1);

  canvas1_->setObjectName("canvas1");
  canvas2_->setObjectName("canvas2");

  connect(canvas1_, SIGNAL(tileClicked(int, int, int)),
          this, SLOT(tileClickSlot(int, int, int)));
  connect(canvas2_, SIGNAL(tileClicked(int, int, int)),
          this, SLOT(tileClickSlot(int, int, int)));

  canvas1Layout->addWidget(canvas1_);
  canvas2Layout->addWidget(canvas2_);

  groupLayout->addWidget(canvas1Group);
  groupLayout->addWidget(canvas2Group);

  vlayout->addLayout(groupLayout);

  //---

  QFrame *infoFrame = new QFrame;

  infoFrame->setObjectName("info");

  infoLabel_ = new QLabel;

  QHBoxLayout *infoLayout = new QHBoxLayout(infoFrame);

  infoLayout->addWidget(infoLabel_);

  vlayout->addWidget(infoFrame);
}

CQGameBoyGraphics::
~CQGameBoyGraphics()
{
}

void
CQGameBoyGraphics::
scaleSlot()
{
  setScale(scaleSpin_->value());

  canvas1_->update();
  canvas2_->update();
}

void
CQGameBoyGraphics::
tileDataSlot()
{
  setTileData(tileDataSpin_->value());

  canvas1_->update();
  canvas2_->update();
}

void
CQGameBoyGraphics::
tileClickSlot(int tileMap, int tx, int ty)
{
  CQGameBoy *gameboy = video_->screen()->gameboy();

  int tile     = gameboy->getTileNum(tileMap, tx, ty);
//int tileData = tileDataSpin_->value();

  QString text;

  if (gameboy->isGBC()) {
    CGameBoyTileAttr attr;

    gameboy->getTileAttr(tileMap, tx, ty, attr);

    text = QString("(%1 %2) : Tile %3, Palette %4, VBank %5, XFlip %6, YFlip %7").
                   arg(tx).arg(ty).arg(tile).arg(attr.pnum).arg(attr.bank).
                   arg(attr.hflip).arg(attr.vflip);
  }
  else {
    text = QString("(%1 %2) : Tile %3").
                   arg(tx).arg(ty).arg(tile);
  }

  infoLabel_->setText(text);
}

//------

CQGameBoyGraphicsCanvas::
CQGameBoyGraphicsCanvas(CQGameBoyGraphics *graphics, int tileMap) :
 graphics_(graphics), tileMap_(tileMap)
{
  setFocusPolicy(Qt::StrongFocus);

  setFrameShape (QFrame::Panel);
  setFrameShadow(QFrame::Sunken);
}

void
CQGameBoyGraphicsCanvas::
mousePressEvent(QMouseEvent *e)
{
  int scale = graphics_->getScale();

  tileX_ = std::min(std::max(e->x()/(scale*8), 0), 31);
  tileY_ = std::min(std::max(e->y()/(scale*8), 0), 31);

  emit tileClicked(tileMap_, tileX_, tileY_);
}

void
CQGameBoyGraphicsCanvas::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.fillRect(rect(), Qt::white);

  // 384 tiles (8 rows of 2 bytes)

  int scale    = graphics_->getScale();
  int tileMap  = getTileMap();
  int tileData = graphics_->getTileData();

  // draw screen tiles
  graphics_->video()->drawScreen(&painter, 0, 0, tileMap, tileData, scale);
}

QSize
CQGameBoyGraphicsCanvas::
sizeHint() const
{
  int scale = graphics_->getScale();

  int rows = 32;
  int cols = 32;

  return QSize(rows*8*scale, cols*8*scale);
}
