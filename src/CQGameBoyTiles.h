#ifndef CQGameBoyTiles_H
#define CQGameBoyTiles_H

#include <QFrame>

class CQGameBoyVideo;
class CQGameBoyTilesCanvas;
class QSpinBox;

class CQGameBoyTiles : public QFrame {
  Q_OBJECT

  Q_PROPERTY(int scale READ getScale WRITE setScale)

 public:
  CQGameBoyTiles(CQGameBoyVideo *video);

 ~CQGameBoyTiles();

  CQGameBoyVideo *video() const { return video_; }

  int getScale() const { return scale_; }
  void setScale(int i) { scale_ = i; }

 private slots:
  void scaleSlot();

 private:
  CQGameBoyVideo*       video_     { nullptr };
  int                   scale_     { 1 };
  QSpinBox*             scaleSpin_ { nullptr };
  CQGameBoyTilesCanvas* canvas1_   { nullptr };
  CQGameBoyTilesCanvas* canvas2_   { nullptr };
};

//------

class CQGameBoyTilesCanvas : public QFrame {
  Q_OBJECT

  Q_PROPERTY(int bank READ getBank WRITE setBank)

 public:
  CQGameBoyTilesCanvas(CQGameBoyTiles *tiles, int bank);

  int getBank() const { return bank_; }
  void setBank(int i) { bank_ = i; update(); }

  void paintEvent(QPaintEvent *);

  QSize sizeHint() const;

 private:
  CQGameBoyTiles *tiles_ { nullptr };
  int             bank_  { 0 };
};

#endif
