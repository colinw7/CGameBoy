#include <CQGameBoyGraphics.h>
#include <CQGameBoy.h>
#include <CQUtil.h>
#include <QFrame>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QItemDelegate>
#include <QPainter>

#include <iostream>

CQGameBoyVideo::
CQGameBoyVideo(CQGameBoy *gameboy) :
 CZ80Trace(*gameboy->gameboy()->getZ80()), gameboy_(gameboy)
{
  setWindowTitle("GameBoy Video");

  setObjectName("video");

  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  //---

  graphics_ = new CQGameBoyGraphics(gameboy);

  layout->addWidget(graphics_);

  //---

  QFrame *controlFrame = new QFrame;

  layout->addWidget(controlFrame);

  QVBoxLayout *controlLayout = new QVBoxLayout(controlFrame);

  //---

  scaleSpin_ = new QSpinBox;

  scaleSpin_->setValue(graphics_->getScale());

  connect(scaleSpin_, SIGNAL(valueChanged(int)), this, SLOT(scaleSlot()));

  controlLayout->addWidget(scaleSpin_);

  //---

  QFrame *registersFrame = new QFrame;

  controlLayout->addWidget(registersFrame);

  QVBoxLayout *registersLayout = new QVBoxLayout(registersFrame);

  registersLayout->addWidget(addRegisterWidget("LCDC", 0xff40));
  registersLayout->addWidget(addRegisterWidget("STAT", 0xff41));
  registersLayout->addWidget(addRegisterWidget("SCY" , 0xff42));
  registersLayout->addWidget(addRegisterWidget("SCX" , 0xff43));
  registersLayout->addWidget(addRegisterWidget("LY"  , 0xff44));
  registersLayout->addWidget(addRegisterWidget("LYC" , 0xff45));
  registersLayout->addWidget(addRegisterWidget("BGP" , 0xff47));
  registersLayout->addWidget(addRegisterWidget("OBP0", 0xff48));
  registersLayout->addWidget(addRegisterWidget("OBP1", 0xff49));
  registersLayout->addWidget(addRegisterWidget("WY"  , 0xff4a));
  registersLayout->addWidget(addRegisterWidget("WX"  , 0xff4b));

  //---

  traceCheck_ = new QCheckBox("Trace");

  traceCheck_->setChecked(isTrace());

  connect(traceCheck_, SIGNAL(stateChanged(int)), this, SLOT(traceSlot()));

  controlLayout->addWidget(traceCheck_);

  //---

  QPushButton *redrawButton = new QPushButton("Redraw");

  connect(redrawButton, SIGNAL(clicked()), this, SLOT(redrawSlot()));

  controlLayout->addWidget(redrawButton);

  //---

  CQGameBoySpriteList *sprites = new CQGameBoySpriteList(this);

  controlLayout->addWidget(sprites);

  //---

  controlLayout->addStretch(1);

  //---

  gameboy->gameboy()->getZ80()->addTrace(this);
}

CQGameBoyVideo::
~CQGameBoyVideo()
{
}

void
CQGameBoyVideo::
setFixedFont(const QFont &font)
{
  for (const auto &ne : nameEdits_) {
    ne.second->setFont(font);
  }
}

void
CQGameBoyVideo::
scaleSlot()
{
  graphics_->setScale(scaleSpin_->value());

  graphics_->update();
}

void
CQGameBoyVideo::
traceSlot()
{
  setTrace(traceCheck_->isChecked());
}

void
CQGameBoyVideo::
redrawSlot()
{
  gameboy_->drawScreen();
}

int
CQGameBoyVideo::
getScale() const
{
  return graphics_->getScale();
}

void
CQGameBoyVideo::
setScale(int scale)
{
  graphics_->setScale(scale);
}

QFrame *
CQGameBoyVideo::
addRegisterWidget(const QString &name, ushort addr)
{
  QFrame *frame = new QFrame;

  QHBoxLayout *layout = new QHBoxLayout(frame);
  layout->setMargin(0); layout->setSpacing(0);

  QLabel *label = new QLabel(name);

  layout->addWidget(label);

  CQGameBoyVideoRegEdit *edit = new CQGameBoyVideoRegEdit(this, name, addr);

  edit->setText ("00" );
  edit->setLabel(label);

  layout->addWidget(edit);

  nameEdits_[name] = edit;
  addrEdits_[addr] = edit;

  return frame;
}

void
CQGameBoyVideo::
memChanged(ushort pos, ushort len)
{
  if (! isTrace())
    return;

  for (int i = 0; i < len; ++i) {
    auto p = addrEdits_.find(pos + i);

    if (p == addrEdits_.end())
      continue;

    (*p).second->update();
  }

  if (pos <= 0x9fff && pos + len - 1 >= 0x8000)
    update();
}

//------

CQGameBoyVideoRegEdit::
CQGameBoyVideoRegEdit(CQGameBoyVideo *video, const QString &name, ushort addr) :
 video_(video), name_(name), addr_(addr)
{
  setObjectName(name);

  connect(this, SIGNAL(editingFinished()), this, SLOT(valueSlot()));
}

void
CQGameBoyVideoRegEdit::
setFont(const QFont &font)
{
  QFontMetrics fm(font);

  int w = fm.width("XXXX");

  label_->setFixedWidth(w);

  label_->setFont(font);
}

void
CQGameBoyVideoRegEdit::
update()
{
  CZ80 *z80 = video_->gameboy()->gameboy()->getZ80();

  std::string text = CStrUtil::toHexString(z80->getMemory(addr_), 2);

  setText(text.c_str());
}

void
CQGameBoyVideoRegEdit::
setValue(int value)
{
  int value1 = std::min(std::max(value, 0), 255);

  std::string text = CStrUtil::toHexString(value1, 2);

  QLineEdit::setText(text.c_str());
}

int
CQGameBoyVideoRegEdit::
value() const
{
  uint value;

  if (! CStrUtil::decodeHexString(text().toStdString(), &value))
    return 0;

  return std::min(int(value), 0xff);
}

void
CQGameBoyVideoRegEdit::
valueSlot()
{
  int value = this->value();

  CZ80 *z80 = video_->gameboy()->gameboy()->getZ80();

  z80->setByte(addr_, value);

  emit valueChanged(value);
}

//------

CQGameBoyGraphics::
CQGameBoyGraphics(CQGameBoy *gameboy) :
 gameboy_(gameboy)
{
  setObjectName("graphics");

  setFocusPolicy(Qt::StrongFocus);
}

CQGameBoyGraphics::
~CQGameBoyGraphics()
{
}

void
CQGameBoyGraphics::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.fillRect(rect(), Qt::white);

  // 384 tiles (8 rows of 2 bytes)

  int scale = getScale();

  int rows = 16;
  int cols = 16;

  for (int bank = 0; bank < 2; ++bank) {
    for (int i = 0; i < 256; ++i) {
      int x = 8*scale*(i % cols);
      int y = 8*scale*(i / cols) + bank*(rows + 2)*8*scale;

      drawTile(&painter, x, y, bank, i, false, false, scale);
    }
  }

  // draw tiles
  drawTile(&painter, (cols + 2)*8*scale, 0, bank_, tile_, false, false, 16);

  // draw screen tiles
  for (int screen = 0; screen < 2; ++screen) {
    drawScreen(&painter, 0, 2*(screen + 1)*(rows + 2)*8*scale, screen, 0, 1);
  }

  // draw sprites
  drawSprites(&painter, (32 + 2)*8*scale, 2*(rows + 2)*8*scale, 4);
}

void
CQGameBoyGraphics::
drawScreen(QPainter *painter, int x, int y, int screen, int bank, int scale)
{
  CZ80 *z80 = gameboy_->gameboy()->getZ80();

  ushort memStart = (screen == 0 ? 0x9800 : 0x9C00);

  ushort p = memStart;

  for (int ty = 0; ty < 32; ++ty) {
    for (int tx = 0; tx < 32; ++tx) {
      uchar tile = z80->getByte(p);

      int x1 = x + 8*tx*scale;
      int y1 = y + 8*ty*scale;

      drawTile(painter, x1, y1, bank, tile, false, false, scale);

      ++p;
    }
  }
}

void
CQGameBoyGraphics::
drawTile(QPainter *painter, int x, int y, int bank, int tile, bool xflip, bool yflip, int scale)
{
#if 1
  for (int line = 0; line < 8; ++line)
    drawTileLine(painter, x, y, bank, tile, line, xflip, yflip, scale);
#else
  ushort memStart = (bank == 0 ? 0x8800 : 0x8000);

  // 0 transparent for sprites

  static QColor colors[4] = { QColor(255, 255, 255),
                              QColor(192, 192, 192),
                              QColor( 96,  96,  96),
                              QColor(  0,   0,   0) };

  CZ80 *z80 = gameboy_->gameboy()->getZ80();

  ushort p = memStart + tile*16;

  for (int line = 0; line < 8; ++line) {
    ushort pi = p + 2*line;

    uchar b1 = z80->getByte(pi    );
    uchar b2 = z80->getByte(pi + 1);

    for (int pixel = 0; pixel < 8; ++pixel) {
      bool set1 = (b1 & (1 << pixel));
      bool set2 = (b2 & (1 << pixel));

      int ind = (2*set2 + set1);

      if (ind == 0)
        continue;

      int j1;

      if (xflip)
        j1 = pixel;
      else
        j1 = 7 - pixel;

      int i1;

      if (yflip)
        i1 = 7 - line;
      else
        i1 = line;

      if (scale <= 1) {
        painter->setPen(colors[ind]);

        painter->drawPoint(x + j1, y + i1);
      }
      else {
        painter->fillRect(QRect(x + j1*scale, y + i1*scale, scale, scale), colors[ind]);
      }
    }
  }
#endif
}

void
CQGameBoyGraphics::
drawTileLine(QPainter *painter, int x, int y, int bank, int tile, int line,
             bool xflip, bool yflip, int scale)
{
  // 0 transparent for sprites

  static QColor colors[4] = { QColor(255, 255, 255),
                              QColor(192, 192, 192),
                              QColor( 96,  96,  96),
                              QColor(  0,   0,   0) };

  //---

  CZ80 *z80 = gameboy_->gameboy()->getZ80();

  ushort memStart = (bank == 0 ? 0x8800 : 0x8000);

  ushort p = memStart + tile*16;

  ushort pi = p + 2*line;

  uchar b1 = z80->getByte(pi    );
  uchar b2 = z80->getByte(pi + 1);

  for (int pixel = 0; pixel < 8; ++pixel) {
    bool set1 = (b1 & (1 << pixel));
    bool set2 = (b2 & (1 << pixel));

    int ind = (2*set2 + set1);

    if (ind == 0)
      continue;

    int j1;

    if (xflip)
      j1 = pixel;
    else
      j1 = 7 - pixel;

    int i1;

    if (yflip)
      i1 = 7 - line;
    else
      i1 = line;

    if (scale <= 1) {
      painter->setPen(colors[ind]);

      painter->drawPoint(x + j1, y + i1);
    }
    else {
      painter->fillRect(QRect(x + j1*scale, y + i1*scale, scale, scale), colors[ind]);
    }
  }
}

void
CQGameBoyGraphics::
drawSprites(QPainter *painter, int x, int y, int scale)
{
  CGameBoySprite sprite;

  // 160 bytes: 40 sprites, 4 bytes each
  for (int i = 0; i < 40; ++i) {
    gameboy_->gameboy()->getSprite(i, sprite);

    if (sprite.y == 0 || sprite.y >= 160)
      continue;

    if (sprite.x == 0 || sprite.x >= 168)
      continue;

    sprite.x += 8;
    sprite.y += 16;

    drawTile(painter, x + sprite.x*scale, y + sprite.y*scale,
             sprite.bankNum, sprite.t, sprite.xflip, sprite.yflip, scale);
  }
}

void
CQGameBoyGraphics::
displaySprites()
{
  CGameBoySprite sprite;

  // 160 bytes: 40 sprites, 4 bytes each
  for (int i = 0; i < 40; ++i) {
    gameboy_->gameboy()->getSprite(i, sprite);

    sprite.print(std::cerr);

    std::cerr << std::endl;
  }
}

void
CQGameBoyGraphics::
keyPressEvent(QKeyEvent *e)
{
  CKeyEvent *kevent = CQUtil::convertEvent(e);

  CKeyType type = kevent->getType();

  if      (type == CKEY_TYPE_Left) {
    if (tile_ > 0)
      --tile_;
  }
  else if (type == CKEY_TYPE_Right) {
    if (tile_ < 255)
      ++tile_;
  }
  else if (type == CKEY_TYPE_Up) {
    if (bank_ > 0)
      --bank_;
  }
  else if (type == CKEY_TYPE_Down) {
    if (bank_ < 1)
      ++bank_;
  }

  update();
}

QSize
CQGameBoyGraphics::
sizeHint() const
{
  return QSize(1024, 1024);
}

//------

class CQGameBoySpriteItem : public QListWidgetItem {
 public:
  CQGameBoySpriteItem(CQGameBoySpriteList *list, int ind) :
   list_(list), ind_(ind) {
  }

  int ind() const { return ind_; }

 private:
  CQGameBoySpriteList *list_ { nullptr };
  int                  ind_  { 0 };
};

class CQGameBoySpriteDelegate : public QItemDelegate {
 public:
  CQGameBoySpriteDelegate(CQGameBoySpriteList *list) :
   list_(list) {
  }

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const {
    QItemDelegate::drawBackground(painter, option, index);

    CQGameBoySpriteItem *item = static_cast<CQGameBoySpriteItem *>(list_->item(index.row()));

    CGameBoySprite sprite;

    list_->video()->gameboy()->gameboy()->getSprite(item->ind(), sprite);

    QString text = QString("%1 : %2 %3").arg(sprite.i).arg(sprite.x).arg(sprite.y);

    QItemDelegate::drawDisplay(painter, option, option.rect, text);
  }

  QSize sizeHint(const QStyleOptionViewItem &item, const QModelIndex &index) const {
    //CQGameBoySpriteItem *item = static_cast<CQGameBoySpriteItem *>(list_->item(index.row()));

    return QItemDelegate::sizeHint(item, index);
  }

 private:
  CQGameBoySpriteList *list_ { nullptr };
};

CQGameBoySpriteList::
CQGameBoySpriteList(CQGameBoyVideo *video) :
 QListWidget(), video_(video)
{
  setItemDelegate(new CQGameBoySpriteDelegate(this));

  for (int i = 0; i < 40; ++i) {
    addItem(new CQGameBoySpriteItem(this, i));
  }
}
