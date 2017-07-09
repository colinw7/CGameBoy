#ifndef CQGameBoyTile_H
#define CQGameBoyTile_H

#include <QFrame>

class CQGameBoyVideo;
class CQGameBoyTileCanvas;
class QSpinBox;

class CQGameBoyTile : public QFrame {
  Q_OBJECT

  Q_PROPERTY(int scale READ getScale WRITE setScale)
  Q_PROPERTY(int bank  READ getBank  WRITE setBank )
  Q_PROPERTY(int vbank READ getVBank WRITE setVBank)
  Q_PROPERTY(int tile  READ getTile  WRITE setTile )

 public:
  CQGameBoyTile(CQGameBoyVideo *video);

 ~CQGameBoyTile();

  CQGameBoyVideo *video() const { return video_; }

  int getScale() const { return scale_; }
  void setScale(int i) { scale_ = i; update(); }

  int getBank() const { return bank_; }
  void setBank(int i) { bank_ = i; update(); }

  int getVBank() const { return vbank_; }
  void setVBank(int i) { vbank_ = i; update(); }

  int getTile() const { return tile_; }
  void setTile(int i) { tile_ = i; update(); }

  void update();

 private slots:
  void bankSlot();
  void tileSlot();
  void scaleSlot();

 private:
  CQGameBoyVideo*      video_     { nullptr };
  int                  scale_     { 16 };
  int                  bank_      { 0 };
  int                  vbank_     { 0 };
  int                  tile_      { 0 };
  QSpinBox*            bankSpin_  { nullptr };
  QSpinBox*            tileSpin_  { nullptr };
  QSpinBox*            scaleSpin_ { nullptr };
  CQGameBoyTileCanvas* canvas_    { nullptr };
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
