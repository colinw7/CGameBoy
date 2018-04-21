#ifndef CQGameBoyPalette_H
#define CQGameBoyPalette_H

#include <QFrame>

class CQGameBoyVideo;
class CQGameBoyColorPalette;
class CQGameBoyPaletteSet;

class QLabel;
class QToolButton;

class CQGameBoyPalette : public QFrame {
  Q_OBJECT

 public:
  CQGameBoyPalette(CQGameBoyVideo *video);

 ~CQGameBoyPalette();

  CQGameBoyVideo *video() const { return video_; }

  void update();

 private slots:
  void colorPalettePressed(int color, int palette);

  void colorSetPressed(int color, int addr);

  void colorButtonSlot();

 private:
  void setButtonColor(const QColor &c);

 private:
  typedef std::vector<CQGameBoyColorPalette *> Palettes;
  typedef std::vector<CQGameBoyPaletteSet *>   Sets;

  CQGameBoyVideo* video_       { nullptr };
  Palettes        palettes_;
  Sets            sets_;
  QLabel*         colorLabel_  { nullptr };
  QToolButton*    colorButton_ { nullptr };
  int             palette_     { 0 };
  int             addr_        { 0 };
  int             color_       { 0 };
};

//------

class CQGameBoyColorPalette : public QFrame {
  Q_OBJECT

 public:
  CQGameBoyColorPalette(CQGameBoyPalette *sprites, uchar ind);

  void mousePressEvent(QMouseEvent *e);

  void paintEvent(QPaintEvent *);

  QSize sizeHint() const;

 signals:
  void colorPressed(int, int);

 private:
  CQGameBoyPalette *palette_ { nullptr };
  uchar             ind_     { 0 };
  uint              size_    { 16 };
  QColor            bg_      { 255, 255, 255 };
};

//------

class CQGameBoyPaletteSet : public QFrame {
  Q_OBJECT

 public:
  CQGameBoyPaletteSet(CQGameBoyPalette *sprites, const QString &name, ushort addr);

  void mousePressEvent(QMouseEvent *e);

  void paintEvent(QPaintEvent *);

  QSize sizeHint() const;

 signals:
  void colorPressed(int, int);

 private:
  CQGameBoyPalette *palette_ { nullptr };
  QString           name_;
  ushort            addr_    { 0 };
  uint              size_    { 16 };
  QColor            bg_      { 100, 100, 200 };
};

#endif
