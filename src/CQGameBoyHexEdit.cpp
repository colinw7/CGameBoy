#include <CQGameBoyHexEdit.h>
#include <CQGameBoy.h>
#include <QLabel>
#include <CStrUtil.h>

CQGameBoyHexEdit::
CQGameBoyHexEdit(ushort value) :
 value_(value)
{
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
  int value1 = std::min(std::max(value, 0), 255);

  std::string text = CStrUtil::toHexString(value1, 2);

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

  connect(this, SIGNAL(valueChanged(int)), this, SLOT(addrSlot(int)));
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
