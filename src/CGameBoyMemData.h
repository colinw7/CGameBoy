#ifndef CGameBoyMemData_H
#define CGameBoyMemData_H

#include <CZ80MemData.h>

class CGameBoy;

class CGameBoyMemData : public CZ80MemData {
 public:
  CGameBoyMemData(CGameBoy *gameboy);

  CGameBoy *gameboy() const { return gameboy_; }

  bool isEnabled() const { return enabled_; }
  void setEnabled(bool b) { enabled_ = b; }

  bool memRead(uchar *data, ushort pos, ushort) override;

  void memPreWrite (const uchar *data, ushort pos, ushort) override;
  void memPostWrite(const uchar *data, ushort pos, ushort) override;

  bool memTrigger(const uchar *data, ushort pos, ushort);

 private:
  void memReadData(ushort pos, uchar *data);

  void memPreWriteData (ushort pos, uchar data);
  void memPostWriteData(ushort pos, uchar data);

  void memTriggerData(uchar data, ushort pos);

 private:
  CGameBoy *gameboy_  { nullptr };
  bool      enabled_  { true };
  uchar     lastLCDC_ { 0 };
};

#endif
