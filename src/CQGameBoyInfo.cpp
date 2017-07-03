#include <CQGameBoyInfo.h>
#include <CQGameBoy.h>
#include <CStrUtil.h>
#include <QLabel>
#include <QGridLayout>

#include <iostream>

CQGameBoyInfo::
CQGameBoyInfo(CQGameBoy *gameboy) :
 gameboy_(gameboy)
{
  setObjectName("info");

  setWindowTitle("GameBoy Info");

  QGridLayout *layout = new QGridLayout(this);

  titleLabel_   = addLabel(layout, "Title");
  typeLabel_    = addLabel(layout, "Type");
  romLabel_     = addLabel(layout, "ROM");
  ramLabel_     = addLabel(layout, "RAM");
  makerLabel_   = addLabel(layout, "Maker");
  versionLabel_ = addLabel(layout, "Version");
  complement_   = addLabel(layout, "Complement");
  checkSum_     = addLabel(layout, "Checksum");

  updateState();
}

QLabel *
CQGameBoyInfo::
addLabel(QGridLayout *layout, const QString &text)
{
  int r = layout->rowCount();

  QLabel *label = new QLabel;

  layout->addWidget(new QLabel(text), r, 0);
  layout->addWidget(label           , r, 1);

  return label;
}

void
CQGameBoyInfo::
updateState()
{
  CZ80 *z80 = gameboy_->getZ80();

  QString title;

  for (ushort pos = 0x134; pos <= 0x143; ++pos) {
    char c = (char) z80->getByte(pos);

    if (c == 0)
      break;

    title += c;
  }

  titleLabel_->setText(title);

  //---

  uchar type = z80->getByte(0x147);

  QString typeText;

  switch (type) {
    case 0: typeText = "ROM ONLY"            ; break;
    case 1: typeText = "ROM+MBC1"            ; break;
    case 2: typeText = "ROM+MBC1+RAM"        ; break;
    case 3: typeText = "ROM+MBC1+RAM+BATTERY"; break;
    case 5: typeText = "ROM+MBC2"            ; break;
    case 6: typeText = "ROM+MBC2+BATTERY"    ; break;
  }

  typeLabel_->setText(QString("%1 (%2)").arg(typeText).arg(int(type)));

  //---

  uchar rom = z80->getByte(0x148);

  QString romText;

  switch (rom) {
    case 0: romText = "256kbit"; break;
    case 1: romText = "512kbit"; break;
    case 2: romText = "1M-Bit" ; break;
    case 3: romText = "2M-Bit" ; break;
    case 4: romText = "4M-Bit" ; break;
  }

  romLabel_->setText(QString("%1 (%2)").arg(romText).arg(int(rom)));

  //---

  uchar ram = z80->getByte(0x149);

  QString ramText;

  switch (ram) {
    case 0: ramText = "NONE"   ; break;
    case 1: ramText = "16kbit" ; break;
    case 2: ramText = "64kbit" ; break;
    case 3: ramText = "256kbit"; break;
  }

  ramLabel_->setText(QString("%1 (%2)").arg(ramText).arg(int(ram)));

  //---

  uchar m1 = z80->getByte(0x14a);
  uchar m2 = z80->getByte(0x14b);

  makerLabel_->setText(CZ80::hexString((ushort) MAKE_DWORD(m1, m2)).c_str());

  //---

  uchar ver = z80->getByte(0x14c);

  versionLabel_->setText(CZ80::hexString(ver).c_str());

  //---

  uchar compliment = z80->getByte(0x14c);

  complement_->setText(CZ80::hexString(compliment).c_str());

  //---

  uchar check1 = z80->getByte(0x14e);
  uchar check2 = z80->getByte(0x14f);

  checkSum_->setText(CZ80::hexString((ushort) MAKE_DWORD(check1, check2)).c_str());
}
