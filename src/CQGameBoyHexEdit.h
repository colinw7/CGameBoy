#ifndef CQGameBoyHexEdit_H
#define CQGameBoyHexEdit_H

#include <QLineEdit>

class CQGameBoyHexEdit : public QLineEdit {
  Q_OBJECT

  Q_PROPERTY(int value READ value WRITE setValue)

 public:
  CQGameBoyHexEdit(ushort value=0);

  int value() const;
  void setValue(int v);

 signals:
  void valueChanged(int);

 protected slots:
  void valueSlot();

 protected:
  ushort value_ { 0 };
};

//---

class CQGameBoy;
class QLabel;

class CQGameBoyAddrEdit : public CQGameBoyHexEdit {
  Q_OBJECT

 public:
  CQGameBoyAddrEdit(CQGameBoy *gameboy, const QString &name, ushort addr);

  CQGameBoy *gameboy() const { return gameboy_; }

  QLabel *label() const { return label_; }
  void setLabel(QLabel *label) { label_ = label; }

  void setFont(const QFont &font);

  void update();

 private slots:
  void addrSlot(int);

 private:
  CQGameBoy* gameboy_ { nullptr };
  QString    name_;
  QLabel*    label_   { nullptr };
};

#endif
