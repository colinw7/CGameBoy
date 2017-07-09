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
  gbTypeLabel_  = addLabel(layout, "GameBoy");
  sgbLabel_     = addLabel(layout, "SGB");
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

  uchar gbType = z80->getByte(0x143);

  QString gbTypeText;

  switch (gbType) {
    case 0x00: gbTypeText = "BW"      ; break;
    case 0x80: gbTypeText = "BW/Color"; break;
    case 0xc0: gbTypeText = "Color"; break;
  }

  gbTypeLabel_->setText(QString("%1 (%2)").arg(gbTypeText).arg(int(gbType)));

  //---

  uchar sgbFlag = z80->getByte(0x146);

  QString sgbText;

  switch (sgbFlag) {
    case 0x00: sgbText = "None"     ; break;
    case 0x03: sgbText = "Supported"; break;
  }

  sgbLabel_->setText(QString("%1 (%2)").arg(sgbText).arg(int(sgbFlag)));

  //---

  uchar type = z80->getByte(0x147);

  QString typeText;

  switch (type) {
    case 0x00: typeText = "ROM ONLY"               ; break;
    case 0x01: typeText = "ROM+MBC1"               ; break;
    case 0x02: typeText = "ROM+MBC1+RAM"           ; break;
    case 0x03: typeText = "ROM+MBC1+RAM+BATTERY"   ; break;
    case 0x05: typeText = "ROM+MBC2"               ; break;
    case 0x06: typeText = "ROM+MBC2+BATTERY"       ; break;
    case 0x08: typeText = "ROM+RAM"                ; break;
    case 0x09: typeText = "ROM+RAM+BATTERY"        ; break;
    case 0x0B: typeText = "MMM01"                  ; break;
    case 0x0C: typeText = "MMM01+RAM"              ; break;
    case 0x0D: typeText = "MMM01+RAM+BATTERY"      ; break;
    case 0x0F: typeText = "MBC3+TIMER+BATTERY"     ; break;
    case 0x10: typeText = "MBC3+TIMER+RAM+BATTERY" ; break;
    case 0x11: typeText = "MBC3"                   ; break;
    case 0x12: typeText = "MBC3+RAM"               ; break;
    case 0x13: typeText = "MBC3+RAM+BATTERY"       ; break;
    case 0x15: typeText = "MBC4"                   ; break;
    case 0x16: typeText = "MBC4+RAM"               ; break;
    case 0x17: typeText = "MBC4+RAM+BATTERY"       ; break;
    case 0x19: typeText = "MBC5"                   ; break;
    case 0x1A: typeText = "MBC5+RAM"               ; break;
    case 0x1B: typeText = "MBC5+RAM+BATTERY"       ; break;
    case 0x1C: typeText = "MBC5+RUMBLE"            ; break;
    case 0x1D: typeText = "MBC5+RUMBLE+RAM"        ; break;
    case 0x1E: typeText = "MBC5+RUMBLE+RAM+BATTERY"; break;
    case 0xFC: typeText = "POCKET CAMERA"          ; break;
    case 0xFD: typeText = "BANDAI TAMA5"           ; break;
    case 0xFE: typeText = "HuC3"                   ; break;
    case 0xFF: typeText = "HuC1+RAM+BATTERY"       ; break;
  }

  typeLabel_->setText(QString("%1 (%2)").arg(typeText).arg(int(type)));

  //---

  uchar rom = z80->getByte(0x148);

  QString romText;

  switch (rom) {
    case 0x00: romText =  "32KByte (0 banks)"  ; break;
    case 0x01: romText =  "64KByte (4 banks)"  ; break;
    case 0x02: romText = "128KByte (8 banks)"  ; break;
    case 0x03: romText = "256KByte (16 banks)" ; break;
    case 0x04: romText = "512KByte (32 banks)" ; break;
    case 0x05: romText =   "1MByte (64 banks)" ; break;
    case 0x06: romText =   "2MByte (128 banks)"; break;
    case 0x07: romText =   "4MByte (256 banks)"; break;
    case 0x52: romText = "1.1MByte (72 banks)" ; break;
    case 0x53: romText = "1.2MByte (80 banks)" ; break;
    case 0x54: romText = "1.5MByte (96 banks)" ; break;
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
