#ifndef CQGameBoyInterrupt_H
#define CQGameBoyInterrupt_H

#include <QWidget>
#include <CZ80.h>

class CQGameBoy;
class CQGameBoyHexEdit;
class CQGameBoyInterruptCanvas;
class QCheckBox;

//---

class CQGameBoyInterrupt : public QWidget, public CZ80Trace {
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
  CQGameBoyHexEdit*         iffEdit_     { nullptr };
  CQGameBoyHexEdit*         statEdit_    { nullptr };
  CQGameBoyInterruptCanvas* canvas_      { nullptr };
};

//---

class CQGameBoyInterruptCanvas : public QWidget {
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
