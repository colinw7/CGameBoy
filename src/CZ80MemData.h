#ifndef CZ80MemData_H
#define CZ80MemData_H

class CZ80MemData {
 public:
  CZ80MemData(CZ80 &z80) :
   z80_(z80) {
  }

  virtual ~CZ80MemData() { }

  virtual bool memRead(uchar *c, ushort pos, ushort len);

  virtual void memWrite(const uchar *c, ushort pos, ushort len);

  virtual bool memTrigger(const uchar *c, ushort pos, ushort len);

 protected:
  CZ80 &z80_;
};

#endif
