#include <CGameBoyPortData.h>
#include <CGameBoy.h>

CGameBoyPortData::
CGameBoyPortData(CGameBoy *gameboy) :
 CZ80PortData(*gameboy->getZ80()), gameboy_(gameboy)
{
}

void
CGameBoyPortData::
out(uchar port, uchar value)
{
  std::cerr << "Out: Port " << (int) port << " Value " << (int) value << std::endl;
}

uchar
CGameBoyPortData::
in(uchar port, uchar value)
{
  std::cerr << "In: Port " << (int) port << " Value " << (int) value << std::endl;

  return 0;
}

void
CGameBoyPortData::
keyPress(const CKeyEvent &e)
{
  CKeyType type = CEvent::keyTypeToUpper(e.getType());

  gameboy_->keyPress(type);
}

void
CGameBoyPortData::
keyRelease(const CKeyEvent &e)
{
  CKeyType type = CEvent::keyTypeToUpper(e.getType());

  gameboy_->keyRelease(type);
}
