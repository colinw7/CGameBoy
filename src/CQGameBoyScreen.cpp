#include <CQGameBoyScreen.h>
#include <CQGameBoy.h>
#include <CQUtil.h>
#include <CQUtilEvent.h>

#include <svg/GameboyShell_png.h>

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QMenu>
#include <QTimer>

CQGameBoyScreen::
CQGameBoyScreen(CQGameBoy *gameboy) :
 CZ80Screen(*gameboy->getZ80()), gameboy_(gameboy)
{
  setObjectName("screen");

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

        if (screenLine_ >= gameboy_->getScreenPixelHeight()) {
          setLCDMode(1); // Vertical Blank
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

  if      (screenMode_ == 0) { // Horizontal Blank
    // signal STAT interrupt if STAT bit set
    if (TST_BIT(stat, 3))
      gameboy()->signalInterrupt(CGameBoy::InterruptType::LCD_STAT);
  }
  else if (screenMode_ == 1) { // Vertical Blank
    // signal Vertical Blank interrupt
    gameboy()->signalInterrupt(CGameBoy::InterruptType::VBLANK);

    // signal STAT interrupt if STAT bit set
    if (TST_BIT(stat, 4))
      gameboy()->signalInterrupt(CGameBoy::InterruptType::LCD_STAT);
  }
  else if (screenMode_ == 2) { // OAM
    // signal STAT interrupt if STAT bit set
    if (TST_BIT(stat, 5))
      gameboy()->signalInterrupt(CGameBoy::InterruptType::LCD_STAT);
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
    z80->setBit(0xff41, 2); // set coincidence in STAT

    // signal STAT interrupt if STAT bit set
    if (TST_BIT(stat, 6))
      gameboy()->signalInterrupt(CGameBoy::InterruptType::LCD_STAT);
  }
  else {
    z80->resetBit(0xff41, 2); // reset coincidence in STAT
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

  menu->addAction("Video"     , this, SLOT(videoSlot    ()));
  menu->addAction("Debug"     , this, SLOT(debugSlot    ()));
  menu->addAction("Keys"      , this, SLOT(keysSlot     ()));
  menu->addAction("Interrupt" , this, SLOT(interruptSlot()));
  menu->addAction("Timer"     , this, SLOT(timerSlot    ()));
  menu->addAction("Memory Map", this, SLOT(memoryMapSlot()));
  menu->addAction("Info"      , this, SLOT(infoSlot     ()));

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
memoryMapSlot()
{
  gameboy()->addMemoryMap();
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

  if (! gameboy()->isGBC()) {
    palette1_ = z80->getByte(0xff47);
    palette2_ = palette1_;
  }
  else {
    palette1_ = 0;
    palette2_ = 0;
  }

  bool lcdDisplay = TST_BIT(lcdc_, 7); // Screen display on/off
  bool bgDisplay  = true;

  if (! gameboy()->isGBC())
    bgDisplay = TST_BIT(lcdc_, 0); // Background display on/off (GB), Sprite Priority (CGB)

  if (lcdDisplay && bgDisplay) {
    for (uint y = 0; y < gameboy_->getScreenPixelHeight(); ++y) {
      for (uint x = 0; x < gameboy_->getScreenPixelWidth(); ++x) {
        drawBackgroundPixel(x, y);
      }
    }
  }
  else {
    const QColor &c = gameboy()->mappedPaletteColor(palette1_, 0);

    ipainter_->fillRect(QRect(0, 0, image_->width(), image_->height()), c);
  }

  //---

  for (uint y = 0; y < gameboy_->getScreenPixelHeight(); ++y)
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

  if (! gameboy()->isGBC()) {
    palette1_ = z80->getByte(0xff47);
    palette2_ = palette1_;
  }
  else {
    palette1_ = 0;
    palette2_ = 0;
  }

  for (uint x = 0; x < gameboy_->getScreenPixelWidth(); ++x)
    drawScanPixel(x, y);

  //---

  if (! gameboy()->isGBC()) {
    palette1_ = z80->getByte(0xff48);
    palette2_ = z80->getByte(0xff49);
  }
  else {
    palette1_ = 0;
    palette2_ = 0;
  }

  drawLineSprites(y);

  update();
}

void
CQGameBoyScreen::
drawSprites()
{
  CZ80 *z80 = gameboy()->getZ80();

  lcdc_ = z80->getByte(0xff40);

  if (! gameboy()->isGBC()) {
    palette1_ = z80->getByte(0xff48);
    palette2_ = z80->getByte(0xff49);
  }
  else {
    palette1_ = 0;
    palette2_ = 0;
  }

  for (uint y = 0; y < gameboy_->getScreenPixelHeight(); ++y)
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

    uchar palette;

    if (gameboy()->isGBC()) {
      palette = sprite.palNum;
    }
    else {
      palette = (sprite.palNum == 0 ? palette1_ : palette2_);
    }

    if (spriteSize == 8) {
      for (int xo = 0; xo < 8; ++xo) {
        int xof = (sprite.xflip ? 7 - xo : xo);

        int x2 = x1 + xo;

        if (x2 >= 0 && x2 < int(gameboy_->getScreenPixelWidth())) {
          drawTilePixel(x2, y, bank, sprite.t, xof, yof, sprite.bankNum, palette, true);
        }
      }
    }
    else {
      int t1 = sprite.t & 0xFE;
      int t2 = t1 + 1;

      for (int xo = 0; xo < 8; ++xo) {
        int xof = (sprite.xflip ? 7 - xo : xo);

        int x2 = x1 + xo;

        if (x2 >= 0 && x2 < int(gameboy_->getScreenPixelWidth())) {
          if (yo < 8)
            drawTilePixel(x2, y, bank, t1, xof, yof    , sprite.bankNum, palette, true);
          else
            drawTilePixel(x2, y, bank, t2, xof, yof - 8, sprite.bankNum, palette, true);
        }
      }
    }
  }
}

void
CQGameBoyScreen::
drawBackgroundPixel(int pixel, int line)
{
  bool lcdDisplay = TST_BIT(lcdc_, 7); // Screen display on/off

  if (! lcdDisplay)
    return;

  if (! gameboy()->isGBC()) {
    bool bgDisplay = TST_BIT(lcdc_, 0); // Background display on/off

    if (! bgDisplay)
      return;
  }

  //---

  int bgWindowData = TST_BIT(lcdc_, 4); // Background Character Data
  int bgTileMap    = TST_BIT(lcdc_, 3); // Background Display Data

  //CZ80 *z80 = gameboy()->getZ80();

  int pixel1 = (pixel + scx_) & 0xff;
  int line1  = (line  + scy_) & 0xff;

  //---

  // get tile at pixel/line
  int ty = line1  / 8;
  int tx = pixel1 / 8;

  //ushort tileInd = ty*32 + tx; // 32 bytes per line, 1 byte per tile
  //ushort tileMem = (bgTileMap == 0 ? 0x9800 : 0x9C00) + tileInd;

  // get tile internal pixel coord
  int tl = line1  % 8;
  int tp = pixel1 % 8;

  //uchar tile = z80->getByte(tileMem);
  uchar tile = gameboy()->getTileNum(bgTileMap, tx, ty);

  CGameBoyTileAttr attr;

  if (gameboy()->isGBC()) {
#if 0
    // get color attributes for tile (from VRAM 1)
    uchar cp = gameboy()->getVRam(1, tileMem - 0x8000);

    // Bit 0-2  Background Palette number  (BGP0-7)
    // Bit 3    Tile VRAM Bank number      (0=Bank 0, 1=Bank 1)
    // Bit 4    Not used
    // Bit 5    Horizontal Flip            (0=Normal, 1=Mirror horizontally)
    // Bit 6    Vertical Flip              (0=Normal, 1=Mirror vertically)
    // Bit 7    BG-to-OAM Priority         (0=Use OAM priority bit, 1=BG Priority)

    attr.pnum     = cp & 0x7;
    attr.bank     = TST_BIT(cp, 3);
    attr.hflip    = TST_BIT(cp, 5);
    attr.vflip    = TST_BIT(cp, 6);
    attr.priority = TST_BIT(cp, 7);
#else
    gameboy()->getTileAttr(bgTileMap, tx, ty, attr);
#endif
  }
  else {
    attr.pnum = palette1_;
  }

  //---

  // flip internal pixel if needed
  int tp1 = (attr.hflip ? 7 - tp : tp);
  int tl1 = (attr.vflip ? 7 - tl : tl);

  //---

  // draw tile pixel
  drawTilePixel(pixel, line, bgWindowData, tile, tp1, tl1, attr.bank, attr.pnum, false);
}

void
CQGameBoyScreen::
drawScanPixel(int pixel, int line)
{
  bool lcdDisplay = TST_BIT(lcdc_, 7); // Screen display on/off

  if (! lcdDisplay)
    return;

  bool windowDisplay = TST_BIT(lcdc_, 5); // Window Display On/Off

  if (! windowDisplay)
    return;

  if (pixel < wx_ - 7 || line < wy_)
    return;

  //---

  int bgWindowData  = TST_BIT(lcdc_, 4); // Background Character Data
  int windowTileMap = TST_BIT(lcdc_, 6); // Window Screen Display Data

  //CZ80 *z80 = gameboy()->getZ80();

  //---

  // get tile at pixel/line
  int ty = line  / 8;
  int tx = pixel / 8;

  //ushort tileInd = ty*32 + tx; // 32 bytes per line, 1 byte per tile
  //ushort tileMem = (windowTileMap == 0 ? 0x9800 : 0x9C00) + tileInd;

  // get tile internal pixel coord
  int tl = line  % 8;
  int tp = pixel % 8;

  //uchar tile = z80->getByte(tileMem);
  uchar tile = gameboy()->getTileNum(windowTileMap, tx, ty);

  CGameBoyTileAttr attr;

  if (gameboy()->isGBC()) {
#if 0
    // get color attributes for tile (from VRAM 1)
    uchar cp = gameboy()->getVRam(1, tileMem - 0x8000);

    // Bit 0-2  Background Palette number  (BGP0-7)
    // Bit 3    Tile VRAM Bank number      (0=Bank 0, 1=Bank 1)
    // Bit 4    Not used
    // Bit 5    Horizontal Flip            (0=Normal, 1=Mirror horizontally)
    // Bit 6    Vertical Flip              (0=Normal, 1=Mirror vertically)
    // Bit 7    BG-to-OAM Priority         (0=Use OAM priority bit, 1=BG Priority)

    attr.pnum     = cp & 0x7;
    attr.bank     = TST_BIT(cp, 3);
    attr.hflip    = TST_BIT(cp, 5);
    attr.vflip    = TST_BIT(cp, 6);
    attr.priority = TST_BIT(cp, 7);
#else
    gameboy()->getTileAttr(windowTileMap, tx, ty, attr);
#endif
  }
  else {
    attr.pnum = palette1_;
  }

  //---

  // flip internal pixel if needed
  int tp1 = (attr.hflip ? 7 - tp : tp);
  int tl1 = (attr.vflip ? 7 - tl : tl);

  //---

  // draw tile pixel
  drawTilePixel(pixel, line, bgWindowData, tile, tp1, tl1, attr.bank, attr.pnum, false);
}

void
CQGameBoyScreen::
drawTilePixel(int x, int y, int bank, int tile, int pixel, int line, int vbank,
              int palette, bool isSprite)
{
  int scale = gameboy()->getScale();

  //---

  CZ80 *z80 = gameboy()->getZ80();

  //---

  ushort p = gameboy()->getTileAddr(bank, tile);

  ushort pi = p + 2*line; // 2 bytes per line

  assert(pi >= 0x8000 && pi < 0x9fff);

  uchar b1, b2;

  if (gameboy_->isGBC()) {
    b1 = gameboy()->getVRam(vbank, pi     - 0x8000);
    b2 = gameboy()->getVRam(vbank, pi + 1 - 0x8000);
  }
  else {
    b1 = z80->getMemory(pi    );
    b2 = z80->getMemory(pi + 1);
  }

  //---

  int ipixel = 7 - pixel;

  bool set1 = TST_BIT(b1, ipixel);
  bool set2 = TST_BIT(b2, ipixel);

  int ind = (2*set2 + set1);

  // 0 transparent for sprites

  if (ind == 0 && isSprite)
    return;

  QColor c;

  if (gameboy()->isGBC()) {
    if (isSprite)
      c = gameboy()->vramSpritePaletteColor(palette, ind);
    else
      c = gameboy()->vramBgPaletteColor(palette, ind);
  }
  else {
    c = gameboy()->mappedPaletteColor(palette, ind);
  }

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
    z80->execStep();
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
