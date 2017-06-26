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
class CQGameBoyDbg;

class CQGameBoy : public CGameBoy {
 public:
  CQGameBoy();

  CQGameBoyVideo     *addVideo();
  CQGameBoyKeys      *addKeys();
  CQGameBoyInterrupt *addInterrupt();
  CQGameBoyTimer     *addTimer();
  CQGameBoyDbg       *addDebug();

  void setFixedFont(const QFont &font);

  void updateKeys() override;

  void keyPress  (CKeyType key) override;
  void keyRelease(CKeyType key) override;

 private:
  QFont                        fixedFont_;
  QPointer<CQGameBoyScreen>    screen_;
  QPointer<CQGameBoyVideo>     video_;
  QPointer<CQGameBoyKeys>      keys_;
  QPointer<CQGameBoyInterrupt> interrupt_;
  QPointer<CQGameBoyTimer>     timer_;
  QPointer<CQGameBoyDbg>       dbg_;
};

#endif
