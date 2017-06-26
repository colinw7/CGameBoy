#include <CQGameBoyInterrupt.h>
#include <CQGameBoyHexEdit.h>
#include <CQGameBoy.h>
#include <CQUtil.h>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>

#include <iostream>

CQGameBoyInterrupt::
CQGameBoyInterrupt(CQGameBoy *gameboy) :
 CZ80Trace(*gameboy->getZ80()), gameboy_(gameboy)
{
  setObjectName("interrupt");

  setWindowTitle("GameBoy Interrupt");

  QVBoxLayout *layout = new QVBoxLayout(this);

  QHBoxLayout *enabledLayout = new QHBoxLayout;

  enableCheck_ = new QCheckBox("Enabled");

  connect(enableCheck_, SIGNAL(stateChanged(int)), this, SLOT(enableSlot()));

  enabledLayout->addWidget(enableCheck_);
  enabledLayout->addStretch(1);

  layout->addLayout(enabledLayout);

  QHBoxLayout *regLayout = new QHBoxLayout;

  iffEdit_  = new CQGameBoyHexEdit;
  statEdit_ = new CQGameBoyHexEdit;

  regLayout->addWidget(new QLabel("IFF"));
  regLayout->addWidget(iffEdit_);
  regLayout->addWidget(new QLabel("STAT"));
  regLayout->addWidget(statEdit_);

  layout->addLayout(regLayout);

  canvas_ = new CQGameBoyInterruptCanvas(this);

  layout->addWidget(canvas_);

  CZ80 *z80 = gameboy->getZ80();

  z80->addTrace(this);

  updateState();
}

void
CQGameBoyInterrupt::
regChanged(CZ80Reg reg)
{
  if (reg == CZ80Reg::IFF)
    updateState();
}

void
CQGameBoyInterrupt::
memChanged(ushort pos, ushort)
{
  if (pos == 0xff0f || pos == 0xff41 || pos == 0xffff)
    updateState();
}

void
CQGameBoyInterrupt::
updateState()
{
  CZ80 *z80 = gameboy()->getZ80();

  enableCheck_->setChecked(z80->getAllowInterrupts());

  iffEdit_ ->setValue(z80->getIFF());
  statEdit_->setValue(z80->getMemory(0xff41));

  canvas_->update();
}

void
CQGameBoyInterrupt::
enableSlot()
{
  CZ80 *z80 = gameboy()->getZ80();

  z80->setAllowInterrupts(enableCheck_->isChecked());
}

//------

CQGameBoyInterruptCanvas::
CQGameBoyInterruptCanvas(CQGameBoyInterrupt *interrupt) :
 interrupt_(interrupt)
{
  setObjectName("canvas");
}

CQGameBoyInterruptCanvas::
~CQGameBoyInterruptCanvas()
{
}

void
CQGameBoyInterruptCanvas::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  QFontMetrics fm(font());

  uchar interruptFlag   = gameboy()->interruptFlag();
  uchar interruptEnable = gameboy()->interruptEnable();

  painter.fillRect(rect(), Qt::white);

  int border = 4;

  int bx = fm.ascent();
  int by = fm.ascent();
  int dy = fm.height() + 2;

  int cw1 = fm.width("Serial I/O");
  int cw2 = fm.width("Flag");
  int cw3 = fm.width("Enable");

  int x, y;

  painter.setPen(Qt::black);

  x = cw1 + 2*border + (cw3 - cw2)/2;
  y = dy;

  painter.drawText(x, y, "Flag"  );

  x = cw1 + 3*border + cw3;

  painter.drawText(x, y, "Enable");

  x = border;
  y = dy + fm.ascent() + border;

  painter.drawText(x, y, "Key"       ); y += dy;
  painter.drawText(x, y, "Serial I/O"); y += dy;
  painter.drawText(x, y, "Timer"     ); y += dy;
  painter.drawText(x, y, "LCDC"      ); y += dy;
  painter.drawText(x, y, "V-Blank"   ); y += dy;

  x = cw1 + 2*border + (cw3 - bx)/2;
  y = dy + 2*border;

  drawBox(&painter, QRect(x, y, bx, by), TST_BIT(interruptFlag, 4)); y += dy;
  drawBox(&painter, QRect(x, y, bx, by), TST_BIT(interruptFlag, 3)); y += dy;
  drawBox(&painter, QRect(x, y, bx, by), TST_BIT(interruptFlag, 2)); y += dy;
  drawBox(&painter, QRect(x, y, bx, by), TST_BIT(interruptFlag, 1)); y += dy;
  drawBox(&painter, QRect(x, y, bx, by), TST_BIT(interruptFlag, 0)); y += dy;

  x = cw1 + 3*border + cw3 + (cw3 - bx)/2;
  y = dy + 2*border;

  drawBox(&painter, QRect(x, y, bx, by), TST_BIT(interruptEnable, 4)); y += dy;
  drawBox(&painter, QRect(x, y, bx, by), TST_BIT(interruptEnable, 3)); y += dy;
  drawBox(&painter, QRect(x, y, bx, by), TST_BIT(interruptEnable, 2)); y += dy;
  drawBox(&painter, QRect(x, y, bx, by), TST_BIT(interruptEnable, 1)); y += dy;
  drawBox(&painter, QRect(x, y, bx, by), TST_BIT(interruptEnable, 0)); y += dy;
}

void
CQGameBoyInterruptCanvas::
drawBox(QPainter *painter, const QRect &rect, bool on)
{
  painter->fillRect(rect, (on ? Qt::green : Qt::red));
}

QSize
CQGameBoyInterruptCanvas::
sizeHint() const
{
  return QSize(256, 256);
}
