#ifndef CGameBoyPortData_H
#define CGameBoyPortData_H

#include <CZ80PortData.h>

class CGameBoy;

class CGameBoyPortData : public CZ80PortData {
 public:
  CGameBoyPortData(CGameBoy *gameboy);

  CGameBoy *gameboy() const { return gameboy_; }

  void out(uchar port, uchar value);

  uchar in(uchar port, uchar qual);

  void keyPress  (const CKeyEvent &kevent);
  void keyRelease(const CKeyEvent &kevent);

 private:
  CGameBoy *gameboy_ { nullptr };
};

#endif
