#include <CQGameBoyVideo.h>
#include <CQGameBoyGraphics.h>
#include <CQGameBoyScreen.h>
#include <CQGameBoyTiles.h>
#include <CQGameBoyTile.h>
#include <CQGameBoySprites.h>
#include <CQGameBoySpriteList.h>
#include <CQGameBoyPalette.h>
#include <CQGameBoyVReg.h>
#include <CQGameBoy.h>
#include <QHBoxLayout>
#include <QPainter>

#include <iostream>

CQGameBoyVideo::
CQGameBoyVideo(CQGameBoyScreen *screen) :
 CZ80Trace(*screen->gameboy()->getZ80()), screen_(screen)
{
  setWindowTitle("GameBoy Video");

  setObjectName("video");

  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  QTabWidget *tab = new QTabWidget;

  layout->addWidget(tab);

  //---

  graphics_ = new CQGameBoyGraphics(this);

  tab->addTab(graphics_, "Screen");

  //---

  tiles_ = new CQGameBoyTiles(this);

  tab->addTab(tiles_, "Tiles");

  //---

  tile_ = new CQGameBoyTile(this);

  tab->addTab(tile_, "Tile");

  //---

  sprites_ = new CQGameBoySprites(this);

  tab->addTab(sprites_, "Sprites");

  //---

  palette_ = new CQGameBoyPalette(this);

  tab->addTab(palette_, "Palette");

  //---

  vreg_ = new CQGameBoyVReg(this);

  tab->addTab(vreg_, "Registers");

  //---

  screen->gameboy()->getZ80()->addTrace(this);
}

CQGameBoyVideo::
~CQGameBoyVideo()
{
}

void
CQGameBoyVideo::
setFixedFont(const QFont &font)
{
  vreg_->setFixedFont(font);
}

void
CQGameBoyVideo::
setScale(int scale)
{
  graphics_->setScale(scale);
}

void
CQGameBoyVideo::
memChanged(ushort pos, ushort len)
{
  if (pos >= 0xff40)
    vreg_->updateMem(pos, len);
}

void
CQGameBoyVideo::
updateTiles()
{
  graphics_->update();

  tiles_->update();

  tile_->update(); // need bank and tile
}

void
CQGameBoyVideo::
updateScreen()
{
  graphics_->update();
}

void
CQGameBoyVideo::
updateSprites()
{
  sprites_->update();
}

void
CQGameBoyVideo::
updatePalette()
{
  palette_->update();
}

void
CQGameBoyVideo::
drawScreen(QPainter *painter, int x, int y, int screen, int bank, int scale)
{
  CZ80 *z80 = this->screen()->gameboy()->getZ80();

  uchar palette = z80->getByte(0xff47);

  ushort memStart = (screen == 0 ? 0x9800 : 0x9C00);

  ushort p = memStart;

  // draw whole screen (not just visible area)
  for (int ty = 0; ty < 32; ++ty) {
    for (int tx = 0; tx < 32; ++tx) {
      uchar tile = z80->getByte(p);

      int x1 = x + 8*tx*scale;
      int y1 = y + 8*ty*scale;

      drawTile(painter, x1, y1, bank, tile, palette, false, false, scale);

      ++p;
    }
  }
}

void
CQGameBoyVideo::
drawTile(QPainter *painter, int x, int y, int bank, int tile,
         uchar palette, bool xflip, bool yflip, int scale)
{
  for (int line = 0; line < 8; ++line)
    drawTileLine(painter, x, y, bank, tile, line, palette, xflip, yflip, scale);
}

void
CQGameBoyVideo::
drawTileLine(QPainter *painter, int x, int y, int bank, int tile, int line,
             uchar palette, bool xflip, bool yflip, int scale)
{
  CZ80 *z80 = screen()->gameboy()->getZ80();

  ushort p = screen()->gameboy()->getTileAddr(bank, tile);

  ushort pi = p + 2*line; // 2 bytes per line

  uchar b1 = z80->getByte(pi    );
  uchar b2 = z80->getByte(pi + 1);

  for (int pixel = 0; pixel < 8; ++pixel) {
    bool set1 = TST_BIT(b1, pixel);
    bool set2 = TST_BIT(b2, pixel);

    int ind = (2*set2 + set1);

    // 0 transparent for sprites
    //if (ind == 0)
    //  continue;

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

    const QColor &c = screen()->gameboy()->mappedPaletteColor(palette, ind);

    if (scale <= 1) {
      painter->setPen(c);

      painter->drawPoint(x + j1, y + i1);
    }
    else {
      painter->fillRect(QRect(x + j1*scale, y + i1*scale, scale, scale), c);
    }
  }
}
