#include <CQGameBoyVReg.h>
#include <CQGameBoyVideo.h>
#include <CQGameBoyScreen.h>
#include <CQGameBoy.h>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>

CQGameBoyVReg::
CQGameBoyVReg(CQGameBoyVideo *video) :
 video_(video)
{
  CQGameBoy *gameboy = video->screen()->gameboy();

  //---

  setObjectName("vreg");

  QVBoxLayout *layout = new QVBoxLayout(this);

  traceCheck_ = new QCheckBox("Trace");

  traceCheck_->setChecked(isTrace());

  connect(traceCheck_, SIGNAL(stateChanged(int)), this, SLOT(traceSlot()));

  layout->addWidget(traceCheck_);

  //---

  QFrame *registersFrame = new QFrame;

  layout->addWidget(registersFrame);

  QVBoxLayout *registersLayout = new QVBoxLayout(registersFrame);

  registersLayout->addWidget(addRegisterWidget("LCDC", 0xff40));
  registersLayout->addWidget(addRegisterWidget("STAT", 0xff41));
  registersLayout->addWidget(addRegisterWidget("SCY" , 0xff42));
  registersLayout->addWidget(addRegisterWidget("SCX" , 0xff43));
  registersLayout->addWidget(addRegisterWidget("LY"  , 0xff44));
  registersLayout->addWidget(addRegisterWidget("LYC" , 0xff45));
  registersLayout->addWidget(addRegisterWidget("DMA" , 0xff46));
  registersLayout->addWidget(addRegisterWidget("WY"  , 0xff4a));
  registersLayout->addWidget(addRegisterWidget("WX"  , 0xff4b));

  if (! gameboy->isGBC()) {
    registersLayout->addWidget(addRegisterWidget("BGP" , 0xff47));
    registersLayout->addWidget(addRegisterWidget("OBP0", 0xff48));
    registersLayout->addWidget(addRegisterWidget("OBP1", 0xff49));
  }
  else {
    registersLayout->addWidget(addRegisterWidget("VBK"      , 0xff4f));
    registersLayout->addWidget(addRegisterWidget("BCPS/BGPI", 0xff68));
    registersLayout->addWidget(addRegisterWidget("BCPD/BGPD", 0xff69));
    registersLayout->addWidget(addRegisterWidget("OCPS/OBPI", 0xff6a));
    registersLayout->addWidget(addRegisterWidget("OCPD/OBPD", 0xff6b));
  }

  //---

  layout->addStretch(1);
}

QFrame *
CQGameBoyVReg::
addRegisterWidget(const QString &name, ushort addr)
{
  QFrame *frame = new QFrame;

  QHBoxLayout *layout = new QHBoxLayout(frame);
  layout->setMargin(0); layout->setSpacing(0);

  QLabel *label = new QLabel(name);

  layout->addWidget(label);

  CQGameBoyVideoRegEdit *edit = new CQGameBoyVideoRegEdit(this, name, addr);

  edit->setText ("00" );
  edit->setLabel(label);

  layout->addWidget(edit);

  nameEdits_[name] = edit;
  addrEdits_[addr] = edit;

  return frame;
}

void
CQGameBoyVReg::
setFixedFont(const QFont &font)
{
  for (const auto &ne : nameEdits_) {
    ne.second->setFont(font);
  }
}

void
CQGameBoyVReg::
traceSlot()
{
  setTrace(traceCheck_->isChecked());

  if (isTrace())
    updateAll();
}

void
CQGameBoyVReg::
updateMem(ushort pos, ushort len)
{
  if (! isTrace())
    return;

  for (int i = 0; i < len; ++i) {
    if (pos < 0xff40)
      continue;

    auto p = addrEdits_.find(pos + i);

    if (p == addrEdits_.end())
      continue;

    (*p).second->update();
  }
}

void
CQGameBoyVReg::
updateAll()
{
  for (const auto &ne : nameEdits_) {
    ne.second->update();
  }
}

//------

CQGameBoyVideoRegEdit::
CQGameBoyVideoRegEdit(CQGameBoyVReg *vreg, const QString &name, ushort addr) :
 CQGameBoyAddrEdit(vreg->video()->screen()->gameboy(), name, addr), vreg_(vreg)
{
}
