#include <CQZ80Instructions.h>
#include <CQZ80Dbg.h>
#include <CZ80OpData.h>
#include <QMenu>
#include <QScrollBar>
#include <QContextMenuEvent>
#include <QPainter>
#include <QStyle>

CQZ80Inst::
CQZ80Inst(CQZ80Dbg *dbg) :
 QFrame(nullptr), dbg_(dbg)
{
  setObjectName("inst");

  vw_ = style()->pixelMetric(QStyle::PM_ScrollBarExtent);

  setContentsMargins(0, 0, vw_, 0);

  //--

  vbar_ = new QScrollBar(Qt::Vertical, this);

  vbar_->setObjectName("instructionsVbar");
  vbar_->setPageStep  (dbg->getNumMemoryLines());
  vbar_->setSingleStep(1);
  vbar_->setRange     (0, 8192 - vbar_->pageStep());

  connect(vbar_, SIGNAL(valueChanged(int)), this, SLOT(sliderSlot(int)));

  //--

  lines_.resize(65536);
}

void
CQZ80Inst::
setFont(const QFont &font)
{
  QWidget::setFont(font);

  QFontMetrics fm(font);

  int instructionsWidth = fm.horizontalAdvance("0000  123456789012  AAAAAAAAAAAAAAAAAAAAAA");
  int charHeight        = fm.height();

  setFixedWidth (instructionsWidth + 32);
  setFixedHeight(charHeight*dbg_->getNumMemoryLines());
}

void
CQZ80Inst::
clear()
{
  lineNum_ = 0;

  pcLineMap_.clear();
  linePcMap_.clear();
}

void
CQZ80Inst::
setLine(uint pc, const std::string &pcStr, const std::string &codeStr, const std::string &textStr)
{
  assert(lineNum_ < int(lines_.size()));

  lines_[lineNum_] = CQZ80InstLine(pc, pcStr, codeStr, textStr);

  pcLineMap_[pc      ] = lineNum_;
  linePcMap_[lineNum_] = pc;

  ++lineNum_;
}

bool
CQZ80Inst::
getLineForPC(uint pc, uint &lineNum) const
{
  auto p = pcLineMap_.find(pc);

  if (p == pcLineMap_.end())
    return false;

  lineNum = (*p).second;

  return true;
}

uint
CQZ80Inst::
getPCForLine(uint lineNum)
{
  return linePcMap_[lineNum];
}

int
CQZ80Inst::
vbarValue() const
{
  return vbar_->value();
}

void
CQZ80Inst::
setVBarValue(int v)
{
  vbar_->setValue(v);
}

void
CQZ80Inst::
resizeEvent(QResizeEvent *)
{
  vbar_->move  (width() - vw_, 0);
  vbar_->resize(vw_, height());
}

void
CQZ80Inst::
contextMenuEvent(QContextMenuEvent *event)
{
  QMenu *menu = new QMenu;

  QAction *dumpAction = menu->addAction("Dump");

  connect(dumpAction, SIGNAL(triggered()), this, SLOT(dumpSlot()));

  QAction *reloadAction = menu->addAction("Reload");

  connect(reloadAction, SIGNAL(triggered()), this, SLOT(reloadSlot()));

  menu->exec(event->globalPos());

  delete menu;
}

void
CQZ80Inst::
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

  int charWidth  = fm.horizontalAdvance(" ");
  int charAscent = fm.ascent();

  int w1 =  4*charWidth;
  int w2 = 12*charWidth;

  int y = -yOffset_*charHeight_ + charAscent;

  int ymin = -charHeight_;
  int ymax = height() + charHeight_;

  if (! isEnabled())
    p.setPen(palette().color(QPalette::Disabled, QPalette::WindowText));

  for (const auto &line : lines_) {
    if (y >= ymin && y <= ymax) {
      int x = 2;

      if (line.pc() == pc) {
        if (isEnabled())
          p.setPen(dbg_->currentColor());

        p.drawText(x, y, ">");
      }

      x += charWidth;

      if (isEnabled())
        p.setPen(dbg_->addrColor());

      p.drawText(x, y, line.pcStr().c_str());

      x += w1 + charWidth;

      if (isEnabled())
        p.setPen(dbg_->memDataColor());

      p.drawText(x, y, line.codeStr().c_str());

      x += w2 + charWidth;

      if (isEnabled())
        p.setPen(dbg_->memCharsColor());

      p.drawText(x, y, line.textStr().c_str());
    }

    y += charHeight_;
  }
}

void
CQZ80Inst::
mouseDoubleClickEvent(QMouseEvent *e)
{
  int iy = (e->pos().y() + yOffset_*charHeight_)/charHeight_;

  CZ80 *z80 = dbg_->getZ80();

  z80->setPC(getPCForLine(iy));

  z80->callRegChanged(CZ80Reg::PC);
}

void
CQZ80Inst::
sliderSlot(int y)
{
  yOffset_ = y;

  update();
}

void
CQZ80Inst::
dumpSlot()
{
  FILE *fp = fopen("inst.txt", "w");
  if (! fp) return;

  for (const auto &line : lines_) {
    fprintf(fp, "%s %s %s\n", line.pcStr().c_str(), line.codeStr().c_str(),
            line.textStr().c_str());
  }

  fclose(fp);
}

void
CQZ80Inst::
reloadSlot()
{
  reload();

  CZ80 *z80 = dbg_->getZ80();

  uint lineNum;

  if (getLineForPC(z80->getPC(), lineNum))
    vbar_->setValue(lineNum);

}

void
CQZ80Inst::
reload()
{
  CZ80 *z80 = dbg_->getZ80();

  uint pos1 = 0;
  uint pos2 = 65536;

  clear();

  uint pc      = pos1;
  bool pcFound = false;

  while (pc < pos2) {
    // resync to PC (should be legal instruction here)
    if (! pcFound && pc >= z80->getPC()) {
      pc      = z80->getPC();
      pcFound = true;
    }

    //-----

    std::string pcStr = CStrUtil::toHexString(pc, 4);

    //-----

    CZ80OpData opData;

    z80->readOpData(pc, &opData);

    uint pc1 = pc + opData.op->len;

    //-----

    std::string codeStr;

    ushort len1 = 0;

    for (uint i = pc; i < pc1; ++i) {
      if (i > pc) codeStr += " ";

      codeStr += CStrUtil::toHexString(z80->getByte(i), 2);

      len1 += 3;
    }

    for ( ; len1 < 12; ++len1)
      codeStr += " ";

    //-----

    std::string textStr = "; ";

    if (opData.op)
      textStr += opData.getOpString(pc1);
    else
      textStr += "??";

    setLine(pc, pcStr, codeStr, textStr);

    //------

    pc = pc1;
  }

  uint numLines = getNumLines();

  vbar_->setRange(0, numLines - vbar_->pageStep());

  vbar_->setValue(0);

  update();
}
