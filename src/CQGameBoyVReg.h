#ifndef CQGameBoyVReg_H
#define CQGameBoyVReg_H

#include <CQGameBoyHexEdit.h>
#include <QFrame>

class CQGameBoyVideo;
class CQGameBoyVReg;
class QCheckBox;

class CQGameBoyVideoRegEdit : public CQGameBoyAddrEdit {
  Q_OBJECT

 public:
  CQGameBoyVideoRegEdit(CQGameBoyVReg *video, const QString &name, ushort addr);

  CQGameBoyVReg *vreg() const { return vreg_; }

 private:
  CQGameBoyVReg* vreg_ { nullptr };
};

//---

class CQGameBoyVReg : public QFrame {
  Q_OBJECT

 public:
  CQGameBoyVReg(CQGameBoyVideo *video);

  CQGameBoyVideo *video() const { return video_; }

  bool isTrace() const { return trace_; }
  void setTrace(bool b) { trace_ = b; }

  void setFixedFont(const QFont &font);

  void updateMem(ushort pos, ushort len);

  void updateAll();

 private slots:
  void traceSlot();

 private:
  QFrame *addRegisterWidget(const QString &label, ushort addr);

 private:
  typedef std::map<QString,CQGameBoyVideoRegEdit*> NameRegisterEdits;
  typedef std::map<ushort,CQGameBoyVideoRegEdit*>  AddrRegisterEdits;

  CQGameBoyVideo*   video_      { nullptr };
  NameRegisterEdits nameEdits_;
  AddrRegisterEdits addrEdits_;
  QCheckBox*        traceCheck_ { nullptr };
  bool              trace_      { true };
};

#endif
