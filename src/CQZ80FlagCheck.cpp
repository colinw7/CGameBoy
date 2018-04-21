#include <CQZ80FlagCheck.h>

CQZ80FlagCheck::
CQZ80FlagCheck(CZ80 *z80, CZ80Flag flag) :
 z80_(z80), flag_(flag)
{
  setObjectName(flagName(flag_).toLower() + "FlagCheck");
}

QString
CQZ80FlagCheck::
flagName() const
{
  return flagName(flag_);
}

QString
CQZ80FlagCheck::
flagName(CZ80Flag flag)
{
  switch (flag) {
    case CZ80Flag::S: return "S";
    case CZ80Flag::Z: return "Z";
    case CZ80Flag::Y: return "Y";
    case CZ80Flag::H: return "H";
    case CZ80Flag::X: return "X";
    case CZ80Flag::P: return "P";
    case CZ80Flag::N: return "N";
    case CZ80Flag::C: return "C";
    default:          return "";
  }
}

void
CQZ80FlagCheck::
updateState()
{
  switch (flag_) {
    case CZ80Flag::S: setChecked(z80_->tstSFlag()); break;
    case CZ80Flag::Z: setChecked(z80_->tstZFlag()); break;
    case CZ80Flag::Y: setChecked(z80_->tstYFlag()); break;
    case CZ80Flag::H: setChecked(z80_->tstHFlag()); break;
    case CZ80Flag::X: setChecked(z80_->tstXFlag()); break;
    case CZ80Flag::P: setChecked(z80_->tstPFlag()); break;
    case CZ80Flag::N: setChecked(z80_->tstNFlag()); break;
    case CZ80Flag::C: setChecked(z80_->tstCFlag()); break;
    default:                                        break;
  }
}
