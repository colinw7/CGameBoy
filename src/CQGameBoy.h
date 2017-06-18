#ifndef CQGameBoy_H
#define CQGameBoy_H

#include <QWidget>
#include <CZ80.h>
#include <CGameBoy.h>

//---

class CQGameBoy : public QWidget, public CZ80Screen {
  Q_OBJECT

  Q_PROPERTY(int border       READ border       WRITE setBorder      )
  Q_PROPERTY(int instTimerLen READ instTimerLen WRITE setInstTimerLen)
  Q_PROPERTY(int instSteps    READ instSteps    WRITE setInstSteps   )

 public:
  CQGameBoy(CGameBoy *gameboy);

 ~CQGameBoy();

  CGameBoy *gameboy() const { return gameboy_; }

  int border() const { return border_; }
  void setBorder(int i) { border_ = i; }

  int instTimerLen() const { return instTimerLen_; }
  void setInstTimerLen(int i) { instTimerLen_ = i; }

  int instSteps() const { return instSteps_; }
  void setInstSteps(int i) { instSteps_ = i; }

  void exec();

  void screenMemChanged(ushort pos, ushort len) override;

  void screenStep(int t) override;

  void redraw();

  void paintEvent(QPaintEvent *);
  void keyPressEvent(QKeyEvent *e);
  void keyReleaseEvent(QKeyEvent *e);

  void drawScreen();
  void drawScreenPixel(int pixel, int line);

  void drawTilePixel(int x, int y, int bank, int tile, int pixel, int line);

  void displaySprites();

  QSize sizeHint() const;

 public slots:
  void instTimeOut();

 private:
  void setLCDMode(int mode);
  void updateLCDLine();

 private:
  CGameBoy *gameboy_      { nullptr };
  QTimer*   instTimer_    { nullptr };
  int       instTimerLen_ { 1 };
  int       instSteps_    { 1000 };
  int       border_       { 0 };
  QImage*   image_;
  QPainter* ipainter_     { nullptr };
  uint      screenMode_   { 0 };
  uint      screenScan_   { 0 };
  uint      screenLine_   { 0 };
};

//---

#if 0
class CQGameBoyRenderer : public CGameBoyRenderer {
 public:
  CQGameBoyRenderer(CQGameBoy *qgameboy, QPainter *painter) :
   qgameboy_(qgameboy), painter_(painter) {
  }

  void clear(const CRGBA &bg);

 private:
  CQGameBoy *qgameboy_ { nullptr };
  QPainter  *painter_  { nullptr };
};
#endif

//---

#endif
