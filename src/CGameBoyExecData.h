#ifndef CGameBoyExecData_H
#define CGameBoyExecData_H

#include <CZ80ExecData.h>

class CGameBoy;

class CGameBoyExecData : public CZ80ExecData {
 public:
  CGameBoyExecData(CGameBoy *gameboy);

  CGameBoy *gameboy() const { return gameboy_; }

  void preStep () override;
  void postStep() override;

  void setStop(bool b) override;
  void setHalt(bool b) override;

 private:
  CGameBoy *gameboy_ { nullptr };
  ulong     m_       { 0 };
  ulong     timer_   { 0 };
};

#endif
