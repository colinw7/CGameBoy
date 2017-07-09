#include <CQGameBoyGraphics.h>
#include <CQGameBoyVideo.h>
#include <CQGameBoyScreen.h>
#include <CQGameBoy.h>
#include <QSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>

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

  bankSpin_ = new QSpinBox;

  bankSpin_->setValue(0);
  bankSpin_->setToolTip("Scale");
  bankSpin_->setRange(0, 1);

  connect(bankSpin_, SIGNAL(valueChanged(int)), this, SLOT(bankSlot()));

  controlLayout->addWidget(new QLabel("Bank"));
  controlLayout->addWidget(bankSpin_);

  controlLayout->addStretch(1);

  vlayout->addLayout(controlLayout);

  //---

  QHBoxLayout *canvasLayout = new QHBoxLayout;
  canvasLayout->setMargin(2); canvasLayout->setSpacing(2);

  canvas1_ = new CQGameBoyGraphicsCanvas(this, 0);
  canvas2_ = new CQGameBoyGraphicsCanvas(this, 1);

  canvas1_->setObjectName("canvas1");
  canvas2_->setObjectName("canvas2");

  canvasLayout->addWidget(canvas1_);
  canvasLayout->addWidget(canvas2_);

  vlayout->addLayout(canvasLayout);
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
bankSlot()
{
  canvas1_->setBank(bankSpin_->value());
  canvas2_->setBank(bankSpin_->value());

  canvas1_->update();
  canvas2_->update();
}

//------

CQGameBoyGraphicsCanvas::
CQGameBoyGraphicsCanvas(CQGameBoyGraphics *graphics, int screen) :
 graphics_(graphics), screen_(screen)
{
  setFocusPolicy(Qt::StrongFocus);

  setFrameShape (QFrame::Panel);
  setFrameShadow(QFrame::Sunken);
}

void
CQGameBoyGraphicsCanvas::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.fillRect(rect(), Qt::white);

  // 384 tiles (8 rows of 2 bytes)

  int scale  = graphics_->getScale();
  int screen = getScreen();
  int bank   = getBank();

  // draw screen tiles
  graphics_->video()->drawScreen(&painter, 2, 2, screen, bank, scale);

  QSize s = sizeHint();

  painter.drawRect(QRect(0, 0, s.width(), s.height()));
}

QSize
CQGameBoyGraphicsCanvas::
sizeHint() const
{
  int scale = graphics_->getScale();

  int rows = 32;
  int cols = 32;

  return QSize(rows*8*scale + 4, cols*8*scale + 4);
}
