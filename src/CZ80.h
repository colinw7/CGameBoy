#ifndef CZ80_H
#define CZ80_H

#include <map>
#include <string>
#include <vector>
#include <list>
#include <cstring>
#include <cassert>
#include <iostream>

//-----------------

#include <CBits.h>
#include <CStrUtil.h>

//-----------------

#include <CZ80Types.h>

// Flags
enum class CZ80Flag {
  NONE=-1,

#ifdef GAMEBOY_Z80
  // 76543210
  // ZSHC----
  S=0,      // Not used (Always zero)
  P=1, V=1, // Not used (Always zero)
  X=2,      // Not used (Always zero)
  Y=3,      // Not used (Always zero)
  C=4,      // Carry
  H=5,      // Half Carry
  N=6,      // Last Op was Subtract
  Z=7,      // Zero
#else
  // 75643210
  // SZYHXPNC
  C=0,      // Carry
  N=1,      // Last Op was Subtract
  P=2, V=2, // Parity
  X=3,      // Unused ?
  H=4,      // Half Carry
  Y=5,      // Unused ?
  Z=6,      // Zero
  S=7,      // Sign
#endif

  INV=(1<<30),

  NC=(C|INV),
  NN=(N|INV),
  NP=(P|INV),
  NV=(V|INV),
  NX=(X|INV),
  NH=(H|INV),
  NY=(Y|INV),
  NZ=(Z|INV),
  NS=(S|INV)
};

//------

class  CZ80;
struct CZ80Op;
struct CZ80OpData;

class CZ80MemData;
class CZ80ExecData;
class CZ80PortData;
class CZ80Trace;
class CZ80Screen;
class CZ80RstData;
class CZ80DebugData;
class CZ80SpeedData;

class CFile;
class CFileBase;
class CFileParse;
class CKeyEvent;

typedef void (CZ80OpProc)(CZ80OpData *opData);

typedef CZ80OpProc *CZ80OpProcP;

//------

#include <CZ80Registers.h>
#include <CZ80AssembleData.h>
#include <CZ80LabelData.h>
#include <CZ80Macro.h>
#include <CZ80Reg.h>

//----------

enum class CZ80MemType {
  READ_WRITE   =0,
  READ_ONLY    =(1<<0),
  SCREEN       =(1<<1),
  WRITE_TRIGGER=(1<<2)
};

//----------

template<typename T>
class CircBufferT {
 public:
  CircBufferT() {
    buffer_.resize(size_);
  }

  uint numValues() const { return num_; }

  const T &getValue(uint ind) const {
    int pos = last_pos_ - ind;

    if (pos < 0)
      pos += size_ - 1;

    return buffer_[pos];
  }

  void addValue(const T &value) {
    if (num_ > 0 && buffer_[last_pos_] == value)
      return;

    buffer_[pos_] = value;

    last_pos_ = pos_++;

    if (pos_ >= size_)
      pos_ = 0;

    if (num_ < size_)
      ++num_;
  }

  void printValues(std::ostream &os) const {
    for (uint i = 0; i < num_; ++i) {
      const T &v = getValue(i);

      if (i > 0)
        os << " ";

      os << "0x" << CStrUtil::toHexString(v, 4);
    }

    os << std::endl;
  }

 private:
  typedef std::vector<T> Buffer;

  uint   pos_      { 0 };
  uint   last_pos_ { 0 };
  uint   size_     { 32 };
  uint   num_      { 0 };
  Buffer buffer_;
};

//--------

class CZ80MemFlags {
 public:
  CZ80MemFlags(ushort pos, ushort len, uchar flags) :
   pos_(pos), len_(len), flags_(flags) {
  }

  ushort pos() const { return pos_; }
  ushort len() const { return len_; }

  const uchar &flags() const { return flags_; }
  void setFlags(const uchar &v) { flags_ = v; }

  bool overlaps(ushort pos, ushort len) const {
    return ! (pos + len - 1 < pos_ || pos > pos_ + len_ - 1);
  }

 private:
  ushort pos_   { 0 };
  ushort len_   { 0 };
  uchar  flags_ { 0 };
};

//--------

class CZ80 {
 public:
  typedef std::vector<CZ80MemFlags> MemFlagsArray;

 public:
  CZ80();
 ~CZ80();

  // -----------

  void reset();
  void resetRegisters();

 private:
  void initOpInds();

  // -----------

  // Control

 public:
  ushort getIFreq() const { return ifreq_; }

  void setDump(bool dump);
  bool getDump() const { return dump_; }

  void setVerbose(bool verbose) { verbose_ = verbose; }
  bool getVerbose() { return verbose_; }

  void setExecData(CZ80ExecData *execData);

  void setMemData(CZ80MemData *memData);

  void setPortData(CZ80PortData *portData);

  void setScreen(CZ80Screen *screen);
  CZ80Screen *getScreen() const { return screen_; }

  void setRstData(CZ80RstData *rstData);

  void setDebugData(CZ80DebugData *debugData);

  void setSpeedData(CZ80SpeedData *speedData);

  void setHalt(bool halt);
  bool getHalt() const { return halt_; }

  void setStop(bool stop);
  bool getStop() const { return stop_; }

  void setAssembleStream(bool stream);

  ushort getLoadPos() const { return load_pos_; }
  ushort getLoadLen() const { return load_len_; }

  void setLoadPos(ushort pos) { load_pos_ = pos; }
  void setLoadLen(ushort len) { load_len_ = len; }

  CZ80OpData *opData() const { return opData_; }

  // -----------

  // Debug

  void addTrace(CZ80Trace *trace);

  void addBreakpoint(ushort pc);
  void removeBreakpoint(ushort pc);
  void removeAllBreakpoints();
  bool isBreakpoint(ushort pc);
  void getBreakpoints(std::vector<ushort> &addrs);

  void callPreStepProcs ();
  void callPostStepProcs();

  void callRegChanged(const CZ80Reg &reg);

  // -----------

  // Labels

  bool isLabelName(const std::string &name);
  void setLabelValue(const std::string &name, uint value);
  bool getLabelValue(const std::string &name, uint *value);
  bool getValueLabel(uint value, std::string &name);

  // -----------

  // Interrupts

  bool getAllowInterrupts() const { return allowInterrupts_; }
  void setAllowInterrupts(bool allow) { allowInterrupts_ = allow; }

  void setIM0(ushort im0) { im0_ = im0; }
  void setIM2(ushort im2) { im2_ = im2; }

  // -----------

  // Port

  void keyPress  (const CKeyEvent &kevent);
  void keyRelease(const CKeyEvent &kevent);

  // -----------

 public:
  // CPU State

  uchar getA() const { return registers_.a_; }
  uchar getF() const { return registers_.f_; }
  uchar getB() const { return registers_.b_; }
  uchar getC() const { return registers_.c_; }
  uchar getD() const { return registers_.d_; }
  uchar getE() const { return registers_.e_; }
  uchar getH() const { return registers_.h_; }
  uchar getL() const { return registers_.l_; }
  uchar getI() const { return registers_.i_; }

  ushort getAF  () const { return registers_.af_  ; }
  ushort getBC  () const { return registers_.bc_  ; }
  ushort getDE  () const { return registers_.de_  ; }
  ushort getHL  () const { return registers_.hl_  ; }
  ushort getSP  () const { return registers_.sp_  ; }
  ushort getPC  () const { return registers_.pc_  ; }
  ushort getIX  () const { return registers_.ix_  ; }
  ushort getIY  () const { return registers_.iy_  ; }
  uchar  getIXL () const { return registers_.ixl_ ; }
  uchar  getIXH () const { return registers_.ixh_ ; }
  uchar  getIYL () const { return registers_.iyl_ ; }
  uchar  getIYH () const { return registers_.iyh_ ; }

  ushort getIFF () const { return registers_.iff_ ; }
  uchar  getIFF1() const { return registers_.iff1_; }
  uchar  getIFF2() const { return registers_.iff2_; }

  ushort getAF1() const { return registers_.af_1_ ; }
  ushort getBC1() const { return registers_.bc_1_ ; }
  ushort getDE1() const { return registers_.de_1_ ; }
  ushort getHL1() const { return registers_.hl_1_ ; }

  uchar getIM() const { return registers_.im_; }

  uchar getPAF() const { return getByte(getAF()); }
  uchar getPBC() const { return getByte(getBC()); }
  uchar getPDE() const { return getByte(getDE()); }
  uchar getPHL() const { return getByte(getHL()); }
  uchar getPIX() const { return getByte(getIX()); }
  uchar getPIY() const { return getByte(getIY()); }
  uchar getPSP() const { return getByte(getSP()); }

  ushort getPBC2() const { return getWord(getBC()); }
  ushort getPDE2() const { return getWord(getDE()); }
  ushort getPHL2() const { return getWord(getHL()); }
  ushort getPIX2() const { return getWord(getIX()); }
  ushort getPIY2() const { return getWord(getIY()); }
  ushort getPPC2() const { return getWord(getPC()); }
  ushort getPSP2() const { return getWord(getSP()); }

  uchar getPOIX(schar o) const { return getByte(getIX() + o); }
  uchar getPOIY(schar o) const { return getByte(getIY() + o); }

  ushort getPOIX2(schar o) const { return getWord(getIX() + o); }
  ushort getPOIY2(schar o) const { return getWord(getIY() + o); }

  // TODO: Inline ?

  void setA(uchar a);
  void setF(uchar f);
  void setB(uchar b);
  void setC(uchar c);
  void setD(uchar d);
  void setE(uchar e);
  void setH(uchar h);
  void setL(uchar l);
  void setI(uchar i);

  void setAF  (ushort af);
  void setBC  (ushort bc);
  void setDE  (ushort de);
  void setHL  (ushort hl);
  void setSP  (ushort sp);
  void setPC  (ushort pc);
  void setIX  (ushort ix);
  void setIY  (ushort iy);
  void setIXL (uchar  ix);
  void setIXH (uchar  ix);
  void setIYL (uchar  iy);
  void setIYH (uchar  iy);
  void setIFF (ushort iff);
  void setIFF1(uchar  iff1);
  void setIFF2(uchar  iff2);

  void setAF1(ushort af);
  void setBC1(ushort bc);
  void setDE1(ushort de);
  void setHL1(ushort hl);

  void setIM(uchar im);

  void setPBC(uchar bc);
  void setPDE(uchar bc);
  void setPHL(uchar bc);

  void setPOIX(schar o, uchar ix);
  void setPOIY(schar o, uchar iy);

  void setPSP2(ushort sp);

  void  setFlag(uchar bit);
  void  resFlag(uchar bit);
  uchar tstFlag(uchar bit);

  void setCFlag(bool b=true) {b ? setFlag(uchar(CZ80Flag::C)) : resFlag(uchar(CZ80Flag::C));}
  void setNFlag(bool b=true) {b ? setFlag(uchar(CZ80Flag::N)) : resFlag(uchar(CZ80Flag::N));}
  void setPFlag(bool b=true) {b ? setFlag(uchar(CZ80Flag::P)) : resFlag(uchar(CZ80Flag::P));}
  void setVFlag(bool b=true) {b ? setFlag(uchar(CZ80Flag::V)) : resFlag(uchar(CZ80Flag::V));}
  void setXFlag(bool b=true) {b ? setFlag(uchar(CZ80Flag::X)) : resFlag(uchar(CZ80Flag::X));}
  void setHFlag(bool b=true) {b ? setFlag(uchar(CZ80Flag::H)) : resFlag(uchar(CZ80Flag::H));}
  void setYFlag(bool b=true) {b ? setFlag(uchar(CZ80Flag::Y)) : resFlag(uchar(CZ80Flag::Y));}
  void setZFlag(bool b=true) {b ? setFlag(uchar(CZ80Flag::Z)) : resFlag(uchar(CZ80Flag::Z));}
  void setSFlag(bool b=true) {b ? setFlag(uchar(CZ80Flag::S)) : resFlag(uchar(CZ80Flag::S));}

  void resCFlag(bool b=true) {b ? resFlag(uchar(CZ80Flag::C)) : setFlag(uchar(CZ80Flag::C));}
  void resNFlag(bool b=true) {b ? resFlag(uchar(CZ80Flag::N)) : setFlag(uchar(CZ80Flag::N));}
  void resPFlag(bool b=true) {b ? resFlag(uchar(CZ80Flag::P)) : setFlag(uchar(CZ80Flag::P));}
  void resVFlag(bool b=true) {b ? resFlag(uchar(CZ80Flag::V)) : setFlag(uchar(CZ80Flag::V));}
  void resXFlag(bool b=true) {b ? resFlag(uchar(CZ80Flag::X)) : setFlag(uchar(CZ80Flag::X));}
  void resHFlag(bool b=true) {b ? resFlag(uchar(CZ80Flag::H)) : setFlag(uchar(CZ80Flag::H));}
  void resYFlag(bool b=true) {b ? resFlag(uchar(CZ80Flag::Y)) : setFlag(uchar(CZ80Flag::Y));}
  void resZFlag(bool b=true) {b ? resFlag(uchar(CZ80Flag::Z)) : setFlag(uchar(CZ80Flag::Z));}
  void resSFlag(bool b=true) {b ? resFlag(uchar(CZ80Flag::S)) : setFlag(uchar(CZ80Flag::S));}

  uchar tstCFlag() { return tstFlag(uchar(CZ80Flag::C)); }
  uchar tstNFlag() { return tstFlag(uchar(CZ80Flag::N)); }
  uchar tstPFlag() { return tstFlag(uchar(CZ80Flag::P)); }
  uchar tstVFlag() { return tstFlag(uchar(CZ80Flag::V)); }
  uchar tstXFlag() { return tstFlag(uchar(CZ80Flag::X)); }
  uchar tstHFlag() { return tstFlag(uchar(CZ80Flag::H)); }
  uchar tstYFlag() { return tstFlag(uchar(CZ80Flag::Y)); }
  uchar tstZFlag() { return tstFlag(uchar(CZ80Flag::Z)); }
  uchar tstSFlag() { return tstFlag(uchar(CZ80Flag::S)); }

  void setR(uchar r) { registers_.r_ = r; }

  uint  getHR() const { return registers_.r_; }
  uchar getR () const { return getHR() & 0x7F; }

  void decR(uchar d);
  void incR(uchar d);

  ulong getT() const { return t_; }
  void setT(ulong t) { t_ = t; }

  void decT(ushort d);
  void incT(ushort d);

  int timerInterrupt();

  int interrupt();

  // ------

  // CPU Memory

 public:
  uchar  getByte() const;
  uchar  getByte(ushort pos) const;
  ushort getWord() const;
  ushort getWord(ushort pos) const;
  char   getSByte() const { return schar(getByte()); }
  char   getSByte(ushort pos) const { return schar(getByte(pos)); }
  short  getSWord() const { return sshort(getWord()); }
  short  getSWord(ushort pos) const { return sshort(getWord(pos)); }
  bool   getBit(ushort pos, uchar bit) const;
  void   getBytes(uchar *c, ushort pos, ushort len) const;

  uchar getMemory(ushort pos) const;
  void  setMemory(ushort pos, uchar data);

  void setByte(uchar c);
  void setByte(ushort pos, uchar c);
  void setWord(ushort c);
  void setWord(ushort pos, ushort c);
  void setBit(ushort pos, uchar bit);
  void resetBit(ushort pos, uchar bit);
  void setBytes(uchar *c, ushort pos, ushort len);

  // ------

  bool isReadOnly    (ushort pos, ushort len) const;
  bool isWriteTrigger(ushort pos, ushort len) const;
  bool isScreen      (ushort pos, ushort len) const;

  void setMemFlags  (ushort pos, ushort len, uchar flag);
  void resetMemFlags(ushort pos, ushort len, uchar flag);

  const MemFlagsArray &memFlagsArray() const { return memFlagsArray_; }

  // -------

 public:
  // CPU Ops

  void pushAF();
  void pushBC();
  void pushDE();
  void pushHL();
  void pushIX();
  void pushIY();
  void pushPC();

  void push(ushort r);

  void popAF();
  void popBC();
  void popDE();
  void popHL();
  void popIX();
  void popIY();
  void popPC();

  ushort pop();

  ushort peek();

  void addA (uchar  a);
  void addHL(ushort hl);
  void addIX(ushort ix);
  void addIY(ushort iy);

  void adcA (uchar  a);
  void adcHL(ushort hl);

  void subA(uchar a);

  void sbcA (uchar  a);
  void sbcHL(ushort hl);

  void cpA(uchar a);

  void andA(uchar a);
  void orA (uchar a);
  void xorA(uchar a);

  void incA();
  void incB();
  void incC();
  void incD();
  void incE();
  void incH();
  void incL();
  void incIXL();
  void incIXH();
  void incIYL();
  void incIYH();
  void incBC();
  void incDE();
  void incHL();
  void incSP();
  void incPC(sshort o=1);
  void incIX();
  void incIY();
  void incPHL();
  void incPOIX(schar o);
  void incPOIY(schar o);

  void decA();
  void decB();
  void decC();
  void decD();
  void decE();
  void decH();
  void decL();
  void decIXL();
  void decIXH();
  void decIYL();
  void decIYH();
  void decBC();
  void decDE();
  void decHL();
  void decSP();
  void decPC(sshort o=1);
  void decIX();
  void decIY();
  void decPHL();
  void decPOIX(schar o);
  void decPOIY(schar o);

#ifndef GAMEBOY_Z80
  void sllA();
  void sllB();
  void sllC();
  void sllD();
  void sllE();
  void sllH();
  void sllL();
  void sllPHL();
#else
  void swapA();
  void swapB();
  void swapC();
  void swapD();
  void swapE();
  void swapH();
  void swapL();
  void swapPHL();
#endif
#ifndef GAMEBOY_Z80
  void sllPOIX(schar o);
  void sllPOIY(schar o);
#endif

  void srlA();
  void srlB();
  void srlC();
  void srlD();
  void srlE();
  void srlH();
  void srlL();
  void srlPHL();
  void srlPOIX(schar o);
  void srlPOIY(schar o);

  void slaA();
  void slaB();
  void slaC();
  void slaD();
  void slaE();
  void slaH();
  void slaL();
  void slaPHL();
  void slaPOIX(schar o);
  void slaPOIY(schar o);

  void sraA();
  void sraB();
  void sraC();
  void sraD();
  void sraE();
  void sraH();
  void sraL();
  void sraPHL();
  void sraPOIX(schar o);
  void sraPOIY(schar o);

  void rlA();
  void rlB();
  void rlC();
  void rlD();
  void rlE();
  void rlH();
  void rlL();
  void rlPHL();
  void rlPOIX(schar o);
  void rlPOIY(schar o);

  void rlcA();
  void rlcB();
  void rlcC();
  void rlcD();
  void rlcE();
  void rlcH();
  void rlcL();
  void rlcPHL();
  void rlcPOIX(schar o);
  void rlcPOIY(schar o);

  void rrA();
  void rrB();
  void rrC();
  void rrD();
  void rrE();
  void rrH();
  void rrL();
  void rrPHL();
  void rrPOIX(schar o);
  void rrPOIY(schar o);

  void rrcA();
  void rrcB();
  void rrcC();
  void rrcD();
  void rrcE();
  void rrcH();
  void rrcL();
  void rrcPHL();
  void rrcPOIX(schar o);
  void rrcPOIY(schar o);

  void rld();
  void rrd();

  void tstBitA(uchar bit);
  void tstBitB(uchar bit);
  void tstBitC(uchar bit);
  void tstBitD(uchar bit);
  void tstBitE(uchar bit);
  void tstBitH(uchar bit);
  void tstBitL(uchar bit);
  void tstBitPHL(uchar bit);
  void tstBitPOIX(schar o, uchar bit, bool is_phl);
  void tstBitPOIY(schar o, uchar bit, bool is_phl);

  void setBitA(uchar bit);
  void setBitB(uchar bit);
  void setBitC(uchar bit);
  void setBitD(uchar bit);
  void setBitE(uchar bit);
  void setBitH(uchar bit);
  void setBitL(uchar bit);
  void setBitPHL(uchar bit);
  void setBitPOIX(schar o, uchar bit);
  void setBitPOIY(schar o, uchar bit);

  void resBitA(uchar bit);
  void resBitB(uchar bit);
  void resBitC(uchar bit);
  void resBitD(uchar bit);
  void resBitE(uchar bit);
  void resBitH(uchar bit);
  void resBitL(uchar bit);
  void resBitPHL(uchar bit);
  void resBitPOIX(schar o, uchar bit);
  void resBitPOIY(schar o, uchar bit);

  void call(ushort addr);

  void rst(ushort id);

  void daa();
  void cpl();
  void neg();
  void ccf();
  void scf();
  void di();
  void ei();
  void reti();
  void retn();

  void halt();
  void stop();

  void ex_de_hl();
  void ex_af_af1();
  void ex_p_sp_hl();
  void ex_p_sp_ix();
  void ex_p_sp_iy();
#ifndef GAMEBOY_Z80
  void exx();
#endif
  void ldi();
  void ldir();
  void ldd();
  void lddr();
  void cpi();
  void cpir();
  void cpd();
  void cpdr();

  void outC(uchar value);
  void out(uchar port, uchar value);

  uchar inC();
  uchar in(uchar port, uchar qual);

  // ---------

  // Load

 public:
  bool load(const std::string &file);
  bool load(CFile *file);
  bool load(CFile *file, ushort *pos, ushort *len);

  bool loadBin(const std::string &file);
  bool loadBin(CFile *file);
  bool loadBin(CFile *file, ushort *pos, ushort *len);
  void loadBin(const uchar *data, size_t len);

 private:
  uchar addR(uchar r, uchar a);
  uchar adcR(uchar r, uchar a);
  uchar subR(uchar r, uchar a);
  uchar sbcR(uchar r, uchar a);

  uchar setBitR(uchar r, uchar bit);
  uchar resBitR(uchar r, uchar bit);
  void  tstBitR(uchar r, uchar bit, bool is_phl);

  uchar inc(uchar r);
  uchar dec(uchar r);

#ifndef GAMEBOY_Z80
  uchar sll(uchar r);
#else
  uchar swap(uchar r);
#endif
  uchar srl(uchar r);
  uchar sla(uchar r);
  uchar sra(uchar r);

  void rlca();
  void rla();
  void rrca();
  void rra ();

  uchar rlc(uchar r);
  uchar rl (uchar r);
  uchar rrc(uchar r);
  uchar rr (uchar r);

  void setBRFlags(uchar r);
  void setRFlags(uchar r);

  bool tstParity(uchar c);

  // ---------

  // Disassemble

 public:
  bool disassemble(std::ostream &os=std::cout);
  bool disassemble(ushort pos, ushort len, std::ostream &os=std::cout);

  // ---------

  // Assemble

 public:
  bool assemble(CFile *ifile, std::ostream &os=std::cout);
  bool assemble(CFile *ifile, CFile *ofile);

  void assembleAll(ushort pc, std::ostream &os=std::cout);

  void assembleDumpSymbols();

 private:
  bool assemble(CFile *ifile, CFileBase *ofile);

  void assembleLoadLine(CFileParse *parse);

  void assembleDumpValues(CFileBase *ofile);

  bool assembleParseOp(CFileParse *parse, std::string &str, uint pass, bool *continued);

  bool assembleParseInteger(CFileParse *parse, uint pass, uint *i);

  void assembleSetLabelValue(const std::string &name, uint value);

  bool assembleEvalExpr(const std::string &expr, uint *value);

  void assembleError(const char *format, ...);

  void assembleOp(CZ80Op *op, std::ostream &os=std::cout);

  bool assembleStringToOpId(const std::string &str, uint *id);
  bool assembleStringToFlagId(const std::string &str, uint *id);
  bool assembleStringToRegisterId(const std::string &str, uint *id);

  void assembleDefineMacro(const std::string &name, const std::vector<std::string> &args,
                           const std::string &body);

  bool assembleGetMacro(const std::string &name, std::vector<std::string> &args,
                        std::string &body) const;

  // ---------

  // Execute

 public:
  bool execute();
  bool execute(ushort pos);

  bool execNext();
  void execSkip();
  bool execStep();
  bool execCont();
  void execStop();

 private:
  bool execute1(bool notify);
  bool execStep1(bool notify);


  // ---------

 public:
  bool undump(CFile *file, std::ostream &os=std::cout);

  // ---------

  // Read Next Memory Op

 public:
  void getOpData(CZ80OpData *opData);

  bool readOpData(ushort pc, CZ80OpData *opData);

 private:
  CZ80Op *readOp(ushort pc);

  void readOpValues(ushort pc, CZ80Op *op, uchar *values1, uchar *num_values1,
                    uchar *values2, uchar *num_values2);

  uchar getNumArgValues(uint type, uint arg);

  // ---------

  // Functions

 public:
  //  adc

  static CZ80OpProc f_adc_a_a;
  static CZ80OpProc f_adc_a_b;
  static CZ80OpProc f_adc_a_c;
  static CZ80OpProc f_adc_a_d;
  static CZ80OpProc f_adc_a_e;
  static CZ80OpProc f_adc_a_h;
  static CZ80OpProc f_adc_a_l;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_adc_a_ixh;
  static CZ80OpProc f_adc_a_ixl;
  static CZ80OpProc f_adc_a_iyh;
  static CZ80OpProc f_adc_a_iyl;
#endif
  static CZ80OpProc f_adc_a_p_hl;
  static CZ80OpProc f_adc_a_n;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_adc_a_po_ix;
  static CZ80OpProc f_adc_a_po_iy;
#endif
  static CZ80OpProc f_adc_hl_bc;
  static CZ80OpProc f_adc_hl_de;
  static CZ80OpProc f_adc_hl_hl;
  static CZ80OpProc f_adc_hl_sp;

  // add

  static CZ80OpProc f_add_a_a;
  static CZ80OpProc f_add_a_b;
  static CZ80OpProc f_add_a_c;
  static CZ80OpProc f_add_a_d;
  static CZ80OpProc f_add_a_e;
  static CZ80OpProc f_add_a_h;
  static CZ80OpProc f_add_a_l;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_add_a_ixh;
  static CZ80OpProc f_add_a_ixl;
  static CZ80OpProc f_add_a_iyh;
  static CZ80OpProc f_add_a_iyl;
#endif
  static CZ80OpProc f_add_a_p_hl;
  static CZ80OpProc f_add_a_n;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_add_a_po_ix;
  static CZ80OpProc f_add_a_po_iy;
#endif
  static CZ80OpProc f_add_hl_bc;
  static CZ80OpProc f_add_hl_de;
  static CZ80OpProc f_add_hl_hl;
  static CZ80OpProc f_add_hl_sp;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_add_ix_bc;
  static CZ80OpProc f_add_ix_de;
  static CZ80OpProc f_add_ix_ix;
  static CZ80OpProc f_add_ix_sp;
  static CZ80OpProc f_add_iy_bc;
  static CZ80OpProc f_add_iy_de;
  static CZ80OpProc f_add_iy_iy;
  static CZ80OpProc f_add_iy_sp;
#endif

  // and

  static CZ80OpProc f_and_a;
  static CZ80OpProc f_and_b;
  static CZ80OpProc f_and_c;
  static CZ80OpProc f_and_d;
  static CZ80OpProc f_and_e;
  static CZ80OpProc f_and_h;
  static CZ80OpProc f_and_l;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_and_ixh;
  static CZ80OpProc f_and_ixl;
  static CZ80OpProc f_and_iyh;
  static CZ80OpProc f_and_iyl;
#endif
  static CZ80OpProc f_and_p_hl;
  static CZ80OpProc f_and_n;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_and_po_ix;
  static CZ80OpProc f_and_po_iy;
#endif

  // bit

  static CZ80OpProc f_bit_a_n;
  static CZ80OpProc f_bit_b_n;
  static CZ80OpProc f_bit_c_n;
  static CZ80OpProc f_bit_d_n;
  static CZ80OpProc f_bit_e_n;
  static CZ80OpProc f_bit_h_n;
  static CZ80OpProc f_bit_l_n;
  static CZ80OpProc f_bit_p_hl_n;

#ifndef GAMEBOY_Z80
  static CZ80OpProc f_bit_po_ix_n;
  static CZ80OpProc f_bit_po_ix_ns;
  static CZ80OpProc f_bit_po_iy_n;
  static CZ80OpProc f_bit_po_iy_ns;
#endif

  // call

  static CZ80OpProc f_call;
  static CZ80OpProc f_call_z;
  static CZ80OpProc f_call_nz;
  static CZ80OpProc f_call_c;
  static CZ80OpProc f_call_nc;
  static CZ80OpProc f_call_pe;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_call_po;
#endif
  static CZ80OpProc f_call_p;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_call_m;
#endif

  static CZ80OpProc f_ccf;

  // cp

  static CZ80OpProc f_cp_a;
  static CZ80OpProc f_cp_b;
  static CZ80OpProc f_cp_c;
  static CZ80OpProc f_cp_d;
  static CZ80OpProc f_cp_e;
  static CZ80OpProc f_cp_h;
  static CZ80OpProc f_cp_l;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_cp_ixh;
  static CZ80OpProc f_cp_ixl;
  static CZ80OpProc f_cp_iyh;
  static CZ80OpProc f_cp_iyl;
#endif
  static CZ80OpProc f_cp_p_hl;
  static CZ80OpProc f_cp_n;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_cp_po_ix;
  static CZ80OpProc f_cp_po_iy;
#endif

  static CZ80OpProc f_cpd;
  static CZ80OpProc f_cpdr;
  static CZ80OpProc f_cpi;
  static CZ80OpProc f_cpir;
  static CZ80OpProc f_cpl;
  static CZ80OpProc f_daa;

  // dec

  static CZ80OpProc f_dec_a;
  static CZ80OpProc f_dec_b;
  static CZ80OpProc f_dec_c;
  static CZ80OpProc f_dec_d;
  static CZ80OpProc f_dec_e;
  static CZ80OpProc f_dec_h;
  static CZ80OpProc f_dec_l;
  static CZ80OpProc f_dec_p_hl;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_dec_po_ix;
  static CZ80OpProc f_dec_po_iy;
#endif
  static CZ80OpProc f_dec_bc;
  static CZ80OpProc f_dec_de;
  static CZ80OpProc f_dec_hl;
  static CZ80OpProc f_dec_sp;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_dec_ix;
  static CZ80OpProc f_dec_iy;
  static CZ80OpProc f_dec_ixh;
  static CZ80OpProc f_dec_ixl;
  static CZ80OpProc f_dec_iyh;
  static CZ80OpProc f_dec_iyl;
#endif

  static CZ80OpProc f_di;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_djnz;
#else
  static CZ80OpProc f_stop;
#endif
  static CZ80OpProc f_ei;

  // ex

  static CZ80OpProc f_ex_de_hl;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ex_af_af1;
#else
  static CZ80OpProc f_ld_p_nn_sp;
#endif
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ex_p_sp_hl;
#endif
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ex_p_sp_ix;
  static CZ80OpProc f_ex_p_sp_iy;
#endif

#ifndef GAMEBOY_Z80
  static CZ80OpProc f_exx;
#endif
  static CZ80OpProc f_halt;

  // im

  static CZ80OpProc f_im_n;

  // in

#ifndef GAMEBOY_Z80
  static CZ80OpProc f_in_a_p_n;
#endif
  static CZ80OpProc f_in_a_p_c;
  static CZ80OpProc f_in_b_p_c;
  static CZ80OpProc f_in_c_p_c;
  static CZ80OpProc f_in_d_p_c;
  static CZ80OpProc f_in_e_p_c;
  static CZ80OpProc f_in_h_p_c;
  static CZ80OpProc f_in_l_p_c;
  static CZ80OpProc f_in_n_p_c;

  // inc

  static CZ80OpProc f_inc_a;
  static CZ80OpProc f_inc_b;
  static CZ80OpProc f_inc_c;
  static CZ80OpProc f_inc_d;
  static CZ80OpProc f_inc_e;
  static CZ80OpProc f_inc_h;
  static CZ80OpProc f_inc_l;
  static CZ80OpProc f_inc_p_hl;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_inc_po_ix;
  static CZ80OpProc f_inc_po_iy;
#endif
  static CZ80OpProc f_inc_bc;
  static CZ80OpProc f_inc_de;
  static CZ80OpProc f_inc_hl;
  static CZ80OpProc f_inc_sp;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_inc_ix;
  static CZ80OpProc f_inc_iy;
  static CZ80OpProc f_inc_ixh;
  static CZ80OpProc f_inc_ixl;
  static CZ80OpProc f_inc_iyh;
  static CZ80OpProc f_inc_iyl;
#endif

  static CZ80OpProc f_ind;
  static CZ80OpProc f_indr;
  static CZ80OpProc f_ini;
  static CZ80OpProc f_inir;

  // jp

  static CZ80OpProc f_jp;
  static CZ80OpProc f_jp_z;
  static CZ80OpProc f_jp_nz;
  static CZ80OpProc f_jp_c;
  static CZ80OpProc f_jp_nc;
  static CZ80OpProc f_jp_pe;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_jp_po;
#else
  static CZ80OpProc f_ld_p_fc_a;
#endif
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_jp_p;
#else
  static CZ80OpProc f_ld_a_p_fc;
#endif
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_jp_m;
#else
  static CZ80OpProc f_ld_a_p_nn;
#endif
  static CZ80OpProc f_jp_hl;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_jp_ix;
  static CZ80OpProc f_jp_iy;
#endif

  // jr

  static CZ80OpProc f_jr;
  static CZ80OpProc f_jr_z;
  static CZ80OpProc f_jr_nz;
  static CZ80OpProc f_jr_c;
  static CZ80OpProc f_jr_nc;

  // ld

  static CZ80OpProc f_ld_a_n;
  static CZ80OpProc f_ld_b_n;
  static CZ80OpProc f_ld_c_n;
  static CZ80OpProc f_ld_d_n;
  static CZ80OpProc f_ld_e_n;
  static CZ80OpProc f_ld_h_n;
  static CZ80OpProc f_ld_l_n;
  static CZ80OpProc f_ld_bc_nn;
  static CZ80OpProc f_ld_de_nn;
  static CZ80OpProc f_ld_hl_nn;
  static CZ80OpProc f_ld_sp_nn;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ld_ix_nn;
  static CZ80OpProc f_ld_iy_nn;
  static CZ80OpProc f_ld_ixh_n;
  static CZ80OpProc f_ld_ixl_n;
  static CZ80OpProc f_ld_iyh_n;
  static CZ80OpProc f_ld_iyl_n;
#endif

  static CZ80OpProc f_ld_a_a;
  static CZ80OpProc f_ld_a_b;
  static CZ80OpProc f_ld_a_c;
  static CZ80OpProc f_ld_a_d;
  static CZ80OpProc f_ld_a_e;
  static CZ80OpProc f_ld_a_h;
  static CZ80OpProc f_ld_a_l;
  static CZ80OpProc f_ld_a_i;
  static CZ80OpProc f_ld_a_r;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ld_a_ixh;
  static CZ80OpProc f_ld_a_ixl;
  static CZ80OpProc f_ld_a_iyh;
  static CZ80OpProc f_ld_a_iyl;
#endif

  static CZ80OpProc f_ld_b_a;
  static CZ80OpProc f_ld_b_b;
  static CZ80OpProc f_ld_b_c;
  static CZ80OpProc f_ld_b_d;
  static CZ80OpProc f_ld_b_e;
  static CZ80OpProc f_ld_b_h;
  static CZ80OpProc f_ld_b_l;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ld_b_ixh;
  static CZ80OpProc f_ld_b_ixl;
  static CZ80OpProc f_ld_b_iyh;
  static CZ80OpProc f_ld_b_iyl;
#endif

  static CZ80OpProc f_ld_c_a;
  static CZ80OpProc f_ld_c_b;
  static CZ80OpProc f_ld_c_c;
  static CZ80OpProc f_ld_c_d;
  static CZ80OpProc f_ld_c_e;
  static CZ80OpProc f_ld_c_h;
  static CZ80OpProc f_ld_c_l;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ld_c_ixh;
  static CZ80OpProc f_ld_c_ixl;
  static CZ80OpProc f_ld_c_iyh;
  static CZ80OpProc f_ld_c_iyl;
#endif

  static CZ80OpProc f_ld_d_a;
  static CZ80OpProc f_ld_d_b;
  static CZ80OpProc f_ld_d_c;
  static CZ80OpProc f_ld_d_d;
  static CZ80OpProc f_ld_d_e;
  static CZ80OpProc f_ld_d_h;
  static CZ80OpProc f_ld_d_l;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ld_d_ixh;
  static CZ80OpProc f_ld_d_ixl;
  static CZ80OpProc f_ld_d_iyh;
  static CZ80OpProc f_ld_d_iyl;
#endif

  static CZ80OpProc f_ld_e_a;
  static CZ80OpProc f_ld_e_b;
  static CZ80OpProc f_ld_e_c;
  static CZ80OpProc f_ld_e_d;
  static CZ80OpProc f_ld_e_e;
  static CZ80OpProc f_ld_e_h;
  static CZ80OpProc f_ld_e_l;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ld_e_ixh;
  static CZ80OpProc f_ld_e_ixl;
  static CZ80OpProc f_ld_e_iyh;
  static CZ80OpProc f_ld_e_iyl;
#endif

  static CZ80OpProc f_ld_h_a;
  static CZ80OpProc f_ld_h_b;
  static CZ80OpProc f_ld_h_c;
  static CZ80OpProc f_ld_h_d;
  static CZ80OpProc f_ld_h_e;
  static CZ80OpProc f_ld_h_h;
  static CZ80OpProc f_ld_h_l;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ld_h_ixh;
  static CZ80OpProc f_ld_h_ixl;
  static CZ80OpProc f_ld_h_iyh;
  static CZ80OpProc f_ld_h_iyl;
#endif

  static CZ80OpProc f_ld_l_a;
  static CZ80OpProc f_ld_l_b;
  static CZ80OpProc f_ld_l_c;
  static CZ80OpProc f_ld_l_d;
  static CZ80OpProc f_ld_l_e;
  static CZ80OpProc f_ld_l_h;
  static CZ80OpProc f_ld_l_l;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ld_l_ixh;
  static CZ80OpProc f_ld_l_ixl;
  static CZ80OpProc f_ld_l_iyh;
  static CZ80OpProc f_ld_l_iyl;
#endif

#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ld_ixh_a;
  static CZ80OpProc f_ld_ixh_b;
  static CZ80OpProc f_ld_ixh_c;
  static CZ80OpProc f_ld_ixh_d;
  static CZ80OpProc f_ld_ixh_e;
  static CZ80OpProc f_ld_ixh_ixh;
  static CZ80OpProc f_ld_ixh_ixl;

  static CZ80OpProc f_ld_ixl_a;
  static CZ80OpProc f_ld_ixl_b;
  static CZ80OpProc f_ld_ixl_c;
  static CZ80OpProc f_ld_ixl_d;
  static CZ80OpProc f_ld_ixl_e;
  static CZ80OpProc f_ld_ixl_ixh;
  static CZ80OpProc f_ld_ixl_ixl;

  static CZ80OpProc f_ld_iyh_a;
  static CZ80OpProc f_ld_iyh_b;
  static CZ80OpProc f_ld_iyh_c;
  static CZ80OpProc f_ld_iyh_d;
  static CZ80OpProc f_ld_iyh_e;
  static CZ80OpProc f_ld_iyh_iyh;
  static CZ80OpProc f_ld_iyh_iyl;

  static CZ80OpProc f_ld_iyl_a;
  static CZ80OpProc f_ld_iyl_b;
  static CZ80OpProc f_ld_iyl_c;
  static CZ80OpProc f_ld_iyl_d;
  static CZ80OpProc f_ld_iyl_e;
  static CZ80OpProc f_ld_iyl_iyh;
  static CZ80OpProc f_ld_iyl_iyl;
#endif

  static CZ80OpProc f_ld_i_a;
  static CZ80OpProc f_ld_r_a;

  static CZ80OpProc f_ld_a_p_bc;
  static CZ80OpProc f_ld_a_p_de;
  static CZ80OpProc f_ld_a_p_hl;
  static CZ80OpProc f_ld_b_p_hl;
  static CZ80OpProc f_ld_c_p_hl;
  static CZ80OpProc f_ld_d_p_hl;
  static CZ80OpProc f_ld_e_p_hl;
  static CZ80OpProc f_ld_h_p_hl;
  static CZ80OpProc f_ld_l_p_hl;
  static CZ80OpProc f_ld_n_p_hl;

  static CZ80OpProc f_ld_p_bc_a;
  static CZ80OpProc f_ld_p_de_a;
  static CZ80OpProc f_ld_p_hl_a;
  static CZ80OpProc f_ld_p_hl_b;
  static CZ80OpProc f_ld_p_hl_c;
  static CZ80OpProc f_ld_p_hl_d;
  static CZ80OpProc f_ld_p_hl_e;
  static CZ80OpProc f_ld_p_hl_h;
  static CZ80OpProc f_ld_p_hl_l;
  static CZ80OpProc f_ld_p_hl_n;

#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ld_a_p_nn;
#else
  static CZ80OpProc f_ld_a_p_hld;
#endif
  static CZ80OpProc f_ld_bc_p_nn;
  static CZ80OpProc f_ld_de_p_nn;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ld_hl_p_nn;
#else
  static CZ80OpProc f_ld_hl_p_nn;
  static CZ80OpProc f_ld_a_p_hli;
#endif
  static CZ80OpProc f_ld_sp_p_nn;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ld_ix_p_nn;
  static CZ80OpProc f_ld_iy_p_nn;
#endif

#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ld_p_nn_a;
#else
  static CZ80OpProc f_ld_p_nn_a;
  static CZ80OpProc f_ld_p_hld_a;
#endif

  static CZ80OpProc f_ld_p_nn_bc;
  static CZ80OpProc f_ld_p_nn_de;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ld_p_nn_hl;
#else
  static CZ80OpProc f_ld_p_nn_hl;
  static CZ80OpProc f_ld_p_hli_a;
#endif
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ld_p_nn_sp;
#endif
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ld_p_nn_ix;
  static CZ80OpProc f_ld_p_nn_iy;
#endif

  static CZ80OpProc f_ld_sp_hl;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ld_sp_ix;
  static CZ80OpProc f_ld_sp_iy;
#endif

#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ld_a_po_ix;
  static CZ80OpProc f_ld_b_po_ix;
  static CZ80OpProc f_ld_c_po_ix;
  static CZ80OpProc f_ld_d_po_ix;
  static CZ80OpProc f_ld_e_po_ix;
  static CZ80OpProc f_ld_h_po_ix;
  static CZ80OpProc f_ld_l_po_ix;

  static CZ80OpProc f_ld_a_po_iy;
  static CZ80OpProc f_ld_b_po_iy;
  static CZ80OpProc f_ld_c_po_iy;
  static CZ80OpProc f_ld_d_po_iy;
  static CZ80OpProc f_ld_e_po_iy;
  static CZ80OpProc f_ld_h_po_iy;
  static CZ80OpProc f_ld_l_po_iy;

  static CZ80OpProc f_ld_po_ix_a;
  static CZ80OpProc f_ld_po_ix_b;
  static CZ80OpProc f_ld_po_ix_c;
  static CZ80OpProc f_ld_po_ix_d;
  static CZ80OpProc f_ld_po_ix_e;
  static CZ80OpProc f_ld_po_ix_h;
  static CZ80OpProc f_ld_po_ix_l;
  static CZ80OpProc f_ld_po_ix_n;

  static CZ80OpProc f_ld_po_iy_a;
  static CZ80OpProc f_ld_po_iy_b;
  static CZ80OpProc f_ld_po_iy_c;
  static CZ80OpProc f_ld_po_iy_d;
  static CZ80OpProc f_ld_po_iy_e;
  static CZ80OpProc f_ld_po_iy_h;
  static CZ80OpProc f_ld_po_iy_l;
  static CZ80OpProc f_ld_po_iy_n;
#endif

  // ldd

  static CZ80OpProc f_ldd;
  static CZ80OpProc f_lddr;
  static CZ80OpProc f_ldi;
  static CZ80OpProc f_ldir;
  static CZ80OpProc f_neg;
  static CZ80OpProc f_nop;

  // or

  static CZ80OpProc f_or_a;
  static CZ80OpProc f_or_b;
  static CZ80OpProc f_or_c;
  static CZ80OpProc f_or_d;
  static CZ80OpProc f_or_e;
  static CZ80OpProc f_or_h;
  static CZ80OpProc f_or_l;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_or_ixh;
  static CZ80OpProc f_or_ixl;
  static CZ80OpProc f_or_iyh;
  static CZ80OpProc f_or_iyl;
#endif
  static CZ80OpProc f_or_p_hl;
  static CZ80OpProc f_or_n;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_or_po_ix;
  static CZ80OpProc f_or_po_iy;
#endif

  static CZ80OpProc f_otdr;
  static CZ80OpProc f_otir;

  // out

#ifndef GAMEBOY_Z80
  static CZ80OpProc f_out_p_n_a;
#endif
  static CZ80OpProc f_out_p_c_a;
  static CZ80OpProc f_out_p_c_b;
  static CZ80OpProc f_out_p_c_c;
  static CZ80OpProc f_out_p_c_d;
  static CZ80OpProc f_out_p_c_e;
  static CZ80OpProc f_out_p_c_h;
  static CZ80OpProc f_out_p_c_l;
  static CZ80OpProc f_out_p_c_n;

  static CZ80OpProc f_outd;
  static CZ80OpProc f_outi;

  // pop

  static CZ80OpProc f_pop_af;
  static CZ80OpProc f_pop_bc;
  static CZ80OpProc f_pop_de;
  static CZ80OpProc f_pop_hl;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_pop_ix;
  static CZ80OpProc f_pop_iy;
#endif

  // push

  static CZ80OpProc f_push_af;
  static CZ80OpProc f_push_bc;
  static CZ80OpProc f_push_de;
  static CZ80OpProc f_push_hl;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_push_ix;
  static CZ80OpProc f_push_iy;
#endif

  // res

#ifndef GAMEBOY_Z80
  static CZ80OpProc f_res_po_ix_n;
  static CZ80OpProc f_res_po_iy_n;
#endif
  static CZ80OpProc f_res_a_n;
  static CZ80OpProc f_res_b_n;
  static CZ80OpProc f_res_c_n;
  static CZ80OpProc f_res_d_n;
  static CZ80OpProc f_res_e_n;
  static CZ80OpProc f_res_h_n;
  static CZ80OpProc f_res_l_n;
  static CZ80OpProc f_res_p_hl_n;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_res_po_ix_a_n;
  static CZ80OpProc f_res_po_ix_b_n;
  static CZ80OpProc f_res_po_ix_c_n;
  static CZ80OpProc f_res_po_ix_d_n;
  static CZ80OpProc f_res_po_ix_e_n;
  static CZ80OpProc f_res_po_ix_h_n;
  static CZ80OpProc f_res_po_ix_l_n;
  static CZ80OpProc f_res_po_iy_a_n;
  static CZ80OpProc f_res_po_iy_b_n;
  static CZ80OpProc f_res_po_iy_c_n;
  static CZ80OpProc f_res_po_iy_d_n;
  static CZ80OpProc f_res_po_iy_e_n;
  static CZ80OpProc f_res_po_iy_h_n;
  static CZ80OpProc f_res_po_iy_l_n;
#endif

#ifndef GAMEBOY_Z80
  static void f_res_po_ix_a_n(CZ80OpData *opData, uchar o);
  static void f_res_po_ix_b_n(CZ80OpData *opData, uchar o);
  static void f_res_po_ix_c_n(CZ80OpData *opData, uchar o);
  static void f_res_po_ix_d_n(CZ80OpData *opData, uchar o);
  static void f_res_po_ix_e_n(CZ80OpData *opData, uchar o);
  static void f_res_po_ix_h_n(CZ80OpData *opData, uchar o);
  static void f_res_po_ix_l_n(CZ80OpData *opData, uchar o);
  static void f_res_po_iy_a_n(CZ80OpData *opData, uchar o);
  static void f_res_po_iy_b_n(CZ80OpData *opData, uchar o);
  static void f_res_po_iy_c_n(CZ80OpData *opData, uchar o);
  static void f_res_po_iy_d_n(CZ80OpData *opData, uchar o);
  static void f_res_po_iy_e_n(CZ80OpData *opData, uchar o);
  static void f_res_po_iy_h_n(CZ80OpData *opData, uchar o);
  static void f_res_po_iy_l_n(CZ80OpData *opData, uchar o);
#endif

  // ret

  static CZ80OpProc f_ret;
  static CZ80OpProc f_ret_z;
  static CZ80OpProc f_ret_nz;
  static CZ80OpProc f_ret_c;
  static CZ80OpProc f_ret_nc;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ret_pe;
#else
  static CZ80OpProc f_add_sp_n;
#endif
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ret_po;
#else
  static CZ80OpProc f_ld_p_fn_a;
#endif
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ret_p;
#else
  static CZ80OpProc f_ld_a_p_fn;
#endif
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_ret_m;
#else
  static CZ80OpProc f_ld_hl_sp_n;
#endif

  static CZ80OpProc f_reti;
  static CZ80OpProc f_retn;

  // rl

  static CZ80OpProc f_rl_a;
  static CZ80OpProc f_rl_b;
  static CZ80OpProc f_rl_c;
  static CZ80OpProc f_rl_d;
  static CZ80OpProc f_rl_e;
  static CZ80OpProc f_rl_h;
  static CZ80OpProc f_rl_l;
  static CZ80OpProc f_rl_p_hl;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_rl_po_ix;
  static CZ80OpProc f_rl_po_ix_a;
  static CZ80OpProc f_rl_po_ix_b;
  static CZ80OpProc f_rl_po_ix_c;
  static CZ80OpProc f_rl_po_ix_d;
  static CZ80OpProc f_rl_po_ix_e;
  static CZ80OpProc f_rl_po_ix_h;
  static CZ80OpProc f_rl_po_ix_l;
  static CZ80OpProc f_rl_po_iy;
  static CZ80OpProc f_rl_po_iy_a;
  static CZ80OpProc f_rl_po_iy_b;
  static CZ80OpProc f_rl_po_iy_c;
  static CZ80OpProc f_rl_po_iy_d;
  static CZ80OpProc f_rl_po_iy_e;
  static CZ80OpProc f_rl_po_iy_h;
  static CZ80OpProc f_rl_po_iy_l;
#endif

  // rla

  static CZ80OpProc f_rla;

  // rlc

  static CZ80OpProc f_rlc_a;
  static CZ80OpProc f_rlc_b;
  static CZ80OpProc f_rlc_c;
  static CZ80OpProc f_rlc_d;
  static CZ80OpProc f_rlc_e;
  static CZ80OpProc f_rlc_h;
  static CZ80OpProc f_rlc_l;
  static CZ80OpProc f_rlc_p_hl;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_rlc_po_ix;
  static CZ80OpProc f_rlc_po_ix_a;
  static CZ80OpProc f_rlc_po_ix_b;
  static CZ80OpProc f_rlc_po_ix_c;
  static CZ80OpProc f_rlc_po_ix_d;
  static CZ80OpProc f_rlc_po_ix_e;
  static CZ80OpProc f_rlc_po_ix_h;
  static CZ80OpProc f_rlc_po_ix_l;
  static CZ80OpProc f_rlc_po_iy;
  static CZ80OpProc f_rlc_po_iy_a;
  static CZ80OpProc f_rlc_po_iy_b;
  static CZ80OpProc f_rlc_po_iy_c;
  static CZ80OpProc f_rlc_po_iy_d;
  static CZ80OpProc f_rlc_po_iy_e;
  static CZ80OpProc f_rlc_po_iy_h;
  static CZ80OpProc f_rlc_po_iy_l;
#endif

  static CZ80OpProc f_rlca;
  static CZ80OpProc f_rld;

  // rr

  static CZ80OpProc f_rr_a;
  static CZ80OpProc f_rr_b;
  static CZ80OpProc f_rr_c;
  static CZ80OpProc f_rr_d;
  static CZ80OpProc f_rr_e;
  static CZ80OpProc f_rr_h;
  static CZ80OpProc f_rr_l;
  static CZ80OpProc f_rr_p_hl;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_rr_po_ix;
  static CZ80OpProc f_rr_po_ix_a;
  static CZ80OpProc f_rr_po_ix_b;
  static CZ80OpProc f_rr_po_ix_c;
  static CZ80OpProc f_rr_po_ix_d;
  static CZ80OpProc f_rr_po_ix_e;
  static CZ80OpProc f_rr_po_ix_h;
  static CZ80OpProc f_rr_po_ix_l;
  static CZ80OpProc f_rr_po_iy;
  static CZ80OpProc f_rr_po_iy_a;
  static CZ80OpProc f_rr_po_iy_b;
  static CZ80OpProc f_rr_po_iy_c;
  static CZ80OpProc f_rr_po_iy_d;
  static CZ80OpProc f_rr_po_iy_e;
  static CZ80OpProc f_rr_po_iy_h;
  static CZ80OpProc f_rr_po_iy_l;
#endif

  // rra

  static CZ80OpProc f_rra;

  // rrc

  static CZ80OpProc f_rrc_a;
  static CZ80OpProc f_rrc_b;
  static CZ80OpProc f_rrc_c;
  static CZ80OpProc f_rrc_d;
  static CZ80OpProc f_rrc_e;
  static CZ80OpProc f_rrc_h;
  static CZ80OpProc f_rrc_l;
  static CZ80OpProc f_rrc_p_hl;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_rrc_po_ix;
  static CZ80OpProc f_rrc_po_ix_a;
  static CZ80OpProc f_rrc_po_ix_b;
  static CZ80OpProc f_rrc_po_ix_c;
  static CZ80OpProc f_rrc_po_ix_d;
  static CZ80OpProc f_rrc_po_ix_e;
  static CZ80OpProc f_rrc_po_ix_h;
  static CZ80OpProc f_rrc_po_ix_l;
  static CZ80OpProc f_rrc_po_iy;
  static CZ80OpProc f_rrc_po_iy_a;
  static CZ80OpProc f_rrc_po_iy_b;
  static CZ80OpProc f_rrc_po_iy_c;
  static CZ80OpProc f_rrc_po_iy_d;
  static CZ80OpProc f_rrc_po_iy_e;
  static CZ80OpProc f_rrc_po_iy_h;
  static CZ80OpProc f_rrc_po_iy_l;
#endif

  static CZ80OpProc f_rrca;
  static CZ80OpProc f_rrd;

  // rst

  static CZ80OpProc f_rst_n;

  // sbc

  static CZ80OpProc f_sbc_a_a;
  static CZ80OpProc f_sbc_a_b;
  static CZ80OpProc f_sbc_a_c;
  static CZ80OpProc f_sbc_a_d;
  static CZ80OpProc f_sbc_a_e;
  static CZ80OpProc f_sbc_a_h;
  static CZ80OpProc f_sbc_a_l;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_sbc_a_ixh;
  static CZ80OpProc f_sbc_a_ixl;
  static CZ80OpProc f_sbc_a_iyh;
  static CZ80OpProc f_sbc_a_iyl;
#endif
  static CZ80OpProc f_sbc_a_p_hl;
  static CZ80OpProc f_sbc_a_n;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_sbc_po_ix;
  static CZ80OpProc f_sbc_po_iy;
#endif
  static CZ80OpProc f_sbc_hl_bc;
  static CZ80OpProc f_sbc_hl_de;
  static CZ80OpProc f_sbc_hl_hl;
  static CZ80OpProc f_sbc_hl_sp;

  static CZ80OpProc f_scf;

  // set

  static CZ80OpProc f_set_a_n;
  static CZ80OpProc f_set_b_n;
  static CZ80OpProc f_set_c_n;
  static CZ80OpProc f_set_d_n;
  static CZ80OpProc f_set_e_n;
  static CZ80OpProc f_set_h_n;
  static CZ80OpProc f_set_l_n;
  static CZ80OpProc f_set_p_hl_n;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_set_po_ix_n;
  static CZ80OpProc f_set_po_ix_a_n;
  static CZ80OpProc f_set_po_ix_b_n;
  static CZ80OpProc f_set_po_ix_c_n;
  static CZ80OpProc f_set_po_ix_d_n;
  static CZ80OpProc f_set_po_ix_e_n;
  static CZ80OpProc f_set_po_ix_h_n;
  static CZ80OpProc f_set_po_ix_l_n;
  static CZ80OpProc f_set_po_iy_n;
  static CZ80OpProc f_set_po_iy_a_n;
  static CZ80OpProc f_set_po_iy_b_n;
  static CZ80OpProc f_set_po_iy_c_n;
  static CZ80OpProc f_set_po_iy_d_n;
  static CZ80OpProc f_set_po_iy_e_n;
  static CZ80OpProc f_set_po_iy_h_n;
  static CZ80OpProc f_set_po_iy_l_n;
#endif

#ifndef GAMEBOY_Z80
  static void f_set_po_ix_a_n(CZ80OpData *opData, uchar o);
  static void f_set_po_ix_b_n(CZ80OpData *opData, uchar o);
  static void f_set_po_ix_c_n(CZ80OpData *opData, uchar o);
  static void f_set_po_ix_d_n(CZ80OpData *opData, uchar o);
  static void f_set_po_ix_e_n(CZ80OpData *opData, uchar o);
  static void f_set_po_ix_h_n(CZ80OpData *opData, uchar o);
  static void f_set_po_ix_l_n(CZ80OpData *opData, uchar o);
  static void f_set_po_iy_a_n(CZ80OpData *opData, uchar o);
  static void f_set_po_iy_b_n(CZ80OpData *opData, uchar o);
  static void f_set_po_iy_c_n(CZ80OpData *opData, uchar o);
  static void f_set_po_iy_d_n(CZ80OpData *opData, uchar o);
  static void f_set_po_iy_e_n(CZ80OpData *opData, uchar o);
  static void f_set_po_iy_h_n(CZ80OpData *opData, uchar o);
  static void f_set_po_iy_l_n(CZ80OpData *opData, uchar o);
#endif

  // sla

  static CZ80OpProc f_sla_a;
  static CZ80OpProc f_sla_b;
  static CZ80OpProc f_sla_c;
  static CZ80OpProc f_sla_d;
  static CZ80OpProc f_sla_e;
  static CZ80OpProc f_sla_h;
  static CZ80OpProc f_sla_l;
  static CZ80OpProc f_sla_p_hl;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_sla_po_ix;
  static CZ80OpProc f_sla_po_ix_a;
  static CZ80OpProc f_sla_po_ix_b;
  static CZ80OpProc f_sla_po_ix_c;
  static CZ80OpProc f_sla_po_ix_d;
  static CZ80OpProc f_sla_po_ix_e;
  static CZ80OpProc f_sla_po_ix_h;
  static CZ80OpProc f_sla_po_ix_l;
  static CZ80OpProc f_sla_po_iy;
  static CZ80OpProc f_sla_po_iy_a;
  static CZ80OpProc f_sla_po_iy_b;
  static CZ80OpProc f_sla_po_iy_c;
  static CZ80OpProc f_sla_po_iy_d;
  static CZ80OpProc f_sla_po_iy_e;
  static CZ80OpProc f_sla_po_iy_h;
  static CZ80OpProc f_sla_po_iy_l;
#endif

  // sll

#ifndef GAMEBOY_Z80
  static CZ80OpProc f_sll_a;
  static CZ80OpProc f_sll_b;
  static CZ80OpProc f_sll_c;
  static CZ80OpProc f_sll_d;
  static CZ80OpProc f_sll_e;
  static CZ80OpProc f_sll_h;
  static CZ80OpProc f_sll_l;
  static CZ80OpProc f_sll_p_hl;
#else
  static CZ80OpProc f_swap_a;
  static CZ80OpProc f_swap_b;
  static CZ80OpProc f_swap_c;
  static CZ80OpProc f_swap_d;
  static CZ80OpProc f_swap_e;
  static CZ80OpProc f_swap_h;
  static CZ80OpProc f_swap_l;
  static CZ80OpProc f_swap_p_hl;
#endif
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_sll_po_ix;
  static CZ80OpProc f_sll_po_ix_a;
  static CZ80OpProc f_sll_po_ix_b;
  static CZ80OpProc f_sll_po_ix_c;
  static CZ80OpProc f_sll_po_ix_d;
  static CZ80OpProc f_sll_po_ix_e;
  static CZ80OpProc f_sll_po_ix_h;
  static CZ80OpProc f_sll_po_ix_l;
  static CZ80OpProc f_sll_po_iy;
  static CZ80OpProc f_sll_po_iy_a;
  static CZ80OpProc f_sll_po_iy_b;
  static CZ80OpProc f_sll_po_iy_c;
  static CZ80OpProc f_sll_po_iy_d;
  static CZ80OpProc f_sll_po_iy_e;
  static CZ80OpProc f_sll_po_iy_h;
  static CZ80OpProc f_sll_po_iy_l;
#endif

  // sra

  static CZ80OpProc f_sra_a;
  static CZ80OpProc f_sra_b;
  static CZ80OpProc f_sra_c;
  static CZ80OpProc f_sra_d;
  static CZ80OpProc f_sra_e;
  static CZ80OpProc f_sra_h;
  static CZ80OpProc f_sra_l;
  static CZ80OpProc f_sra_p_hl;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_sra_po_ix;
  static CZ80OpProc f_sra_po_ix_a;
  static CZ80OpProc f_sra_po_ix_b;
  static CZ80OpProc f_sra_po_ix_c;
  static CZ80OpProc f_sra_po_ix_d;
  static CZ80OpProc f_sra_po_ix_e;
  static CZ80OpProc f_sra_po_ix_h;
  static CZ80OpProc f_sra_po_ix_l;
  static CZ80OpProc f_sra_po_iy;
  static CZ80OpProc f_sra_po_iy_a;
  static CZ80OpProc f_sra_po_iy_b;
  static CZ80OpProc f_sra_po_iy_c;
  static CZ80OpProc f_sra_po_iy_d;
  static CZ80OpProc f_sra_po_iy_e;
  static CZ80OpProc f_sra_po_iy_h;
  static CZ80OpProc f_sra_po_iy_l;
#endif

  // srl

  static CZ80OpProc f_srl_a;
  static CZ80OpProc f_srl_b;
  static CZ80OpProc f_srl_c;
  static CZ80OpProc f_srl_d;
  static CZ80OpProc f_srl_e;
  static CZ80OpProc f_srl_h;
  static CZ80OpProc f_srl_l;
  static CZ80OpProc f_srl_p_hl;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_srl_po_ix;
  static CZ80OpProc f_srl_po_ix_a;
  static CZ80OpProc f_srl_po_ix_b;
  static CZ80OpProc f_srl_po_ix_c;
  static CZ80OpProc f_srl_po_ix_d;
  static CZ80OpProc f_srl_po_ix_e;
  static CZ80OpProc f_srl_po_ix_h;
  static CZ80OpProc f_srl_po_ix_l;
  static CZ80OpProc f_srl_po_iy;
  static CZ80OpProc f_srl_po_iy_a;
  static CZ80OpProc f_srl_po_iy_b;
  static CZ80OpProc f_srl_po_iy_c;
  static CZ80OpProc f_srl_po_iy_d;
  static CZ80OpProc f_srl_po_iy_e;
  static CZ80OpProc f_srl_po_iy_h;
  static CZ80OpProc f_srl_po_iy_l;
#endif

  // sub

  static CZ80OpProc f_sub_a;
  static CZ80OpProc f_sub_b;
  static CZ80OpProc f_sub_c;
  static CZ80OpProc f_sub_d;
  static CZ80OpProc f_sub_e;
  static CZ80OpProc f_sub_h;
  static CZ80OpProc f_sub_l;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_sub_ixh;
  static CZ80OpProc f_sub_ixl;
  static CZ80OpProc f_sub_iyh;
  static CZ80OpProc f_sub_iyl;
#endif
  static CZ80OpProc f_sub_p_hl;
  static CZ80OpProc f_sub_n;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_sub_po_ix;
  static CZ80OpProc f_sub_po_iy;
#endif

  // xor

  static CZ80OpProc f_xor_a;
  static CZ80OpProc f_xor_b;
  static CZ80OpProc f_xor_c;
  static CZ80OpProc f_xor_d;
  static CZ80OpProc f_xor_e;
  static CZ80OpProc f_xor_h;
  static CZ80OpProc f_xor_l;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_xor_ixh;
  static CZ80OpProc f_xor_ixl;
  static CZ80OpProc f_xor_iyh;
  static CZ80OpProc f_xor_iyl;
#endif
  static CZ80OpProc f_xor_p_hl;
  static CZ80OpProc f_xor_n;
#ifndef GAMEBOY_Z80
  static CZ80OpProc f_xor_po_ix;
  static CZ80OpProc f_xor_po_iy;
#endif

  static CZ80OpProc f_err;

  static CZ80OpProc f_invalid;

  // --------

  // Debug

 public:
  static std::string getRegisterName(uint reg );
  static std::string getFlagName    (uint flag);

  uchar  getRegValue8(uint reg);
  ushort getRegValue16(uint reg);
  ushort getPRegValue16(uint reg);

  void printState(std::ostream &os=std::cout);

  void printRegValue8(std::ostream &os, uint reg);
  void printRegValue16AndPtr(std::ostream &os, uint reg);

  std::string getFlagsString();

  // ------

  bool lookupOp(CZ80Op *op, CZ80Op **op1);

  static CZ80Op *getIndOp(uint ind);

  // ------

  static std::string hexString(const std::string &prefix, uchar);
  static std::string hexString(const std::string &prefix, ushort);

  static std::string hexString(uchar);
  static std::string hexString(ushort);

  // ------

  // Op Counts

  void dumpOpCounts(std::ostream &os);
  void resetOpCounts();

  // --------

  void tracePC(int d, ushort to);
  void traceBack();

  int traceNum() const { return pcBuffer_.numValues(); }

  ushort traceValue(int i) const { return pcBuffer_.getValue(i); }

  // ------

  // Snapshot

 public:
  bool saveSnapshot(const std::string &filename="snapshot.z80");

  bool loadSnapshot(const std::string &filename);
  bool loadSnapshot(CFile *file);

  // ------

 public:
  ulong getCPUHz() const { return cpuHz_; }
  void setCPUHz(ulong v) { cpuHz_ = v; }

  ulong getScreenHz() const { return screenHtz_; }
  void setScreenHz(ulong v) { screenHtz_ = v; }

  int msCycles(ulong ms) { return ms*1000/getCPUHz(); }

 private:
  typedef std::map<std::string,CZ80Macro> MacroMap;
  typedef CircBufferT<ushort>             PCBuffer;

  bool halt_            { false };
  bool stop_            { false };
  bool allowInterrupts_ { true };

  uchar         *memory_ { nullptr };
//uchar         *flags_  { nullptr };
  MemFlagsArray  memFlagsArray_;
  CZ80Registers  registers_;

  PCBuffer pcBuffer_;

  ulong  cpuHz_     { 4194304 };
  ushort screenHtz_ { 50 };
  ushort ifreq_     { 1 };
  ulong  t_         { 0 };
  ushort im0_       { 0x38 };
  ushort im2_       { 0xfe };

  ushort load_pos_ { 0 };
  uint   load_len_ { 65536 };

  bool verbose_ { false };

  CZ80ExecData  *execData_  { nullptr };
  CZ80MemData   *memData_   { nullptr };
  CZ80PortData  *portData_  { nullptr };
  CZ80Screen    *screen_    { nullptr };
  CZ80RstData   *rstData_   { nullptr };
  CZ80DebugData *debugData_ { nullptr };
  CZ80SpeedData *speedData_ { nullptr };

  bool   dump_      { false };
  CFile *dump_file_ { nullptr };

  CZ80AssembleData assembleData_;
  MacroMap         assembleMacros_;
  CZ80LabelData    labelData_;
  CZ80OpData*      opData_;
};

#endif
