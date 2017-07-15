#ifndef CQGameBoyGraphics_H
#define CQGameBoyGraphics_H

#include <QFrame>

class CQGameBoyVideo;
class CQGameBoyGraphicsCanvas;
class QSpinBox;
class QLabel;

class CQGameBoyGraphics : public QFrame {
  Q_OBJECT

  Q_PROPERTY(int scale READ getScale WRITE setScale)

 public:
  CQGameBoyGraphics(CQGameBoyVideo *video);

 ~CQGameBoyGraphics();

  CQGameBoyVideo *video() const { return video_; }

  int getScale() const { return scale_; }
  void setScale(int i) { scale_ = i; }

  int getTileData() const { return tileData_; }
  void setTileData(int i) { tileData_ = i; }

 private slots:
  void scaleSlot();
  void tileDataSlot();

  void tileClickSlot(int tileMap, int tx, int ty);

 private:
  CQGameBoyVideo*          video_        { nullptr };
  int                      scale_        { 1 };
  int                      tileData_     { 0 };
  QSpinBox*                scaleSpin_    { nullptr };
  QSpinBox*                tileDataSpin_ { nullptr };
  CQGameBoyGraphicsCanvas* canvas1_      { nullptr };
  CQGameBoyGraphicsCanvas* canvas2_      { nullptr };
  QLabel*                  infoLabel_    { nullptr };
};

//------

class CQGameBoyGraphicsCanvas : public QFrame {
  Q_OBJECT

  Q_PROPERTY(int tileMap READ getTileMap WRITE setTileMap)
  Q_PROPERTY(int vbank   READ getVBank   WRITE setVBank  )

 public:
  CQGameBoyGraphicsCanvas(CQGameBoyGraphics *graphics, int tileMap);

  int getTileMap() const { return tileMap_; }
  void setTileMap(int i) { tileMap_ = i; update(); }

  int getVBank() const { return vbank_; }
  void setVBank(int i) { vbank_ = i; update(); }

  void mousePressEvent(QMouseEvent *e);

  void paintEvent(QPaintEvent *);

  QSize sizeHint() const;

 signals:
  void tileClicked(int, int, int);

 private:
  CQGameBoyGraphics* graphics_ { nullptr };
  int                tileMap_  { 0 };
  int                vbank_    { 0 };
  int                tileX_    { 0 };
  int                tileY_    { 0 };
};

#endif
