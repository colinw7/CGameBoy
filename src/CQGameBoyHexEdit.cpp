#include <CQGameBoyHexEdit.h>
#include <CQGameBoy.h>
#include <QLabel>
#include <CStrUtil.h>

CQGameBoyHexEdit::
CQGameBoyHexEdit(ushort value) :
 value_(value)
{
  setObjectName("edit");

  connect(this, SIGNAL(editingFinished()), this, SLOT(valueSlot()));
}

int
CQGameBoyHexEdit::
value() const
{
  uint value;

  if (! CStrUtil::decodeHexString(text().toStdString(), &value))
    return 0;

  return std::min(int(value), 0xff);
}

void
CQGameBoyHexEdit::
setValue(int value)
{
  uchar value1 = std::min(std::max(value, 0), 255);

  std::string text = CZ80::hexString(value1);

  QLineEdit::setText(text.c_str());
}

void
CQGameBoyHexEdit::
valueSlot()
{
  int value = this->value();

  emit valueChanged(value);
}

//------

CQGameBoyAddrEdit::
CQGameBoyAddrEdit(CQGameBoy *gameboy, const QString &name, ushort addr) :
 CQGameBoyHexEdit(addr), gameboy_(gameboy), name_(name)
{
  setObjectName(name);

  setToolTip(name + " : " + CZ80::hexString(addr).c_str());

  connect(this, SIGNAL(valueChanged(int)), this, SLOT(addrSlot(int)));

  update();
}

void
CQGameBoyAddrEdit::
setFont(const QFont &font)
{
  QFontMetrics fm(font);

  int w = fm.width("XXXX");

  label_->setFixedWidth(w);

  label_->setFont(font);
}

void
CQGameBoyAddrEdit::
update()
{
  CZ80 *z80 = gameboy_->getZ80();

  setValue(z80->getMemory(value_));
}

void
CQGameBoyAddrEdit::
addrSlot(int value)
{
  CZ80 *z80 = gameboy_->getZ80();

  z80->setByte(value_, value);
}
