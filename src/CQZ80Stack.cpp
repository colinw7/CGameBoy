#include <CQZ80Stack.h>
#include <CZ80.h>

CQZ80Stack::
CQZ80Stack(CZ80 *z80) :
 z80_(z80)
{
  setObjectName("stackText");

  setReadOnly(true);
}

void
CQZ80Stack::
setFixedFont(const QFont &font)
{
  setFont(font);
}

void
CQZ80Stack::
update()
{
  clear();

  ushort sp = z80_->getSP();

  ushort sp1 = sp - 4;

  std::string str;

  for (ushort i = 0; i < 16; ++i) {
    ushort sp2 = sp1 + i;

    str = "";

    if (sp2 == sp)
      str += "<b><font color=\"red\">&gt;</font></b>";
    else
      str += " ";

    str += CZ80::hexString(sp2);

    str += " ";

    str += CZ80::hexString(z80_->getByte(sp2));

    append(str.c_str());
  }
}
