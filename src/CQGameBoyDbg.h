#ifndef CQGameBoyDbg_H
#define CQGameBoyDbg_H

#include <CQZ80Dbg.h>

class CQGameBoy;
class QPushButton;

class CQGameBoyDbg : public CQZ80Dbg {
  Q_OBJECT

 public:
  CQGameBoyDbg(CQGameBoy *gameboy);

  void addButtonsWidgets() override;

 private slots:
  void bootSlot();

 private:
  CQGameBoy   *gameboy_    { nullptr };
  QPushButton *bootButton_ { nullptr };
};

#endif
