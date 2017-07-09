#include <CQGameBoyTiles.h>
#include <CQGameBoyVideo.h>
#include <CQGameBoyScreen.h>
#include <CQGameBoy.h>

#include <QGroupBox>
#include <QSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>

CQGameBoyTiles::
CQGameBoyTiles(CQGameBoyVideo *video) :
 video_(video)
{
  setObjectName("tiles");

  CQGameBoy *gameboy = video->screen()->gameboy();

  //---

  QVBoxLayout *vlayout = new QVBoxLayout(this);
  vlayout->setMargin(0); vlayout->setSpacing(0);

  //---

  QHBoxLayout *controlLayout = new QHBoxLayout;
  controlLayout->setMargin(2); controlLayout->setSpacing(2);

  //--

  scaleSpin_ = new QSpinBox;

  scaleSpin_->setValue(getScale());
  scaleSpin_->setRange(1, 16);
  scaleSpin_->setToolTip("Scale");

  connect(scaleSpin_, SIGNAL(valueChanged(int)), this, SLOT(scaleSlot()));

  controlLayout->addWidget(new QLabel("Scale"));
  controlLayout->addWidget(scaleSpin_);

  //--

  if (gameboy->isGBC()) {
    vbankSpin_ = new QSpinBox;

    vbankSpin_->setValue(0);
    vbankSpin_->setRange(0, 1);
    vbankSpin_->setToolTip("VRAM Bank");

    connect(vbankSpin_, SIGNAL(valueChanged(int)), this, SLOT(vbankSlot()));

    controlLayout->addWidget(new QLabel("Bank"));
    controlLayout->addWidget(vbankSpin_);
  }

  //--

  controlLayout->addStretch(1);

  vlayout->addLayout(controlLayout);

  //---

  QHBoxLayout *groupLayout = new QHBoxLayout;
  groupLayout->setMargin(2); groupLayout->setSpacing(2);

  QGroupBox *canvas1Group = new QGroupBox("Bank 1");
  QGroupBox *canvas2Group = new QGroupBox("Bank 2");

  canvas1Group->setObjectName("canvas1Group1");
  canvas2Group->setObjectName("canvas1Group2");

  QHBoxLayout *canvas1Layout = new QHBoxLayout(canvas1Group);
  canvas1Layout->setMargin(0); canvas1Layout->setSpacing(0);

  QHBoxLayout *canvas2Layout = new QHBoxLayout(canvas2Group);
  canvas2Layout->setMargin(0); canvas2Layout->setSpacing(0);

  canvas1_ = new CQGameBoyTilesCanvas(this, 0);
  canvas2_ = new CQGameBoyTilesCanvas(this, 1);

  canvas1_->setObjectName("canvas1");
  canvas2_->setObjectName("canvas2");

  canvas1Layout->addWidget(canvas1_);
  canvas2Layout->addWidget(canvas2_);

  groupLayout->addWidget(canvas1Group);
  groupLayout->addWidget(canvas2Group);

  vlayout->addLayout(groupLayout);
}

CQGameBoyTiles::
~CQGameBoyTiles()
{
}

void
CQGameBoyTiles::
scaleSlot()
{
  setScale(scaleSpin_->value());

  canvas1_->update();
  canvas2_->update();
}

void
CQGameBoyTiles::
vbankSlot()
{
  setVBank(vbankSpin_->value());

  canvas1_->update();
  canvas2_->update();
}

//------

CQGameBoyTilesCanvas::
CQGameBoyTilesCanvas(CQGameBoyTiles *tiles, int bank) :
 tiles_(tiles), bank_(bank)
{
  setFocusPolicy(Qt::StrongFocus);

  setFrameShape (QFrame::Panel);
  setFrameShadow(QFrame::Sunken);
}

void
CQGameBoyTilesCanvas::
paintEvent(QPaintEvent *)
{
  CQGameBoyVideo *video = tiles_->video();

  QPainter painter(this);

  painter.fillRect(rect(), Qt::white);

  int scale = tiles_->getScale();
  int vbank = tiles_->getVBank();

  int tw = 8*scale;
  int th = 8*scale;

  int rows = 16;
  int cols = 16;

  uchar palette = 0xe4;

  for (int i = 0; i < rows*cols; ++i) {
    int x = tw*(i % cols);
    int y = th*(i / cols);

    video->drawTile(&painter, x, y, vbank, bank_, i,
                    palette, false, false, /*isSprite*/false, scale);
  }
}

QSize
CQGameBoyTilesCanvas::
sizeHint() const
{
  int scale = tiles_->getScale();

  return QSize(32*8*scale, 32*8*scale);
}
