#ifndef CQZ80FlagCheck_H
#define CQZ80FlagCheck_H

#include <QCheckBox>
#include <CZ80.h>

class CQZ80FlagCheck : public QCheckBox {
  Q_OBJECT

 public:
  CQZ80FlagCheck(CZ80 *z80, CZ80Flag flag);

  QString flagName() const;

  void updateState();

  static QString flagName(CZ80Flag flag);

 private:
  CZ80     *z80_  { nullptr };
  CZ80Flag  flag_ { CZ80Flag::NONE };
};

#endif
