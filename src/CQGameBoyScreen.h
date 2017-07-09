#ifndef CQGameBoyScreen_H
#define CQGameBoyScreen_H

#include <CZ80Screen.h>
#include <CZ80.h>
#include <CEvent.h>
#include <QFrame>

class CQGameBoy;

//---

class CQGameBoyScreen : public QFrame, public CZ80Screen {
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

  void startTimer();
  void stopTimer();

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

  void drawSprites();

  void drawLineSprites(int y);

 public:
  QSize sizeHint() const;

 public slots:
  void instTimeOut();

  void videoSlot();
  void debugSlot();
  void keysSlot();
  void interruptSlot();
  void timerSlot();
  void infoSlot();

 private:
  void setLCDMode(int mode);

  void updateLCDLine();

  void drawBackground();

  void drawScanLine(int line);
  void drawScanPixel(int pixel, int line);
  void drawBackgroundPixel(int pixel, int line);

  void drawTilePixel(int x, int y, int bank, int tile, int pixel, int line,
                     uchar palette, bool isSprite);

  void displaySprites();

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
  uchar      lcdc_       { 0 };
  uchar      scy_        { 0 };
  uchar      scx_        { 0 };
  uchar      palette1_   { 0 };
  uchar      palette2_   { 0 };
  uchar      wy_         { 0 };
  uchar      wx_         { 0 };
};

#endif
