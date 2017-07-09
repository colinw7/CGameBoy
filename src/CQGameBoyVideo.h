#ifndef CQGameBoyVideo_H
#define CQGameBoyVideo_H

#include <QFrame>
#include <CZ80Trace.h>

class CQGameBoyScreen;
class CQGameBoyGraphics;
class CQGameBoyTiles;
class CQGameBoyTile;
class CQGameBoySprites;
class CQGameBoyPalette;
class CQGameBoyVReg;

//---

class CQGameBoyVideo : public QFrame, public CZ80Trace {
  Q_OBJECT

 public:
  CQGameBoyVideo(CQGameBoyScreen *gameboy);

 ~CQGameBoyVideo();

  CQGameBoyScreen *screen() const { return screen_; }

  void setFixedFont(const QFont &font);

  void setScale(int scale);

  void memChanged(ushort pos, ushort len) override;

  void updateTiles  ();
  void updateScreen ();
  void updateSprites();
  void updatePalette();

  void drawScreen(QPainter *painter, int x, int y, int screen, int bank, int scale);

  void drawTile(QPainter *painter, int x, int y, int vbank, int bank, int tile,
                uchar palette, bool xflip, bool yflip, bool isSprite, int scale);

  void drawTileLine(QPainter *painter, int x, int y, int vbank, int bank, int tile, int line,
                    uchar palette, bool xflip, bool yflip, bool isSprite, int scale);

 private:
  CQGameBoyScreen*   screen_   { nullptr };
  CQGameBoyGraphics* graphics_ { nullptr };
  CQGameBoyTiles*    tiles_    { nullptr };
  CQGameBoyTile*     tile_     { nullptr };
  CQGameBoySprites*  sprites_  { nullptr };
  CQGameBoyPalette*  palette_  { nullptr };
  CQGameBoyVReg*     vreg_     { nullptr };
};

#endif
