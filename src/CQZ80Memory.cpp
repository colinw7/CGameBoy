#include <CQZ80Memory.h>
#include <CQZ80Dbg.h>
#include <QMenu>
#include <QScrollBar>
#include <QContextMenuEvent>
#include <QPainter>
#include <QStyle>

#if 0
CQZ80Mem::
CQZ80Mem(CQZ80Dbg *dbg) :
 QFrame(nullptr), dbg_(dbg)
{
  setObjectName("mem");

  vw_ = style()->pixelMetric(QStyle::PM_ScrollBarExtent);

  setContentsMargins(0, 0, vw_, 0);

  //--

  vbar_ = new QScrollBar(Qt::Vertical, this);

  vbar_->setObjectName("memoryVbar");
  vbar_->setPageStep  (dbg->getNumMemoryLines());
  vbar_->setSingleStep(1);
  vbar_->setRange     (0, 8192 - vbar_->pageStep());

  connect(vbar_, SIGNAL(valueChanged(int)), this, SLOT(sliderSlot(int)));

  //--

  lines_.resize(8192);
}

void
CQZ80Mem::
setFont(const QFont &font)
{
  QWidget::setFont(font);

  QFontMetrics fm(font);

  int memoryWidth = fm.horizontalAdvance("0000  00 00 00 00 00 00 00 00  XXXXXXXX");
  int charHeight  = fm.height();

  setFixedWidth (memoryWidth + 32);
  setFixedHeight(charHeight*dbg_->getNumMemoryLines());
}

void
CQZ80Mem::
updatePC()
{
  CZ80 *z80 = dbg_->getZ80();

  ushort pc = z80->getPC();

  int mem1 = vbarValue();
  int mem2 = mem1 + 20;
  int mem  = pc / 8;

  if (mem < mem1 || mem > mem2) {
    setVBarValue(mem);
  }
  else {
    update();
  }
}

void
CQZ80Mem::
updateData()
{
  //CZ80 *z80 = dbg_->getZ80();

  uint len = 65536;

  ushort numLines = len / 8;

  if ((len % 8) != 0) ++numLines;

  //uint pos = z80->getPC();

  //z80_->setPC(0);

  std::string str;

  uint pos1 = 0;

  for (ushort i = 0; i < numLines; ++i) {
    setMemoryLine(pos1);

    pos1 += 8;
  }

  //z80_->setPC(pos);
}

void
CQZ80Mem::
updateMemory(ushort pos, ushort len)
{
  ushort pos1 = pos;
  ushort pos2 = pos + len;

  uint lineNum1 = pos1/8;
  uint lineNum2 = pos2/8;

  for (uint lineNum = lineNum1; lineNum <= lineNum2; ++lineNum)
    setMemoryLine(8*lineNum);

  update();
}

void
CQZ80Mem::
setMemoryLine(uint pos)
{
  CZ80 *z80 = dbg_->getZ80();

  std::string pcStr = CStrUtil::toHexString(pos, 4);

  //-----

  std::string memStr;

  for (ushort j = 0; j < 8; ++j) {
    if (j > 0) memStr += " ";

    memStr += CStrUtil::toHexString(z80->getByte(pos + j), 2);
  }

  std::string textStr;

  for (ushort j = 0; j < 8; ++j) {
    uchar c = z80->getByte(pos + j);

    textStr += getByteChar(c);
  }

  setLine(pos, pcStr, memStr, textStr);
}

void
CQZ80Mem::
setLine(uint pc, const std::string &pcStr, const std::string &memStr, const std::string &textStr)
{
  uint lineNum = pc / 8;

  assert(lineNum < lines_.size());

  lines_[lineNum] = CQZ80MemLine(pc, pcStr, memStr, textStr);
}

std::string
CQZ80Mem::
getByteChar(uchar c)
{
  std::string str;

  if (c >= 0x20 && c < 0x7f)
    str += c;
  else
    str += '.';

  return str;
}

int
CQZ80Mem::
vbarValue() const
{
  return vbar_->value();
}

void
CQZ80Mem::
setVBarValue(int v)
{
  vbar_->setValue(v);
}

void
CQZ80Mem::
resizeEvent(QResizeEvent *)
{
  vbar_->move  (width() - vw_, 0);
  vbar_->resize(vw_, height());
}

void
CQZ80Mem::
contextMenuEvent(QContextMenuEvent *event)
{
  QMenu *menu = new QMenu;

  QAction *action = menu->addAction("Dump");

  connect(action, SIGNAL(triggered()), this, SLOT(dumpSlot()));

  menu->exec(event->globalPos());

  delete menu;
}

void
CQZ80Mem::
paintEvent(QPaintEvent *)
{
  CZ80 *z80 = dbg_->getZ80();

  uint pc = z80->getPC();

  QPainter p(this);

  if (isEnabled())
    p.fillRect(rect(), Qt::white);
  else
    p.fillRect(rect(), palette().window().color());

  QFontMetrics fm(font());

  charHeight_ = fm.height();
  charWidth_  = fm.horizontalAdvance(" ");

  int charAscent = fm.ascent();

  int w1 =  4*charWidth_; // address (4 digits)
  int w2 =    charWidth_; // spacer (1 char)
  int w3 = 23*charWidth_; // data (16 digits + 7 spaces)
  int w4 =    charWidth_; // spacer (1 char)

  int y  = -yOffset_*charHeight_;
  int ya = y + charAscent;

  int ymin = -charHeight_;
  int ymax = height() + charHeight_;

  if (! isEnabled())
    p.setPen(palette().color(QPalette::Disabled, QPalette::WindowText));

  for (const auto &line : lines_) {
    if (y >= ymin && y <= ymax) {
      int x = dx_;

      uint pc1 = line.pc();
      uint pc2 = pc1 + 8;

      if (isEnabled()) {
        if      (z80->isReadOnly(pc1, 8))
          p.fillRect(QRect(x + w1 + w2, y, w3, charHeight_), dbg_->readOnlyBgColor());
        else if (z80->isScreen(pc1, 8))
          p.fillRect(QRect(x + w1 + w2, y, w3, charHeight_), dbg_->screenBgColor());
      }

      if (isEnabled())
        p.setPen(dbg_->addrColor());

      p.drawText(x, ya, line.pcStr().c_str());

      x += w1 + w2;

      if (isEnabled())
        p.setPen(dbg_->memDataColor());

      if (pc >= pc1 && pc < pc2) {
        int i1 = 3*(pc - pc1);
        int i2 = i1 + 2;

        std::string lhs = line.memStr().substr(0, i1);
        std::string mid = line.memStr().substr(i1, 2);
        std::string rhs = line.memStr().substr(i2);

        int w1 = fm.horizontalAdvance(lhs.c_str());
        int w2 = fm.horizontalAdvance(mid.c_str());

        p.drawText(x          , ya, lhs.c_str());
        p.drawText(x + w1 + w2, ya, rhs.c_str());

        if (isEnabled())
          p.setPen(dbg_->currentColor());

        p.drawText(x + w1, ya, mid.c_str());
      }
      else {
        p.drawText(x, ya, line.memStr().c_str());
      }

      x += w3 + w4;

      if (isEnabled())
        p.setPen(dbg_->memCharsColor());

      p.drawText(x, ya, line.textStr().c_str());
    }

    y  += charHeight_;
    ya += charHeight_;
  }
}

void
CQZ80Mem::
mouseDoubleClickEvent(QMouseEvent *e)
{
  int ix = (e->pos().x() - dx_                 )/charWidth_ ;
  int iy = (e->pos().y() + yOffset_*charHeight_)/charHeight_;

  if (ix < 4 || ix >= 28  ) return;
  if (iy < 0 || iy >= 8192) return;

  uint pc = int((ix - 4)/3) + iy*8;

  CZ80 *z80 = dbg_->getZ80();

  z80->setPC(pc);

  z80->callRegChanged(CZ80Reg::PC);
}

void
CQZ80Mem::
sliderSlot(int y)
{
  yOffset_ = y;

  update();
}

void
CQZ80Mem::
dumpSlot()
{
  FILE *fp = fopen("memory.txt", "w");
  if (! fp) return;

  for (const auto &line : lines_) {
    fprintf(fp, "%s %s %s\n", line.pcStr().c_str(), line.memStr().c_str(),
            line.textStr().c_str());
  }

  fclose(fp);
}
#else
class CQZ80MemIFace : public CQHexdumpDataIFace {
 public:
  CQZ80MemIFace(CZ80 *z80) :
   z80_(z80) {
  }

  std::size_t size() const override { return 65536; }

  uchar data(uint i) const override { return z80_->getByte(i); }

 public:
  CZ80* z80_ { nullptr };
};

CQZ80Mem::
CQZ80Mem(CQZ80Dbg *dbg) :
 CQHexdump(nullptr), dbg_(dbg)
{
  setObjectName("mem");

  setShowAddress(true);
  setWidth(8);

  CZ80 *z80 = dbg_->getZ80();

  iface_ = new CQZ80MemIFace(z80);

  setData(iface_);
}

CQZ80Mem::
~CQZ80Mem()
{
  delete iface_;
}

void
CQZ80Mem::
updatePC()
{
  CZ80 *z80 = dbg_->getZ80();

  ushort pc = z80->getPC();

  setPosition(pc);

  // scrollToPosition();
}

void
CQZ80Mem::
updateData()
{
  CZ80 *z80 = dbg_->getZ80();

  const CZ80::MemFlagsArray &memFlagsArray = z80->memFlagsArray();

  for (const auto &memFlags : memFlagsArray) {
    if      (memFlags.flags() & int(CZ80MemType::READ_ONLY))
      addMemColor(memFlags.pos(), memFlags.len(), dbg_->readOnlyBgColor());
    else if (memFlags.flags() & int(CZ80MemType::SCREEN))
      addMemColor(memFlags.pos(), memFlags.len(), dbg_->screenBgColor());
  }

  update();
}

void
CQZ80Mem::
updateMemory(ushort /*pos*/, ushort /*len*/)
{
  updateData();
}
#endif
