#ifndef CQGameBoyGraphics_H
#define CQGameBoyGraphics_H

#include <CZ80.h>
#include <QListWidget>
#include <QLineEdit>

class CQGameBoyVideo;
class CQGameBoyGraphics;
class CQGameBoy;
class QLabel;
class QSpinBox;
class QCheckBox;

//---

class CQGameBoyVideoRegEdit : public QLineEdit {
  Q_OBJECT

  Q_PROPERTY(int value READ value WRITE setValue)

 public:
  CQGameBoyVideoRegEdit(CQGameBoyVideo *video, const QString &name, ushort addr);

  CQGameBoyVideo *video() const { return video_; }

  QLabel *label() const { return label_; }
  void setLabel(QLabel *label) { label_ = label; }

  int value() const;
  void setValue(int v);

  void setFont(const QFont &font);

  void update();

 signals:
  void valueChanged(int);

 private slots:
  void valueSlot();

 private:
  CQGameBoyVideo* video_ { nullptr };
  QString         name_;
  ushort          addr_  { 0 };
  QLabel*         label_ { nullptr };
};

//---

class CQGameBoySpriteList : public QListWidget {
  Q_OBJECT

 public:
  CQGameBoySpriteList(CQGameBoyVideo *video);

  CQGameBoyVideo *video() const { return video_; }

 private:
  CQGameBoyVideo *video_ { nullptr };
};

//---

class CQGameBoyVideo : public QWidget, public CZ80Trace {
  Q_OBJECT

 public:
  CQGameBoyVideo(CQGameBoy *gameboy);

 ~CQGameBoyVideo();

  CQGameBoy *gameboy() const { return gameboy_; }

  bool isTrace() const { return trace_; }
  void setTrace(bool b) { trace_ = b; }

  int getScale() const;
  void setScale(int i);

  void setFixedFont(const QFont &font);

  void memChanged(ushort pos, ushort len) override;

 private:
  QFrame *addRegisterWidget(const QString &label, ushort addr);

 private slots:
  void scaleSlot();
  void traceSlot();
  void redrawSlot();

 private:
  typedef std::map<QString,CQGameBoyVideoRegEdit*> NameRegisterEdits;
  typedef std::map<ushort,CQGameBoyVideoRegEdit*>  AddrRegisterEdits;

  CQGameBoy*         gameboy_    { nullptr };
  CQGameBoyGraphics* graphics_   { nullptr };
  QSpinBox*          scaleSpin_  { nullptr };
  NameRegisterEdits  nameEdits_;
  AddrRegisterEdits  addrEdits_;
  QCheckBox*         traceCheck_ { nullptr };
  bool               trace_      { true };
};

//---

class CQGameBoyGraphics : public QWidget {
  Q_OBJECT

  Q_PROPERTY(int scale READ getScale WRITE setScale)

 public:
  CQGameBoyGraphics(CQGameBoy *gameboy);

 ~CQGameBoyGraphics();

  int getScale() const { return scale_; }
  void setScale(int i) { scale_ = i; }

  void paintEvent(QPaintEvent *);
  void keyPressEvent(QKeyEvent *e);

  void drawTile(QPainter *painter, int x, int y, int bank, int tile,
                bool xflip, bool yflip, int scale);

  void drawTileLine(QPainter *painter, int x, int y, int bank, int tile, int line,
                    bool xflip, bool yflip, int scale);

  void drawScreen(QPainter *painter, int x, int y, int screen, int bank, int scale);

  void drawSprites(QPainter *painter, int x, int y, int scale);

  void displaySprites();

  QSize sizeHint() const;

 private:
  CQGameBoy *gameboy_ { nullptr };
  int        scale_   { 1 };
  int        bank_    { 0 };
  int        tile_    { 0 };
};

//---

#endif
