#ifndef CQGameBoyInfo_H
#define CQGameBoyInfo_H

#include <QFrame>
#include <CZ80.h>

class CQGameBoy;
class QLabel;
class QGridLayout;

//---

class CQGameBoyInfo : public QFrame {
  Q_OBJECT

 public:
  CQGameBoyInfo(CQGameBoy *gameboy);

  CQGameBoy *gameboy() const { return gameboy_; }

  void updateState();

 private:
  QLabel *addLabel(QGridLayout *layout, const QString &label);

 private:
  CQGameBoy* gameboy_      { nullptr };
  QLabel*    titleLabel_   { nullptr };
  QLabel*    gbTypeLabel_  { nullptr };
  QLabel*    sgbLabel_     { nullptr };
  QLabel*    typeLabel_    { nullptr };
  QLabel*    romLabel_     { nullptr };
  QLabel*    ramLabel_     { nullptr };
  QLabel*    makerLabel_   { nullptr };
  QLabel*    versionLabel_ { nullptr };
  QLabel*    complement_   { nullptr };
  QLabel*    checkSum_     { nullptr };
};

#endif
