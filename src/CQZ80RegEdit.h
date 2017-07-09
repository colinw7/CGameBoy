#ifndef CQZ80RegEdit_H
#define CQZ80RegEdit_H

#include <QFrame>
#include <CZ80.h>

class QLabel;
class QLineEdit;

class CQZ80RegEdit : public QFrame {
  Q_OBJECT

 public:
  CQZ80RegEdit(CZ80 *z80, CZ80Reg reg);

  void setFont(const QFont &font);

  void setValue(uint value);
  uint getValue() const;

 private slots:
  void valueChangedSlot();

 private:
  CZ80*      z80_   { nullptr };
  CZ80Reg    reg_;
  QLabel*    label_ { nullptr };
  QLineEdit* edit_  { nullptr };
};

#endif
