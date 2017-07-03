#ifndef CQGameBoy_H
#define CQGameBoy_H

#include <CGameBoy.h>
#include <QFont>
#include <QPointer>

class CQGameBoyScreen;
class CQGameBoyVideo;
class CQGameBoyKeys;
class CQGameBoyInterrupt;
class CQGameBoyTimer;
class CQGameBoyInfo;
class CQGameBoyDbg;

class CQGameBoy : public CGameBoy {
 public:
  CQGameBoy();

  CQGameBoyScreen *screen() const;

  void createScreen();

  CQGameBoyVideo     *addVideo();
  CQGameBoyKeys      *addKeys();
  CQGameBoyInterrupt *addInterrupt();
  CQGameBoyTimer     *addTimer();
  CQGameBoyInfo      *addInfo();
  CQGameBoyDbg       *addDebug();

  void setFixedFont(const QFont &font);

  void updateKeys   () override;
  void updateTiles  () override;
  void updateScreen () override;
  void updateSprites() override;
  void updatePalette() override;

  void keyPress  (CKeyType key) override;
  void keyRelease(CKeyType key) override;

  void execStop(bool b) override;

  const QColor &mappedPaletteColor(uchar palette, uchar ind) const;

  const QColor &paletteColor(uchar i) const;

 private:
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
