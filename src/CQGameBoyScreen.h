#ifndef CQGameBoyScreen_H
#define CQGameBoyScreen_H

#include <QWidget>
#include <CZ80.h>
#include <CEvent.h>

class CQGameBoy;

//---

class CQGameBoyScreen : public QWidget, public CZ80Screen {
  Q_OBJECT

  Q_PROPERTY(int border       READ border       WRITE setBorder      )
  Q_PROPERTY(int instTimerLen READ instTimerLen WRITE setInstTimerLen)
  Q_PROPERTY(int instSteps    READ instSteps    WRITE setInstSteps   )

 public:
  CQGameBoyScreen(CQGameBoy *gameboy);

 ~CQGameBoyScreen();

  CQGameBoy *gameboy() const { return gameboy_; }

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

  void mousePressEvent  (QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);

  void keyPressEvent  (QKeyEvent *e);
  void keyReleaseEvent(QKeyEvent *e);

  void contextMenuEvent(QContextMenuEvent *e);

  void drawScreen();
  void drawScreenPixel(int pixel, int line);

  void drawTilePixel(int x, int y, int bank, int tile, int pixel, int line);

  void displaySprites();

  QSize sizeHint() const;

 public slots:
  void instTimeOut();

  void videoSlot();
  void debugSlot();
  void keysSlot();
  void interruptSlot();
  void timerSlot();

 private:
  void setLCDMode(int mode);
  void updateLCDLine();

 private:
  CQGameBoy *gameboy_      { nullptr };
  QTimer*    instTimer_    { nullptr };
  int        instTimerLen_ { 1 };
  int        instSteps_    { 1000 };
  int        border_       { 0 };
  QImage*    image_;
  QPixmap    pixmap_;
  QPainter*  ipainter_     { nullptr };
  uint       screenMode_   { 0 };
  uint       screenScan_   { 0 };
  uint       screenLine_   { 0 };
  QRect      leftRect_;
  QRect      upRect_;
  QRect      downRect_;
  QRect      rightRect_;
  QRect      arect_;
  QRect      brect_;
  QRect      selectRect_;
  QRect      startRect_;
  bool       mousePress_ { false };
  CKeyType   mouseKey_   { CKEY_TYPE_NUL };
};

//---

#if 0
class CQGameBoyRenderer : public CGameBoyRenderer {
 public:
  CQGameBoyRenderer(CQGameBoyScreen *qgameboy, QPainter *painter) :
   qgameboy_(qgameboy), painter_(painter) {
  }

  void clear(const CRGBA &bg);

 private:
  CQGameBoyScreen *qgameboy_ { nullptr };
  QPainter  *painter_  { nullptr };
};
#endif

//---

#endif
