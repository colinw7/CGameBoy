#include <CQZ80RegEdit.h>
#include <CZ80.h>
#include <CStrUtil.h>

#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>

CQZ80RegEdit::
CQZ80RegEdit(CZ80 *z80, CZ80Reg reg) :
 QFrame(nullptr), z80_(z80), reg_(reg)
{
  QHBoxLayout *layout = new QHBoxLayout(this);

  layout->setMargin(2); layout->setMargin(2);

  QString str;

  switch (reg) {
    case CZ80Reg::AF : str = "AF" ; break;
    case CZ80Reg::AF1: str = "AF'"; break;
    case CZ80Reg::BC : str = "BC" ; break;
    case CZ80Reg::BC1: str = "BC'"; break;
    case CZ80Reg::DE : str = "DE" ; break;
    case CZ80Reg::DE1: str = "DE'"; break;
    case CZ80Reg::HL : str = "HL" ; break;
    case CZ80Reg::HL1: str = "HL'"; break;
    case CZ80Reg::IX : str = "IX" ; break;
    case CZ80Reg::I  : str = "I"  ; break;
    case CZ80Reg::IY : str = "IY" ; break;
    case CZ80Reg::R  : str = "R"  ; break;
    case CZ80Reg::SP : str = "SP" ; break;
    case CZ80Reg::PC : str = "PC" ; break;
    case CZ80Reg::IFF: str = "IFF"; break;
    default          : assert(false);
  }

  setObjectName(str);

  label_ = new QLabel(str);

  label_->setObjectName("label");

  edit_ = new QLineEdit;

  edit_->setObjectName("edit");

  layout->addWidget(label_);
  layout->addWidget(edit_);

  connect(edit_, SIGNAL(returnPressed()), this, SLOT(valueChangedSlot()));
}

void
CQZ80RegEdit::
setFont(const QFont &font)
{
  QWidget::setFont(font);

  label_->setFont(font);
  edit_ ->setFont(font);

  QFontMetrics fm(font);

  label_->setFixedWidth(fm.width("XXX") + 4);
  edit_ ->setFixedWidth(fm.width("0000") + 16);
}

void
CQZ80RegEdit::
setValue(uint value)
{
  int len = 4;

  if (reg_ == CZ80Reg::I || reg_ == CZ80Reg::R || reg_ == CZ80Reg::IFF) len = 2;

  edit_->setText(CStrUtil::toHexString(value, len).c_str());
}

void
CQZ80RegEdit::
valueChangedSlot()
{
  uint value;

  if (! CStrUtil::decodeHexString(edit_->text().toStdString(), &value))
    return;

  switch (reg_) {
    case CZ80Reg::AF : z80_->setAF (value); break;
    case CZ80Reg::AF1: z80_->setAF1(value); break;
    case CZ80Reg::BC : z80_->setBC (value); break;
    case CZ80Reg::BC1: z80_->setBC1(value); break;
    case CZ80Reg::DE : z80_->setDE (value); break;
    case CZ80Reg::DE1: z80_->setDE1(value); break;
    case CZ80Reg::HL : z80_->setHL (value); break;
    case CZ80Reg::HL1: z80_->setHL1(value); break;
    case CZ80Reg::IX : z80_->setIX (value); break;
    case CZ80Reg::I  : z80_->setI  (value); break;
    case CZ80Reg::IY : z80_->setIY (value); break;
    case CZ80Reg::R  : z80_->setR  (value); break;
    case CZ80Reg::SP : z80_->setSP (value); break;
    case CZ80Reg::PC : z80_->setPC (value); break;
    case CZ80Reg::IFF: z80_->setIFF(value); break;
    default          : assert(false);
  }

  z80_->callRegChanged(reg_);
}
