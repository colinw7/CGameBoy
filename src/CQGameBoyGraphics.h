#ifndef CQGameBoyGraphics_H
#define CQGameBoyGraphics_H

#include <QFrame>

class CQGameBoyVideo;
class CQGameBoyGraphicsCanvas;
class QSpinBox;

class CQGameBoyGraphics : public QFrame {
  Q_OBJECT

  Q_PROPERTY(int scale READ getScale WRITE setScale)

 public:
  CQGameBoyGraphics(CQGameBoyVideo *video);

 ~CQGameBoyGraphics();

  CQGameBoyVideo *video() const { return video_; }

  int getScale() const { return scale_; }
  void setScale(int i) { scale_ = i; }

 private slots:
  void scaleSlot();
  void bankSlot();

 private:
  CQGameBoyVideo*          video_     { nullptr };
  int                      scale_     { 1 };
  QSpinBox*                scaleSpin_ { nullptr };
  QSpinBox*                bankSpin_  { nullptr };
  CQGameBoyGraphicsCanvas* canvas1_   { nullptr };
  CQGameBoyGraphicsCanvas* canvas2_   { nullptr };
};

//------

class CQGameBoyGraphicsCanvas : public QFrame {
  Q_OBJECT

  Q_PROPERTY(int screen READ getScreen WRITE setScreen)
  Q_PROPERTY(int bank   READ getBank   WRITE setBank  )

 public:
  CQGameBoyGraphicsCanvas(CQGameBoyGraphics *graphics, int screen);

  int getScreen() const { return screen_; }
  void setScreen(int i) { screen_ = i; update(); }

  int getBank() const { return bank_; }
  void setBank(int i) { bank_ = i; update(); }

  void paintEvent(QPaintEvent *);

  QSize sizeHint() const;

 private:
  CQGameBoyGraphics* graphics_ { nullptr };
  int                screen_   { 0 };
  int                bank_     { 0 };
};

#endif
