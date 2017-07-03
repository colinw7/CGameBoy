#include <CZ80Disassemble.h>
#include <CZ80OpData.h>

bool
CZ80::
disassemble(std::ostream &os)
{
  return disassemble(load_pos_, load_len_, os);
}

bool
CZ80::
disassemble(ushort pos, ushort len, std::ostream &os)
{
  ushort pos1 = pos;
  ushort pos2 = pos + len;

  std::string str;
  ushort      len1, i;

  CZ80OpData op_data;

  //----

  ushort addr = 0;

  uint id = 1;

  // parse labels addresses
  while (pos1 < pos2) {
    readOpData(pos1, &op_data);

    if (! op_data.op)
      continue;

    int epos = pos1 + op_data.op->len;

    if      (op_data.op->id == OP_JP || op_data.op->id == OP_CALL) {
      if      (op_data.op->type1 == A_NUM)
        addr = op_data.getUWord1();
      else if (op_data.op->type2 == A_NUM)
        addr = op_data.getUWord2();
      else
        continue;

      if (! getValueLabel(addr, str)) {
        setLabelValue("LABEL_" + CStrUtil::toString(id), addr);

        ++id;
      }
    }
    else if (op_data.op->id == OP_JR || op_data.op->id == OP_DJNZ) {
      if      (op_data.op->type1 == A_S_NUM)
        addr = epos + op_data.getSByte1();
      else if (op_data.op->type2 == A_S_NUM)
        addr = epos + op_data.getSByte2();

      if (! getValueLabel(addr, str)) {
        setLabelValue("LABEL_" + CStrUtil::toString(id), addr);

        ++id;
      }
    }

    pos1 += op_data.op->len;
  }

  //----

  // disassemble instructions
  pos1 = pos;

  os << "  ORG  0x" << CStrUtil::toHexString(pos1, 4) << std::endl;

  os << std::endl;

  while (pos1 < pos2) {
    if (getValueLabel(pos1, str))
      os << str << ":" << std::endl;

    //----

    readOpData(pos1, &op_data);

    if (! op_data.op)
      continue;

    int epos = pos1 + op_data.op->len;

    //----

    str = op_data.getOpString(pos1);

    os << "  " << str;

    for (i = str.size(); i < 18; ++i)
      os << " ";

    os << " ; ";

    os << CStrUtil::toHexString(pos1, 4);

    len1 = 4;

    for (i = pos1; i < epos; ++i) {
      os << " " << CStrUtil::toHexString(getByte(i), 2);

      len1 += 3;
    }

    for ( ; len1 < 18; ++len1)
      os << " ";

    os << std::endl;

    //----

    pos1 += op_data.op->len;
  }

  return true;
}
