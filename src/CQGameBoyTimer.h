#ifndef CQGameBoyTimer_H
#define CQGameBoyTimer_H

#include <CZ80Trace.h>
#include <QFrame>
#include <CZ80.h>

class CQGameBoy;
class CQGameBoyAddrEdit;

//---

class CQGameBoyTimer : public QFrame, public CZ80Trace {
  Q_OBJECT

 public:
  CQGameBoyTimer(CQGameBoy *gameboy);

  CQGameBoy *gameboy() const { return gameboy_; }

  void memChanged(ushort pos, ushort len);

  void updateState();

 private:
  CQGameBoy*         gameboy_     { nullptr };
  CQGameBoyAddrEdit* dividerEdit_ { nullptr };
  CQGameBoyAddrEdit* counterEdit_ { nullptr };
  CQGameBoyAddrEdit* moduloEdit_  { nullptr };
  CQGameBoyAddrEdit* controlEdit_ { nullptr };
};

#endif
