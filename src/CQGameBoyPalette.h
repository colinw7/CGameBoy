#ifndef CQGameBoyPalette_H
#define CQGameBoyPalette_H

#include <QFrame>

class CQGameBoyVideo;
class CQGameBoyPaletteSet;

class CQGameBoyPalette : public QFrame {
  Q_OBJECT

 public:
  CQGameBoyPalette(CQGameBoyVideo *video);

 ~CQGameBoyPalette();

  CQGameBoyVideo *video() const { return video_; }

  void update();

 private:
  CQGameBoyVideo*      video_ { nullptr };
  CQGameBoyPaletteSet* set1_  { nullptr };
  CQGameBoyPaletteSet* set2_  { nullptr };
  CQGameBoyPaletteSet* set3_  { nullptr };
};

//------

class CQGameBoyPaletteSet : public QFrame {
  Q_OBJECT

 public:
  CQGameBoyPaletteSet(CQGameBoyPalette *sprites, const QString &name, ushort addr);

  void paintEvent(QPaintEvent *);

  QSize sizeHint() const;

 private:
  CQGameBoyPalette *palette_ { nullptr };
  QString           name_;
  ushort            addr_    { 0 };
  uint              size_    { 16 };
  QColor            bg_      { 100, 100, 200 };
};

#endif
