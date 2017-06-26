#include <CQGameBoyTimer.h>
#include <CQGameBoyHexEdit.h>
#include <CQGameBoy.h>
#include <CQUtil.h>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>

#include <iostream>

CQGameBoyTimer::
CQGameBoyTimer(CQGameBoy *gameboy) :
 CZ80Trace(*gameboy->getZ80()), gameboy_(gameboy)
{
  setObjectName("timer");

  setWindowTitle("GameBoy Timer");

  QVBoxLayout *layout = new QVBoxLayout(this);

  QHBoxLayout *hlayout1 = new QHBoxLayout;
  QHBoxLayout *hlayout2 = new QHBoxLayout;
  QHBoxLayout *hlayout3 = new QHBoxLayout;
  QHBoxLayout *hlayout4 = new QHBoxLayout;

  layout->addLayout(hlayout1);
  layout->addLayout(hlayout2);
  layout->addLayout(hlayout3);
  layout->addLayout(hlayout4);

  dividerEdit_ = new CQGameBoyAddrEdit(gameboy, "divider", 0xff04);
  counterEdit_ = new CQGameBoyAddrEdit(gameboy, "counter", 0xff05);
  moduloEdit_  = new CQGameBoyAddrEdit(gameboy, "modulo" , 0xff06);
  controlEdit_ = new CQGameBoyAddrEdit(gameboy, "control", 0xff07);

  hlayout1->addWidget(new QLabel("Divider"));
  hlayout1->addWidget(dividerEdit_);

  hlayout2->addWidget(new QLabel("Counter"));
  hlayout2->addWidget(counterEdit_);

  hlayout3->addWidget(new QLabel("Modulo"));
  hlayout3->addWidget(moduloEdit_);

  hlayout4->addWidget(new QLabel("Control"));
  hlayout4->addWidget(controlEdit_);

  CZ80 *z80 = gameboy->getZ80();

  z80->addTrace(this);

  updateState();
}

void
CQGameBoyTimer::
memChanged(ushort pos, ushort)
{
  if (pos >= 0xff04 && pos <= 0xff07)
    updateState();
}

void
CQGameBoyTimer::
updateState()
{
  dividerEdit_->update();
  counterEdit_->update();
  moduloEdit_ ->update();
  controlEdit_->update();
}
