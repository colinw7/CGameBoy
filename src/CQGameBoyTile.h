#ifndef CQGameBoyTile_H
#define CQGameBoyTile_H

#include <QFrame>

class CQGameBoyVideo;
class CQGameBoyTileCanvas;
class QSpinBox;
class QCheckBox;

class CQGameBoyTile : public QFrame {
  Q_OBJECT

  Q_PROPERTY(int bank    READ getBank    WRITE setBank   )
  Q_PROPERTY(int vbank   READ getVBank   WRITE setVBank  )
  Q_PROPERTY(int tile    READ getTile    WRITE setTile   )
  Q_PROPERTY(int palette READ getPalette WRITE setPalette)
  Q_PROPERTY(int scale   READ getScale   WRITE setScale  )

 public:
  CQGameBoyTile(CQGameBoyVideo *video);

 ~CQGameBoyTile();

  CQGameBoyVideo *video() const { return video_; }

  int getBank() const { return bank_; }
  void setBank(int i) { bank_ = i; update(); }

  int getVBank() const { return vbank_; }
  void setVBank(int i) { vbank_ = i; update(); }

  int getTile() const { return tile_; }
  void setTile(int i) { tile_ = i; update(); }

  int getPalette() const { return palette_; }
  void setPalette(int i) { palette_ = i; update(); }

  bool getXFlip() const { return xflip_; }
  void setXFlip(bool b) { xflip_ = b; }

  bool getYFlip() const { return yflip_; }
  void setYFlip(bool b) { yflip_ = b; }

  int getScale() const { return scale_; }
  void setScale(int i) { scale_ = i; update(); }

  void update();

 private slots:
  void bankSlot();
  void vbankSlot();
  void tileSlot();
  void paletteSlot();
  void xflipSlot();
  void yflipSlot();
  void scaleSlot();

 private:
  CQGameBoyVideo*      video_       { nullptr };
  int                  bank_        { 0 };
  int                  vbank_       { 0 };
  int                  tile_        { 0 };
  int                  palette_     { 0 };
  bool                 xflip_       { false };
  bool                 yflip_       { false };
  int                  scale_       { 16 };
  QSpinBox*            bankSpin_    { nullptr };
  QSpinBox*            vbankSpin_   { nullptr };
  QSpinBox*            tileSpin_    { nullptr };
  QSpinBox*            paletteSpin_ { nullptr };
  QCheckBox*           xFlipCheck_  { nullptr };
  QCheckBox*           yFlipCheck_  { nullptr };
  QSpinBox*            scaleSpin_   { nullptr };
  CQGameBoyTileCanvas* canvas_      { nullptr };
};

//---

class CQGameBoyTileCanvas : public QFrame {
  Q_OBJECT

 public:
  CQGameBoyTileCanvas(CQGameBoyTile *tile);

 ~CQGameBoyTileCanvas();

  CQGameBoyTile *tile() const { return tile_; }

  void paintEvent(QPaintEvent *e);

  void keyPressEvent(QKeyEvent *e);

  QSize sizeHint() const;

 private:
  CQGameBoyTile *tile_ { nullptr };
};

#endif
