#include <CQGameBoySprites.h>
#include <CQGameBoySpriteList.h>
#include <CQGameBoyVideo.h>
#include <CQGameBoyScreen.h>
#include <CQGameBoy.h>

#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>

CQGameBoySprites::
CQGameBoySprites(CQGameBoyVideo *video) :
 video_(video)
{
  setObjectName("sprites");

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  //---

  QHBoxLayout *controlLayout = new QHBoxLayout;
  controlLayout->setMargin(2); controlLayout->setSpacing(2);

  scaleSpin_ = new QSpinBox;

  scaleSpin_->setValue(getScale());
  scaleSpin_->setToolTip("Scale");

  connect(scaleSpin_, SIGNAL(valueChanged(int)), this, SLOT(scaleSlot()));

  controlLayout->addWidget(new QLabel("Scale"));
  controlLayout->addWidget(scaleSpin_);

  doubleHeightCheck_ = new QCheckBox("Double Height");

  connect(doubleHeightCheck_, SIGNAL(stateChanged(int)), this, SLOT(doubleHeightSlot()));

  controlLayout->addWidget(doubleHeightCheck_);

  controlLayout->addStretch(1);

  layout->addLayout(controlLayout);

  //---

  QHBoxLayout *canvasLayout = new QHBoxLayout;
  canvasLayout->setMargin(2); canvasLayout->setSpacing(2);

  canvas_ = new CQGameBoySpritesCanvas(this);

  canvasLayout->addWidget(canvas_);

  //---

  list_ = new CQGameBoySpriteList(video);

  list_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  canvasLayout->addWidget(list_);

  //---

  layout->addLayout(canvasLayout);
}

CQGameBoySprites::
~CQGameBoySprites()
{
}

void
CQGameBoySprites::
scaleSlot()
{
  setScale(scaleSpin_->value());

  canvas_->update();
}

void
CQGameBoySprites::
doubleHeightSlot()
{
  setDoubleHeight(doubleHeightCheck_->isChecked());

  canvas_->update();
}

void
CQGameBoySprites::
update()
{
  canvas_->update();

  list_->update();
}

//------

CQGameBoySpritesCanvas::
CQGameBoySpritesCanvas(CQGameBoySprites *sprites) :
 sprites_(sprites)
{
}

void
CQGameBoySpritesCanvas::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.fillRect(rect(), Qt::white);

  int scale = sprites_->getScale();

  drawSprites(&painter, 0, 0, scale);
}

void
CQGameBoySpritesCanvas::
drawSprites(QPainter *painter, int x, int y, int scale)
{
  CQGameBoyVideo *video = sprites_->video();

  CQGameBoy *gameboy = video->screen()->gameboy();

  CZ80 *z80 = gameboy->getZ80();

  uchar palette1 = z80->getByte(0xff48);
  uchar palette2 = z80->getByte(0xff49);

  int bank = 1; // bank always 1

  // 160 bytes: 40 sprites, 4 bytes each
  CGameBoySprite sprite;

  if (! sprites_->isDoubleHeight()) {
    for (int i = 0; i < 40; ++i) {
      gameboy->getSprite(i, sprite);

      int x1 = sprite.x - 8;
      int x2 = x1 + 8;

      if (x2 < 0 || x1 >= 144)
        continue;

      int y1 = sprite.y - 16;
      int y2 = y1 + 8;

      if (y2 < 0 || y1 >= 160)
        continue;

      uchar palette = (sprite.palNum1 == 0 ? palette1 : palette2);

      video->drawTile(painter, x + x1*scale, y + y1*scale, bank, sprite.t,
                      palette, sprite.xflip, sprite.yflip, scale);
    }
  }
  else {
    for (int i = 0; i < 40; ++i) {
      gameboy->getSprite(i, sprite);

      int x1 = sprite.x - 8;
      int x2 = x1 + 8;

      if (x2 < 0 || x1 >= 144)
        continue;

      int y1 = sprite.y - 16;
      int y2 = y1 + 16;

      if (y2 < 0 || y1 >= 160)
        continue;

      int t1 = sprite.t & 0xFE;
      int t2 = t1 + 1;

      uchar palette = (sprite.palNum1 == 0 ? palette1 : palette2);

      video->drawTile(painter, x + x1*scale, y + y1*scale, bank, t1,
                      palette, sprite.xflip, sprite.yflip, scale);

      video->drawTile(painter, x + x1*scale, y + (y1 + 8)*scale, bank, t2,
                      palette, sprite.xflip, sprite.yflip, scale);
    }
  }

  painter->drawRect(QRect(0, 0, 32*8*scale, 32*8*scale));
}

void
CQGameBoySpritesCanvas::
displaySprites()
{
  CGameBoySprite sprite;

  // 160 bytes: 40 sprites, 4 bytes each
  for (int i = 0; i < 40; ++i) {
    sprites_->video()->screen()->gameboy()->getSprite(i, sprite);

    sprite.print(std::cerr);

    std::cerr << std::endl;
  }
}

QSize
CQGameBoySpritesCanvas::
sizeHint() const
{
  int scale = sprites_->getScale();

  return QSize(32*8*scale, 32*8*scale);
}
