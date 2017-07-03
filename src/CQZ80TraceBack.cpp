#include <CQZ80TraceBack.h>
#include <CZ80.h>

CQZ80TraceBack::
CQZ80TraceBack(CZ80 *z80) :
 z80_(z80)
{
  setObjectName("traceBack");

  setReadOnly(true);
}

void
CQZ80TraceBack::
setFixedFont(const QFont &font)
{
  setFont(font);
}

void
CQZ80TraceBack::
update()
{
  clear();

  for (int i = 0; i < z80_->traceNum(); ++i) {
    ushort v = z80_->traceValue(i);

    std::string str = CZ80::hexString(v);

    append(str.c_str());
  }
}
