#include <CQGameBoy.h>
#include <CGameBoy.h>
#include <CQUtil.h>
#include <QMouseEvent>
#include <svg/GameboyShell_png.h>

#include <QPainter>
#include <QTimer>

CQGameBoy::
CQGameBoy(CGameBoy *gameboy) :
 CZ80Screen(*gameboy->getZ80()), gameboy_(gameboy)
{
  setFocusPolicy(Qt::StrongFocus);

  int s = gameboy->getScale();

  border_ = 4*s;

  int w = gameboy_->getScreenPixelWidth ()*s + 2*border_;
  int h = gameboy_->getScreenPixelHeight()*s + 2*border_;

  resize(302, 383);

  image_ = new QImage(QSize(w, h), QImage::Format_ARGB32);

  pixmap_.loadFromData(GameboyShell_data, GAMEBOYSHELL_DATA_LEN);

  ipainter_ = new QPainter(image_);
}

CQGameBoy::
~CQGameBoy()
{
}

void
CQGameBoy::
exec()
{
  instTimer_ = new QTimer;

  connect(instTimer_, SIGNAL(timeout()), this, SLOT(instTimeOut()));

  instTimer_->start(instTimerLen());
}

void
CQGameBoy::
screenMemChanged(ushort /*start*/, ushort /*len*/)
{
  //if (gameboy_->onScreen(start, len))

  update();
}

void
CQGameBoy::
screenStep(int t)
{
  // scan line 456 clocks
  //  80 (OAM) Mode 2, 172 (VRAM) Mode 3, 204 Horizontal Blank (Mode 0)
  // Vertical Black is 10 lines (4560 clocks) Mode 1

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

          if (z80->getAllowInterrups())
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
CQGameBoy::
setLCDMode(int mode)
{
  assert(mode >= 0 && mode <= 3);

  screenMode_ = mode;

  CZ80 *z80 = gameboy_->getZ80();

  // update mode in stat (bottom two bits)
  z80->setByte(0xff41, (z80->getByte(0xff41) & 0xFC) | screenMode_);
}

void
CQGameBoy::
updateLCDLine()
{
  CZ80 *z80 = gameboy_->getZ80();

  z80->setByte(0xff44, screenLine_);

  uchar lyc = z80->getByte(0xff45); // LYC

  if (lyc == screenLine_)
    z80->setBit  (0xff41, 2);
  else
    z80->resetBit(0xff41, 2);
}

void
CQGameBoy::
redraw()
{
  update();
}

void
CQGameBoy::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.drawPixmap(0, 0, pixmap_);

  painter.drawImage(QPoint(61, 30), *image_);

  painter.setPen(Qt::NoPen);
  painter.setBrush(Qt::black);

  QRect arect(214, 231, 34, 34);
  QRect brect(171, 258, 34, 34);

  //painter.drawEllipse(arect);
  //painter.drawEllipse(brect);

  painter.setPen(Qt::white);

  QFont font;

  font.setPointSizeF(20);

  painter.setFont(font);

  painter.drawText(arect, Qt::AlignCenter, "A");
  painter.drawText(brect, Qt::AlignCenter, "B");

  QPainterPath path;

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
CQGameBoy::
mousePressEvent(QMouseEvent *e)
{
std::cerr << e->x() << " " << e->y() << std::endl;
}

void
CQGameBoy::
drawScreen()
{
  for (uint y = 0; y < 144; ++y) {
    for (uint x = 0; x < 160; ++x) {
      drawScreenPixel(x, y);
    }
  }

  update();
}

void
CQGameBoy::
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
CQGameBoy::
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
CQGameBoy::
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
CQGameBoy::
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
CQGameBoy::
instTimeOut()
{
  CZ80 *z80 = gameboy_->getZ80();

  for (int i = 0; i < instSteps(); ++i)
    z80->step();
}

QSize
CQGameBoy::
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
