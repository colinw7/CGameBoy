#ifndef CQGameBoy_H
#define CQGameBoy_H

#include <CGameBoy.h>
#include <QWidget>
#include <QColor>
#include <QFont>
#include <QPointer>

class CQGameBoyScreen;
class CQGameBoyVideo;
class CQGameBoyKeys;
class CQGameBoyInterrupt;
class CQGameBoyTimer;
class CQGameBoyInfo;
class CQGameBoyDbg;

class CQGameBoy : public QWidget, public CGameBoy {
  Q_OBJECT

  Q_PROPERTY(QColor color0    READ color0    WRITE setColor0   )
  Q_PROPERTY(QColor color1    READ color1    WRITE setColor1   )
  Q_PROPERTY(QColor color2    READ color2    WRITE setColor2   )
  Q_PROPERTY(QColor color3    READ color3    WRITE setColor3   )
  Q_PROPERTY(QFont  fixedFont READ fixedFont WRITE setFixedFont)

 public:
  CQGameBoy();

  CQGameBoyScreen *screen() const;

  const QColor &color0() const { return color0_; }
  void setColor0(const QColor &v) { color0_ = v; }

  const QColor &color1() const { return color1_; }
  void setColor1(const QColor &v) { color1_ = v; }

  const QColor &color2() const { return color2_; }
  void setColor2(const QColor &v) { color2_ = v; }

  const QColor &color3() const { return color3_; }
  void setColor3(const QColor &v) { color3_ = v; }

  void createScreen();

  CQGameBoyVideo     *addVideo();
  CQGameBoyKeys      *addKeys();
  CQGameBoyInterrupt *addInterrupt();
  CQGameBoyTimer     *addTimer();
  CQGameBoyInfo      *addInfo();
  CQGameBoyDbg       *addDebug();

  const QFont &fixedFont() const { return fixedFont_; }
  void setFixedFont(const QFont &font);

  void updateKeys   () override;
  void updateTiles  () override;
  void updateScreen () override;
  void updateSprites() override;
  void updatePalette() override;

  void keyPress  (CKeyType key) override;
  void keyRelease(CKeyType key) override;

  void execStop(bool b) override;

  QColor vramBgPaletteColor(uchar palette, uchar ind) const;

  QColor vramSpritePaletteColor(uchar palette, uchar ind) const;

  const QColor &mappedPaletteColor(uchar palette, uchar ind) const;

  const QColor &paletteColor(uchar i) const;

 private:
  QColor                       color0_ { 255, 255, 255 };
  QColor                       color1_ { 192, 192, 192 };
  QColor                       color2_ {  96,  96,  96 };
  QColor                       color3_ {   0,   0,   0 };
  QFont                        fixedFont_;
  QPointer<CQGameBoyScreen>    screen_;
  QPointer<CQGameBoyVideo>     video_;
  QPointer<CQGameBoyKeys>      keys_;
  QPointer<CQGameBoyInterrupt> interrupt_;
  QPointer<CQGameBoyTimer>     timer_;
  QPointer<CQGameBoyInfo>      info_;
  QPointer<CQGameBoyDbg>       dbg_;
};

#endif
