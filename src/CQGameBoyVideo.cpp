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
  setObjectName("video");

  setWindowTitle("GameBoy Video");

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
  CQGameBoy *gameboy = this->screen()->gameboy();

  CZ80 *z80 = gameboy->getZ80();

  uchar palette = 0;

  if (! gameboy->isGBC()) {
    palette = z80->getByte(0xff47);
  }

  ushort memStart = (screen == 0 ? 0x9800 : 0x9C00);

  ushort tileMem = memStart;

  bool isSprite = false;

  // draw whole screen (not just visible area)
  for (int ty = 0; ty < 32; ++ty) {
    for (int tx = 0; tx < 32; ++tx) {
      uchar tile = z80->getByte(tileMem);

      bool vbank = 0;
      bool xflip = false;
      bool yflip = false;

      if (gameboy->isGBC()) {
        uchar cp = gameboy->getVRam(1, tileMem - 0x8000);

        // Bit 0-2  Background Palette number  (BGP0-7)
        // Bit 3    Tile VRAM Bank number      (0=Bank 0, 1=Bank 1)
        // Bit 4    Not used
        // Bit 5    Horizontal Flip            (0=Normal, 1=Mirror horizontally)
        // Bit 6    Vertical Flip              (0=Normal, 1=Mirror vertically)
        // Bit 7    BG-to-OAM Priority         (0=Use OAM priority bit, 1=BG Priority)

        palette = cp & 0x7;
        vbank   = TST_BIT(cp, 3);

        xflip = TST_BIT(cp, 5);
        yflip = TST_BIT(cp, 6);

      //int priority = TST_BIT(cp, 7);
      }

      //---

      int x1 = x + 8*tx*scale;
      int y1 = y + 8*ty*scale;

      drawTile(painter, x1, y1, vbank, bank, tile, palette, xflip, yflip, isSprite, scale);

      ++tileMem;
    }
  }
}

void
CQGameBoyVideo::
drawTile(QPainter *painter, int x, int y, int vbank, int bank, int tile,
         uchar palette, bool xflip, bool yflip, bool isSprite, int scale)
{
  for (int line = 0; line < 8; ++line)
    drawTileLine(painter, x, y, vbank, bank, tile, line, palette, xflip, yflip, isSprite, scale);
}

void
CQGameBoyVideo::
drawTileLine(QPainter *painter, int x, int y, int vbank, int bank, int tile, int line,
             uchar palette, bool xflip, bool yflip, bool isSprite, int scale)
{
  CQGameBoy *gameboy = screen()->gameboy();

  CZ80 *z80 = gameboy->getZ80();

  ushort p = gameboy->getTileAddr(bank, tile);

  ushort pi = p + 2*line; // 2 bytes per line

  int oldBank = gameboy->vramBank();

  gameboy->setVRamBank(vbank);

  uchar b1 = z80->getByte(pi    );
  uchar b2 = z80->getByte(pi + 1);

  for (int pixel = 0; pixel < 8; ++pixel) {
    int ipixel = 7 - pixel;

    bool set1 = TST_BIT(b1, ipixel);
    bool set2 = TST_BIT(b2, ipixel);

    int ind = (2*set2 + set1);

    // 0 transparent for sprites
    //if (ind == 0)
    //  continue;

    int j1 = (xflip ? 7 - ipixel : ipixel);
    int i1 = (yflip ? 7 - line   : line  );

    QColor c;

    if (gameboy->isGBC()) {
      if (isSprite)
        c = gameboy->vramSpritePaletteColor(palette, ind);
      else
        c = gameboy->vramBgPaletteColor(palette, ind);
    }
    else {
      c = gameboy->mappedPaletteColor(palette, ind);
    }

    if (scale <= 1) {
      painter->setPen(c);

      painter->drawPoint(x + j1, y + i1);
    }
    else {
      painter->fillRect(QRect(x + j1*scale, y + i1*scale, scale, scale), c);
    }
  }

  gameboy->setVRamBank(oldBank);
}
