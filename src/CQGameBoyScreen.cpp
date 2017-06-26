#include <CQGameBoyScreen.h>
#include <CQGameBoy.h>
#include <CQUtil.h>
#include <QMouseEvent>
#include <svg/GameboyShell_png.h>

#include <QPainter>
#include <QMenu>
#include <QTimer>

CQGameBoyScreen::
CQGameBoyScreen(CQGameBoy *gameboy) :
 CZ80Screen(*gameboy->getZ80()), gameboy_(gameboy)
{
  setFocusPolicy(Qt::StrongFocus);

  int s = gameboy->getScale();

  border_ = 4*s;

  int w = gameboy_->getScreenPixelWidth ()*s + 2*border_;
  int h = gameboy_->getScreenPixelHeight()*s + 2*border_;

  resize(302, 383);

  image_ = new QImage(QSize(w, h), QImage::Format_ARGB32);

  image_->fill(0);

  pixmap_.loadFromData(GameboyShell_data, GAMEBOYSHELL_DATA_LEN);

  ipainter_ = new QPainter(image_);
}

CQGameBoyScreen::
~CQGameBoyScreen()
{
}

void
CQGameBoyScreen::
exec()
{
  instTimer_ = new QTimer;

  connect(instTimer_, SIGNAL(timeout()), this, SLOT(instTimeOut()));

  instTimer_->start(instTimerLen());
}

void
CQGameBoyScreen::
screenMemChanged(ushort /*start*/, ushort /*len*/)
{
  //if (gameboy_->onScreen(start, len))

  update();
}

void
CQGameBoyScreen::
screenStep(int t)
{
  // scan line 456 clocks
  //  80 (OAM) Mode 2, 172 (VRAM) Mode 3, 204 Horizontal Blank (Mode 0)
  //  Vertical Blank is 10 lines (4560 clocks) Mode 1

  CZ80 *z80 = gameboy_->getZ80();

  screenScan_ += t;

  switch (screenMode_) {
    case 2: { // OAM
      if (screenScan_ >= 80) {
        screenScan_ = 0;

        setLCDMode(3); // VRAM
      }

      break;
    }
    case 3: { // VRAM
      if (screenScan_ >= 172) {
        screenScan_ = 0;

        setLCDMode(0); // Horizontal Blank
      }

      break;
    }
    case 0: { // Horizontal Blank
      if (screenScan_ >= 204) {
        screenScan_ = 0;

        ++screenLine_;

        if (screenLine_ >= 144) {
          setLCDMode(1); // Vertical Blank

          if (z80->getAllowInterrupts())
            z80->setBit(0xff0f, 0);
        }
        else {
          setLCDMode(2); // OAM
        }

        updateLCDLine();
      }

      break;
    }
    case 1: { // Vertical Blank
      if (screenScan_ >= 456) {
        screenScan_ = 0;

        ++screenLine_;

        if (screenLine_ >= 154) {
          screenLine_ = 0;

          setLCDMode(2); // OAM

          // double buffer switch ?
          drawScreen();

          z80->callPostStepProcs();
        }

        updateLCDLine();
      }

      break;
    }
    default: {
      assert(false);
    }
  }
}

void
CQGameBoyScreen::
setLCDMode(int mode)
{
  assert(mode >= 0 && mode <= 3);

  if (mode == int(screenMode_))
    return;

  screenMode_ = mode;

  CZ80 *z80 = gameboy_->getZ80();

  uchar stat = z80->getByte(0xff41);

  // update mode in STAT (bottom two bits)
  z80->setByte(0xff41, (stat & 0xFC) | screenMode_);

  if (TST_BIT(stat, 5))
    z80->setBit(0xff0f, 1);

  if      (screenMode_ == 0) { // Horizontal Blank
    if (TST_BIT(stat, 3))
      z80->setBit(0xff0f, 1); // enable interrupt if STAT bit set
  }
  else if (screenMode_ == 1) { // Vertical Blank
    if (TST_BIT(stat, 4))
      z80->setBit(0xff0f, 1); // enable interrupt if STAT bit set
  }
  else if (screenMode_ == 2) { // OAM
    if (TST_BIT(stat, 5))
      z80->setBit(0xff0f, 1); // enable interrupt if STAT bit set
  }
}

void
CQGameBoyScreen::
updateLCDLine()
{
  CZ80 *z80 = gameboy_->getZ80();

  z80->setByte(0xff44, screenLine_);

  uchar stat = z80->getByte(0xff41);

  uchar lyc = z80->getByte(0xff45); // LYC

  if (lyc == screenLine_) {
    z80->setBit(0xff41, 2); // update conicidence in STAT

    if (TST_BIT(stat, 6))
      z80->setBit(0xff0f, 1); // enable interrupt if STAT bit set
  }
  else {
    z80->resetBit(0xff41, 2); // update conicidence in STAT

    if (TST_BIT(stat, 6))
      z80->resetBit(0xff0f, 0); // disable interrupt if STAT bit set
  }
}

void
CQGameBoyScreen::
redraw()
{
  update();
}

void
CQGameBoyScreen::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.drawPixmap(0, 0, pixmap_);

  painter.drawImage(QPoint(61, 30), *image_);

  painter.setPen(Qt::NoPen);

  arect_ = QRect(214, 231, 34, 34);
  brect_ = QRect(171, 258, 34, 34);

  //painter.setPen(Qt::red);
  //painter.drawRect(arect_);
  //painter.drawRect(brect_);

  //painter.drawEllipse(arect_);
  //painter.drawEllipse(brect_);

  painter.setPen(Qt::white);

  QFont font;

  font.setPointSizeF(18);

  painter.setFont(font);

  painter.drawText(arect_, Qt::AlignCenter, "A");
  painter.drawText(brect_, Qt::AlignCenter, "B");

  selectRect_ = QRect( 97, 321, 26, 9);
  startRect_  = QRect(140, 321, 26, 9);

  //painter.setPen(Qt::red);
  //painter.drawRect(selectRect_);
  //painter.drawRect(startRect_);

  QRect selectTextRect( 94, 333, 32, 9);
  QRect startTextRect (137, 333, 32, 9);

  font.setPointSizeF(6);

  painter.setPen(Qt::black);

  painter.setFont(font);

  painter.drawText(selectTextRect, Qt::AlignCenter, "SELECT");
  painter.drawText(startTextRect , Qt::AlignCenter, "START" );

  QPainterPath path;

  upRect_    = QRect(67, 230, 18, 18);
  leftRect_  = QRect(44, 252, 18, 18);
  rightRect_ = QRect(90, 253, 18, 18);
  downRect_  = QRect(67, 277, 18, 18);

  //painter.setPen(Qt::red);
  //painter.drawRect(leftRect_);
  //painter.drawRect(upRect_);
  //painter.drawRect(downRect_);
  //painter.drawRect(rightRect_);

  path.moveTo(68, 248);
  path.lineTo(76, 230);
  path.lineTo(84, 248);

  path.closeSubpath();

  path.moveTo(62, 253);
  path.lineTo(44, 261);
  path.lineTo(62, 269);

  path.closeSubpath();

  path.moveTo( 91, 253);
  path.lineTo(109, 261);
  path.lineTo( 91, 269);

  path.closeSubpath();

  path.moveTo(68, 277);
  path.lineTo(76, 295);
  path.lineTo(84, 277);

  path.closeSubpath();

  painter.fillPath(path, Qt::white);
}

void
CQGameBoyScreen::
mousePressEvent(QMouseEvent *e)
{
  mousePress_ = true;
  mouseKey_   = CKEY_TYPE_NUL;

  if      (arect_.contains(e->pos())) {
    mouseKey_ = gameboy_->aKey(); //std::cerr << "A" << std::endl;
  }
  else if (brect_.contains(e->pos())) {
    mouseKey_ = gameboy_->bKey(); //std::cerr << "B" << std::endl;
  }
  else if (selectRect_.contains(e->pos())) {
    mouseKey_ = gameboy_->selectKey(); //std::cerr << "SELECT" << std::endl;
  }
  else if (startRect_.contains(e->pos())) {
    mouseKey_ = gameboy_->startKey(); //std::cerr << "START" << std::endl;
  }
  else if (leftRect_.contains(e->pos())) {
    mouseKey_ = gameboy_->leftKey(); //std::cerr << "Left" << std::endl;
  }
  else if (rightRect_.contains(e->pos())) {
    mouseKey_ = gameboy_->rightKey(); //std::cerr << "Right" << std::endl;
  }
  else if (upRect_.contains(e->pos())) {
    mouseKey_ = gameboy_->upKey(); //std::cerr << "Up" << std::endl;
  }
  else if (downRect_.contains(e->pos())) {
    mouseKey_ = gameboy_->downKey(); //std::cerr << "Down" << std::endl;
  }
  else {
    //std::cerr << e->x() << " " << e->y() << std::endl;
    return;
  }

  gameboy_->keyPress(mouseKey_);
}

void
CQGameBoyScreen::
mouseReleaseEvent(QMouseEvent *)
{
  if (mousePress_) {
    gameboy_->keyRelease(mouseKey_);

    mousePress_ = false;
  }
}

void
CQGameBoyScreen::
contextMenuEvent(QContextMenuEvent *e)
{
  QMenu *menu = new QMenu();

  menu->addAction("Video"    , this, SLOT(videoSlot    ()));
  menu->addAction("Debug"    , this, SLOT(debugSlot    ()));
  menu->addAction("Keys"     , this, SLOT(keysSlot     ()));
  menu->addAction("Interrupt", this, SLOT(interruptSlot()));
  menu->addAction("Timer"    , this, SLOT(timerSlot    ()));

  menu->exec(e->globalPos());

  delete menu;
}

void
CQGameBoyScreen::
videoSlot()
{
  gameboy_->addVideo();
}

void
CQGameBoyScreen::
debugSlot()
{
  gameboy_->addDebug();
}

void
CQGameBoyScreen::
keysSlot()
{
  gameboy_->addKeys();
}

void
CQGameBoyScreen::
interruptSlot()
{
  gameboy_->addInterrupt();
}

void
CQGameBoyScreen::
timerSlot()
{
  gameboy_->addTimer();
}

void
CQGameBoyScreen::
drawScreen()
{
  for (uint y = 0; y < 144; ++y) {
    for (uint x = 0; x < 160; ++x) {
      drawScreenPixel(x, y);
    }
  }

  CGameBoySprite sprite;

  // 160 bytes: 40 sprites, 4 bytes each
  for (int i = 0; i < 40; ++i) {
    gameboy_->getSprite(i, sprite);

    int x1 = sprite.x - 8;
    int x2 = x1 + 8;

    if (x2 < 0 || x1 >= 144)
      continue;

    int y1 = sprite.y - 16;
    int y2 = y1 + 8;

    if (y2 < 0 || y1 >= 160)
      continue;

    for (int y = 0; y < 8; ++y) {
      for (int x = 0; x < 8; ++x) {
        drawTilePixel(x1 + x, y1 + y, sprite.bankNum, sprite.t, x, y);
      }
    }
  }

  update();
}

void
CQGameBoyScreen::
drawScreenPixel(int pixel, int line)
{
  CZ80 *z80 = gameboy_->getZ80();

  uchar lcdc = z80->getByte(0xff40);

//bool lcdDisplay    = TST_BIT(lcdc, 7);
//int  windowTile    = TST_BIT(lcdc, 6);
//bool windowDisplay = TST_BIT(lcdc, 5);
  int  bgWindowData  = TST_BIT(lcdc, 4);
  int  bgTile        = TST_BIT(lcdc, 3);
//int  spriteSize    = TST_BIT(lcdc, 2);
//int  spriteDisplay = TST_BIT(lcdc, 1);
//int  bgDisplay     = TST_BIT(lcdc, 0);

  uchar scy = z80->getByte(0xff42);
  uchar scx = z80->getByte(0xff43);

  int pixel1 = (pixel + scx) & 0xff;
  int line1  = (line  + scy) & 0xff;

  //---

  ushort memStart = (bgTile == 0 ? 0x9800 : 0x9C00);

  int ty = line1 / 8;
  int tl = line1 % 8;

  int tx = pixel1 / 8;
  int tp = pixel1 % 8;

  ushort p = memStart + ty*32 + tx; // 32 bytes per line, 1 bytes per tile

  uchar tile = z80->getByte(p);

  drawTilePixel(pixel, line, bgWindowData, tile, tp, tl);
}

void
CQGameBoyScreen::
drawTilePixel(int x, int y, int bank, int tile, int pixel, int line)
{
  // colors : 0 transparent for sprites
  static QColor colors[4] = { QColor(255, 255, 255),
                              QColor(192, 192, 192),
                              QColor( 96,  96,  96),
                              QColor(  0,   0,   0) };

  //---

  CZ80 *z80 = gameboy_->getZ80();

  int scale = gameboy_->getScale();

  ushort memStart = (bank == 0 ? 0x8800 : 0x8000);

  ushort p = memStart + tile*16; // 16 bytes per tile

  ushort pi = p + 2*line; // 2 bytes per line

  uchar b1 = z80->getByte(pi    );
  uchar b2 = z80->getByte(pi + 1);

  int ipixel = 7 - pixel;

  bool set1 = (b1 & (1 << ipixel));
  bool set2 = (b2 & (1 << ipixel));

  int ind = (2*set2 + set1);

  //if (ind == 0) continue;

  if (scale <= 1) {
    ipainter_->setPen(colors[ind]);

    ipainter_->drawPoint(x*scale, y*scale);
  }
  else {
    ipainter_->fillRect(QRect(x*scale, y*scale, scale, scale), colors[ind]);
  }
}

void
CQGameBoyScreen::
keyPressEvent(QKeyEvent *e)
{
  CKeyEvent *kevent = CQUtil::convertEvent(e);

  CKeyType type = kevent->getType();

  CZ80 *z80 = gameboy_->getZ80();

  if      (type == CKEY_TYPE_Escape)
    exit(0);
  else if (type == CKEY_TYPE_F1)
    z80->saveSnapshot();
  else if (type == CKEY_TYPE_F2)
    z80->resetOpCounts();
  else
    z80->keyPress(*kevent);
}

void
CQGameBoyScreen::
keyReleaseEvent(QKeyEvent *e)
{
  CKeyEvent *kevent = CQUtil::convertEvent(e);

  CKeyType type = kevent->getType();

  if (type == CKEY_TYPE_Escape || type == CKEY_TYPE_F1 || type == CKEY_TYPE_F2)
    return;

  CZ80 *z80 = gameboy_->getZ80();

  z80->keyRelease(*kevent);
}

void
CQGameBoyScreen::
instTimeOut()
{
  CZ80 *z80 = gameboy_->getZ80();

  for (int i = 0; i < instSteps(); ++i)
    z80->step();
}

QSize
CQGameBoyScreen::
sizeHint() const
{
  return QSize(1024, 1024);
}

//------

#if 0
void
CQGameBoyRenderer::
clear(const CRGBA &bg)
{
  painter_->fillRect(qgameboy_->rect(), QBrush(CQUtil::rgbaToColor(bg)));
}
#endif
