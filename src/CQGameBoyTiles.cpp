#include <CQGameBoyTiles.h>
#include <CQGameBoyVideo.h>
#include <CQGameBoy.h>

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

  QVBoxLayout *vlayout = new QVBoxLayout(this);
  vlayout->setMargin(0); vlayout->setSpacing(0);

  //---

  QHBoxLayout *controlLayout = new QHBoxLayout;
  controlLayout->setMargin(2); controlLayout->setSpacing(2);

  scaleSpin_ = new QSpinBox;

  scaleSpin_->setValue(getScale());
  scaleSpin_->setToolTip("Scale");

  connect(scaleSpin_, SIGNAL(valueChanged(int)), this, SLOT(scaleSlot()));

  controlLayout->addWidget (new QLabel("Scale"));
  controlLayout->addWidget (scaleSpin_);
  controlLayout->addStretch(1);

  vlayout->addLayout(controlLayout);

  //---

  QHBoxLayout *canvasLayout = new QHBoxLayout;
  canvasLayout->setMargin(2); canvasLayout->setSpacing(2);

  canvas1_ = new CQGameBoyTilesCanvas(this, 0);
  canvas2_ = new CQGameBoyTilesCanvas(this, 1);

  canvas1_->setObjectName("canvas1");
  canvas2_->setObjectName("canvas2");

  canvasLayout->addWidget(canvas1_);
  canvasLayout->addWidget(canvas2_);

  vlayout->addLayout(canvasLayout);
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
  QPainter painter(this);

  painter.fillRect(rect(), Qt::white);

  int scale = tiles_->getScale();

  int tw = 8*scale;
  int th = 8*scale;

  int rows = 16;
  int cols = 16;

  uchar palette = 0xe4;

  for (int i = 0; i < rows*cols; ++i) {
    int x = tw*(i % cols);
    int y = th*(i / cols);

    tiles_->video()->drawTile(&painter, x + 2, y + 2, bank_, i,
                              palette, false, false, scale);
  }

  QSize s = sizeHint();

  painter.drawRect(QRect(0, 0, s.width(), s.height()));
}

QSize
CQGameBoyTilesCanvas::
sizeHint() const
{
  int scale = tiles_->getScale();

  return QSize(32*8*scale + 4, 32*8*scale + 4);
}
