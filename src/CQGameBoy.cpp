#include <CQGameBoy.h>
#include <CGameBoy.h>
#include <CQUtil.h>

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

  resize(w, h);

  image_ = new QImage(QSize(w, h), QImage::Format_ARGB32);

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

  z80->setByte(0xff41, (z80->getByte(0xff41) & 0xFC) | screenMode_);
}

void
CQGameBoy::
updateLCDLine()
{
  CZ80 *z80 = gameboy_->getZ80();

  z80->setByte(0xff44, screenLine_);
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

  painter.drawImage(QPoint(border_, border_), *image_);
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
  int  windowTile    = TST_BIT(lcdc, 6);
//bool windowDisplay = TST_BIT(lcdc, 5);
//int  windowTile    = TST_BIT(lcdc, 4);
  int  bgTile        = TST_BIT(lcdc, 3);
//int  spriteSize    = TST_BIT(lcdc, 2);
//int  spriteDisplay = TST_BIT(lcdc, 1);
//int  bgDisplay     = TST_BIT(lcdc, 0);

  uchar scy = z80->getByte(0xff42);
  uchar scx = z80->getByte(0xff43);

  int pixel1 = (pixel + scx) & 0xff;
  int line1  = (line  + scy) & 0xff;

  //---

  ushort memStart = (windowTile == 0 ? 0x9800 : 0x9C00);

  int ty = line1 / 8;
  int tl = line1 % 8;

  int tx = pixel1 / 8;
  int tp = pixel1 % 8;

  ushort p = memStart + ty*32 + tx; // 32 bytes per line, 1 bytes per tile

  uchar tile = z80->getByte(p);

  drawTilePixel(pixel, line, bgTile, tile, tp, tl);
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
