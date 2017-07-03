#ifndef CQZ80Stack_H
#define CQZ80Stack_H

#include <QTextEdit>

class CZ80;

class CQZ80Stack : public QTextEdit {
 public:
  CQZ80Stack(CZ80 *z80);

  void setFixedFont(const QFont &font);

  void update();

 private:
  CZ80 *z80_ { nullptr };
};

#endif
