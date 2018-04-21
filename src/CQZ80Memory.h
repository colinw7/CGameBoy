#ifndef CQZ80Memory_H
#define CQZ80Memory_H

#include <QFrame>

class CQZ80Dbg;
class QScrollBar;

#if 0
class CQZ80MemLine {
 public:
  CQZ80MemLine(ushort pc=0, const std::string &pcStr="", const std::string &memStr="",
               const std::string textStr="") :
   pc_(pc), pcStr_(pcStr), memStr_(memStr), textStr_(textStr) {
  }

  ushort pc() const { return pc_; }

  uint num() const { return pc_ / 8; }

  const std::string &pcStr  () const { return pcStr_  ; }
  const std::string &memStr () const { return memStr_ ; }
  const std::string &textStr() const { return textStr_; }

 private:
  ushort      pc_ { 0 };
  std::string pcStr_;
  std::string memStr_;
  std::string textStr_;
};

//------

class CQZ80Mem : public QFrame {
  Q_OBJECT

 public:
  CQZ80Mem(CQZ80Dbg *dbg);

  void setFont(const QFont &font);

  void updatePC();

  void updateData();

  void updateMemory(ushort pos, ushort len);

  void setMemoryLine(uint pos);

  void setLine(uint i, const std::string &pcStr, const std::string &memStr,
               const std::string &textStr);

  std::string getByteChar(uchar c);

  int vbarValue() const;
  void setVBarValue(int v);

  void resizeEvent(QResizeEvent *);

  void contextMenuEvent(QContextMenuEvent *event);

 public slots:
  void sliderSlot(int y);

  void dumpSlot();

 private:
  void paintEvent(QPaintEvent *);

  void mouseDoubleClickEvent(QMouseEvent *e);

 private:
  typedef std::vector<CQZ80MemLine> LineList;

  CQZ80Dbg*   dbg_        { nullptr };
  int         vw_         { 0 };
  QScrollBar* vbar_       { nullptr };
  LineList    lines_;
  int         yOffset_    { 0 };
  int         charWidth_  { 8 };
  int         charHeight_ { 12 };
  int         dx_         { 2 };
};
#else
#include <CQHexdump.h>

class CQZ80MemIFace;

class CQZ80Mem : public CQHexdump {
  Q_OBJECT

 public:
  CQZ80Mem(CQZ80Dbg *dbg);

 ~CQZ80Mem();

  void updatePC();

  void updateData();

  void updateMemory(ushort pos, ushort len);

 private:
  CQZ80Dbg*      dbg_   { nullptr };
  CQZ80MemIFace* iface_ { nullptr };
};
#endif

#endif
