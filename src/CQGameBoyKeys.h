#ifndef CQGameBoyKeys_H
#define CQGameBoyKeys_H

#include <QWidget>

class CQGameBoy;

//---

class CQGameBoyKeys : public QWidget {
  Q_OBJECT

 public:
  CQGameBoyKeys(CQGameBoy *gameboy);

 ~CQGameBoyKeys();

  CQGameBoy *gameboy() const { return gameboy_; }

  QSize sizeHint() const;

 private:
  void paintEvent(QPaintEvent *);

  void drawKey(QPainter *painter, const QRect &rect, bool on);

 private:
  CQGameBoy* gameboy_ { nullptr };
};

//---

#endif
