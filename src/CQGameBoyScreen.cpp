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

  resize(s*302, s*383);

  image_ = new QImage(QSize(w, h), QImage::Format_ARGB32);

  image_->fill(0);

  pixmap_.loadFromData(GameboyShell_data, GAMEBOYSHELL_DATA_LEN);

  if (s > 1)
    pixmap_ = pixmap_.scaled(pixmap_.width()*s, pixmap_.height()*s);

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

  startTimer();
}

void
CQGameBoyScreen::
startTimer()
{
  if (instTimer_)
    instTimer_->start(instTimerLen());
}

void
CQGameBoyScreen::
stopTimer()
{
  if (instTimer_)
    instTimer_->stop();
}

void
CQGameBoyScreen::
screenMemChanged(ushort /*start*/, ushort /*len*/)
{
  //if (gameboy()->onScreen(start, len))

  update();
}

void
CQGameBoyScreen::
screenStep(int t)
{
  // scan line 456 clocks
  //   80 (OAM) Mode 2
  //  172 (VRAM) Mode 3
  //  204 Horizontal Blank (Mode 0)
  //
  // Vertical Blank is 10 lines (4560 clocks) Mode 1

  CZ80 *z80 = gameboy()->getZ80();

  screenScan_ += t;

  switch (screenMode_) {
    case 2: { // OAM (Sprite DRAW)
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

        // draw scan line
        drawScanLine(screenLine_);
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

          // draw background
          drawBackground();

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

  CZ80 *z80 = gameboy()->getZ80();

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
  CZ80 *z80 = gameboy()->getZ80();

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
  int scale = gameboy()->getScale();

  //---

  QPainter painter(this);

  painter.drawPixmap(0, 0, pixmap_);

  painter.drawImage(QPoint(scale*61, scale*30), *image_);

  //---

  // draw A/B buttons
  painter.setPen(Qt::NoPen);

  arect_ = QRect(scale*214, scale*231, scale*34, scale*34);
  brect_ = QRect(scale*171, scale*258, scale*34, scale*34);

  //painter.setPen(Qt::red);
  //painter.drawRect(arect_);
  //painter.drawRect(brect_);

  //painter.drawEllipse(arect_);
  //painter.drawEllipse(brect_);

  painter.setPen(Qt::white);

  QFont font;

  font.setPointSizeF(scale*18);

  painter.setFont(font);

  painter.drawText(arect_, Qt::AlignCenter, "A");
  painter.drawText(brect_, Qt::AlignCenter, "B");

  //---

  // draw Select/Start buttons
  selectRect_ = QRect(scale* 97, scale*321, scale*26, scale*9);
  startRect_  = QRect(scale*140, scale*321, scale*26, scale*9);

  //painter.setPen(Qt::red);
  //painter.drawRect(selectRect_);
  //painter.drawRect(startRect_);

  QRect selectTextRect(scale* 94, scale*333, scale*32, scale*9);
  QRect startTextRect (scale*137, scale*333, scale*32, scale*9);

  font.setPointSizeF(scale*6);

  painter.setPen(Qt::black);

  painter.setFont(font);

  painter.drawText(selectTextRect, Qt::AlignCenter, "SELECT");
  painter.drawText(startTextRect , Qt::AlignCenter, "START" );

  //---

  // draw arrow buttons
  QPainterPath path;

  upRect_    = QRect(scale*67, scale*230, scale*18, scale*18);
  leftRect_  = QRect(scale*44, scale*252, scale*18, scale*18);
  rightRect_ = QRect(scale*90, scale*253, scale*18, scale*18);
  downRect_  = QRect(scale*67, scale*277, scale*18, scale*18);

  //painter.setPen(Qt::red);
  //painter.drawRect(leftRect_);
  //painter.drawRect(upRect_);
  //painter.drawRect(downRect_);
  //painter.drawRect(rightRect_);

  path.moveTo(scale*68, scale*248);
  path.lineTo(scale*76, scale*230);
  path.lineTo(scale*84, scale*248);

  path.closeSubpath();

  path.moveTo(scale*62, scale*253);
  path.lineTo(scale*44, scale*261);
  path.lineTo(scale*62, scale*269);

  path.closeSubpath();

  path.moveTo(scale* 91, scale*253);
  path.lineTo(scale*109, scale*261);
  path.lineTo(scale* 91, scale*269);

  path.closeSubpath();

  path.moveTo(scale*68, scale*277);
  path.lineTo(scale*76, scale*295);
  path.lineTo(scale*84, scale*277);

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
    mouseKey_ = gameboy()->aKey(); //std::cerr << "A" << std::endl;
  }
  else if (brect_.contains(e->pos())) {
    mouseKey_ = gameboy()->bKey(); //std::cerr << "B" << std::endl;
  }
  else if (selectRect_.contains(e->pos())) {
    mouseKey_ = gameboy()->selectKey(); //std::cerr << "SELECT" << std::endl;
  }
  else if (startRect_.contains(e->pos())) {
    mouseKey_ = gameboy()->startKey(); //std::cerr << "START" << std::endl;
  }
  else if (leftRect_.contains(e->pos())) {
    mouseKey_ = gameboy()->leftKey(); //std::cerr << "Left" << std::endl;
  }
  else if (rightRect_.contains(e->pos())) {
    mouseKey_ = gameboy()->rightKey(); //std::cerr << "Right" << std::endl;
  }
  else if (upRect_.contains(e->pos())) {
    mouseKey_ = gameboy()->upKey(); //std::cerr << "Up" << std::endl;
  }
  else if (downRect_.contains(e->pos())) {
    mouseKey_ = gameboy()->downKey(); //std::cerr << "Down" << std::endl;
  }
  else {
    //std::cerr << e->x() << " " << e->y() << std::endl;
    return;
  }

  gameboy()->keyPress(mouseKey_);
}

void
CQGameBoyScreen::
mouseReleaseEvent(QMouseEvent *)
{
  if (mousePress_) {
    gameboy()->keyRelease(mouseKey_);

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
  menu->addAction("Info"     , this, SLOT(infoSlot     ()));

  menu->exec(e->globalPos());

  delete menu;
}

void
CQGameBoyScreen::
videoSlot()
{
  gameboy()->addVideo();
}

void
CQGameBoyScreen::
debugSlot()
{
  gameboy()->addDebug();
}

void
CQGameBoyScreen::
keysSlot()
{
  gameboy()->addKeys();
}

void
CQGameBoyScreen::
interruptSlot()
{
  gameboy()->addInterrupt();
}

void
CQGameBoyScreen::
timerSlot()
{
  gameboy()->addTimer();
}

void
CQGameBoyScreen::
infoSlot()
{
  gameboy()->addInfo();
}

void
CQGameBoyScreen::
drawBackground()
{
  CZ80 *z80 = gameboy()->getZ80();

  lcdc_ = z80->getByte(0xff40);

  scy_ = z80->getByte(0xff42);
  scx_ = z80->getByte(0xff43);

  palette1_ = z80->getByte(0xff47);
  palette2_ = palette1_;

  for (uint y = 0; y < 144; ++y) {
    for (uint x = 0; x < 160; ++x) {
      drawBackgroundPixel(x, y);
    }
  }

  for (uint y = 0; y < 144; ++y)
    drawScanLine(y);

  update();
}

void
CQGameBoyScreen::
drawScanLine(int y)
{
  CZ80 *z80 = gameboy()->getZ80();

  lcdc_ = z80->getByte(0xff40);

  wy_ = z80->getByte(0xff4a);
  wx_ = z80->getByte(0xff4b);

  palette1_ = z80->getByte(0xff47);
  palette2_ = palette1_;

  for (uint x = 0; x < 160; ++x)
    drawScanPixel(x, y);

  //---

  palette1_ = z80->getByte(0xff48);
  palette2_ = z80->getByte(0xff49);

  drawLineSprites(y);

  update();
}

void
CQGameBoyScreen::
drawSprites()
{
  CZ80 *z80 = gameboy()->getZ80();

  lcdc_ = z80->getByte(0xff40);

  palette1_ = z80->getByte(0xff48);
  palette2_ = z80->getByte(0xff49);

  for (uint y = 0; y < 144; ++y)
    drawLineSprites(y);
}

void
CQGameBoyScreen::
drawLineSprites(int y)
{
  bool spriteDisplay = TST_BIT(lcdc_, 1);

  if (! spriteDisplay)
    return;

  int spriteSize = (TST_BIT(lcdc_, 2) ? 16 : 8);

  std::vector<CGameBoySprite> sprites;

  if (! gameboy()->getLineSprites(y, spriteSize, sprites))
    return;

  int bank = 1; // bank always 1

  for (const auto &sprite : sprites) {
    // 160 bytes: 40 sprites, 4 bytes each
    int x1 = sprite.x - 8;
    int y1 = sprite.y - 16;

    int yo  = y - y1;
    int yof = (sprite.yflip ? spriteSize - 1 - yo : yo);

    uchar palette = (sprite.palNum1 == 0 ? palette1_ : palette2_);

    if (spriteSize == 8) {
      for (int xo = 0; xo < 8; ++xo) {
        int xof = (sprite.xflip ? 7 - xo : xo);

        drawTilePixel(x1 + xo, y, bank, sprite.t, xof, yof, palette, true);
      }
    }
    else {
      int t1 = sprite.t & 0xFE;
      int t2 = t1 + 1;

      for (int xo = 0; xo < 8; ++xo) {
        int xof = (sprite.xflip ? 7 - xo : xo);

        if (yo < 8)
          drawTilePixel(x1 + xo, y, bank, t1, xof, yof    , palette, true);
        else
          drawTilePixel(x1 + xo, y, bank, t2, xof, yof - 8, palette, true);
      }
    }
  }
}

void
CQGameBoyScreen::
drawBackgroundPixel(int pixel, int line)
{
  bool lcdDisplay   = TST_BIT(lcdc_, 7); // Screen display on/off
  int  bgWindowData = TST_BIT(lcdc_, 4); // Background Character Data
  int  bgTile       = TST_BIT(lcdc_, 3); // Background Display Data
  int  bgDisplay    = TST_BIT(lcdc_, 0); // Background display on/off

  if (! lcdDisplay)
    return;

  if (! bgDisplay)
    return;

  int pixel1 = (pixel + scx_) & 0xff;
  int line1  = (line  + scy_) & 0xff;

  //---

  // get tile at pixel/line
  ushort tileMem = (bgTile == 0 ? 0x9800 : 0x9C00);

  int ty = line1 / 8;
  int tl = line1 % 8;

  int tx = pixel1 / 8;
  int tp = pixel1 % 8;

  ushort p = tileMem + ty*32 + tx; // 32 bytes per line, 1 bytes per tile

  CZ80 *z80 = gameboy()->getZ80();

  uchar tile = z80->getByte(p);

  // draw tile pixel
  drawTilePixel(pixel, line, bgWindowData, tile, tp, tl, palette1_, false);
}

void
CQGameBoyScreen::
drawScanPixel(int pixel, int line)
{
  bool lcdDisplay    = TST_BIT(lcdc_, 7); // Screen display on/off
  int  windowTile    = TST_BIT(lcdc_, 6); // Window Screen Display Data
  bool windowDisplay = TST_BIT(lcdc_, 5); // Window Display On/Off
  int  bgWindowData  = TST_BIT(lcdc_, 4); // Background Character Data

  if (! lcdDisplay)
    return;

  if (! windowDisplay)
    return;

  if (pixel < wx_ - 7 || line < wy_)
    return;

  //---

  // get tile at pixel/line
  ushort tileMem = (windowTile == 0 ? 0x9800 : 0x9C00);

  int ty = line / 8;
  int tl = line % 8;

  int tx = pixel / 8;
  int tp = pixel % 8;

  ushort p = tileMem + ty*32 + tx; // 32 bytes per line, 1 bytes per tile

  CZ80 *z80 = gameboy()->getZ80();

  uchar tile = z80->getByte(p);

  // draw tile pixel
  drawTilePixel(pixel, line, bgWindowData, tile, tp, tl, palette1_, false);
}

void
CQGameBoyScreen::
drawTilePixel(int x, int y, int bank, int tile, int pixel, int line,
              uchar palette, bool isSprite)
{
  CZ80 *z80 = gameboy()->getZ80();

  int scale = gameboy()->getScale();

  ushort p = gameboy()->getTileAddr(bank, tile);

  ushort pi = p + 2*line; // 2 bytes per line

  uchar b1 = z80->getByte(pi    );
  uchar b2 = z80->getByte(pi + 1);

  int ipixel = 7 - pixel;

  bool set1 = TST_BIT(b1, ipixel);
  bool set2 = TST_BIT(b2, ipixel);

  int ind = (2*set2 + set1);

  // 0 transparent for sprites

  if (ind == 0 && isSprite)
    return;

  const QColor &c = gameboy()->mappedPaletteColor(palette, ind);

  if (scale <= 1) {
    ipainter_->setPen(c);

    ipainter_->drawPoint(x, y);
  }
  else {
    ipainter_->fillRect(QRect(x*scale, y*scale, scale, scale), c);
  }
}

void
CQGameBoyScreen::
keyPressEvent(QKeyEvent *e)
{
  CKeyEvent *kevent = CQUtil::convertEvent(e);

  CKeyType type = kevent->getType();

  CZ80 *z80 = gameboy()->getZ80();

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

  CZ80 *z80 = gameboy()->getZ80();

  z80->keyRelease(*kevent);
}

void
CQGameBoyScreen::
instTimeOut()
{
  CZ80 *z80 = gameboy()->getZ80();

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
  painter_->fillRect(qgameboy()->rect(), QBrush(CQUtil::rgbaToColor(bg)));
}
#endif
