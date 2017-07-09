#ifndef CQGameBoyInterrupt_H
#define CQGameBoyInterrupt_H

#include <CZ80Trace.h>
#include <CZ80.h>
#include <QFrame>

class CQGameBoy;
class CQGameBoyHexEdit;
class CQGameBoyInterruptCanvas;
class CQZ80RegEdit;
class QCheckBox;

//---

class CQGameBoyInterrupt : public QFrame, public CZ80Trace {
  Q_OBJECT

 public:
  CQGameBoyInterrupt(CQGameBoy *gameboy);

  CQGameBoy *gameboy() const { return gameboy_; }

  void regChanged(CZ80Reg reg);

  void memChanged(ushort pos, ushort len);

  void updateState();

 private slots:
  void enableSlot();

 private:
  CQGameBoy*                gameboy_     { nullptr };
  QCheckBox*                enableCheck_ { nullptr };
  CQZ80RegEdit*             iffEdit_     { nullptr };
  CQGameBoyHexEdit*         statEdit_    { nullptr };
  CQGameBoyInterruptCanvas* canvas_      { nullptr };
};

//---

class CQGameBoyInterruptCanvas : public QFrame {
  Q_OBJECT

 public:
  CQGameBoyInterruptCanvas(CQGameBoyInterrupt *interrupt);

 ~CQGameBoyInterruptCanvas();

  CQGameBoy *gameboy() const { return interrupt_->gameboy(); }

  QSize sizeHint() const;

 private:
  void paintEvent(QPaintEvent *);

  void drawBox(QPainter *painter, const QRect &rect, bool on);

 private:
  CQGameBoyInterrupt* interrupt_ { nullptr };
};

#endif
