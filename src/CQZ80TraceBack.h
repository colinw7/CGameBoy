#ifndef CQZ80TraceBack_H
#define CQZ80TraceBack_H

#include <QTextEdit>

class CZ80;

class CQZ80TraceBack : public QTextEdit {
 public:
  CQZ80TraceBack(CZ80 *z80);

  void setFixedFont(const QFont &font);

  void update();

 private:
  CZ80 *z80_ { nullptr };
};

#endif
