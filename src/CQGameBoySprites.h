#ifndef CQGameBoySprites_H
#define CQGameBoySprites_H

#include <QFrame>

class CQGameBoyVideo;
class CQGameBoySpritesCanvas;
class CQGameBoySpriteList;
class QSpinBox;
class QCheckBox;

class CQGameBoySprites : public QFrame {
  Q_OBJECT

  Q_PROPERTY(int scale READ getScale WRITE setScale)

 public:
  CQGameBoySprites(CQGameBoyVideo *video);

 ~CQGameBoySprites();

  CQGameBoyVideo *video() const { return video_; }

  int getScale() const { return scale_; }
  void setScale(int i) { scale_ = i; }

  bool isDoubleHeight() const { return doubleHeight_; }
  void setDoubleHeight(bool b) { doubleHeight_ = b; }

  void update();

 private slots:
  void scaleSlot();
  void doubleHeightSlot();

 private:
  CQGameBoyVideo*         video_             { nullptr };
  int                     scale_             { 1 };
  bool                    doubleHeight_      { false };
  QSpinBox*               scaleSpin_         { nullptr };
  QCheckBox*              doubleHeightCheck_ { nullptr };
  CQGameBoySpritesCanvas* canvas_            { nullptr };
  CQGameBoySpriteList*    list_              { nullptr };
};

//------

class CQGameBoySpritesCanvas : public QFrame {
  Q_OBJECT

 public:
  CQGameBoySpritesCanvas(CQGameBoySprites *sprites);

  void paintEvent(QPaintEvent *);

  void drawSprites(QPainter *painter, int x, int y, int scale);

  void displaySprites();

  QSize sizeHint() const;

 private:
  CQGameBoySprites *sprites_ { nullptr };
};

#endif
