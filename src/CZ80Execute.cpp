#include <CZ80Execute.h>

bool
CZ80::
execute()
{
  return execute(load_pos_);
}

bool
CZ80::
execute(ushort pos)
{
  setPC(pos);

  if (debugData_)
    debugData_->callInitProcs();

  if (speedData_)
    speedData_->init();

  cont();

  if (debugData_)
    debugData_->callTermProcs();

  return true;
}

bool
CZ80::
next()
{
  ushort pc   = getPC();
  bool   halt = getHalt();

  CZ80OpData op_data;

  readOpData(&op_data);

  ushort pc1 = getPC();

  setPC  (pc);
  setHalt(halt);

  addBreakpoint(pc1);

  cont();

  removeBreakpoint(getPC());

  return true;
}

bool
CZ80::
cont()
{
  execute1(true);

  return true;
}

bool
CZ80::
execute1(bool notify)
{
  while (true) {
    step1(notify);

    if (getHalt() || getStop())
      break;

    if (debugData_ && debugData_->isBreakpoint(getPC()))
      break;
  }

  setStop(false);

  return true;
}

bool
CZ80::
step()
{
  return step1(true);
}

bool
CZ80::
step1(bool notify)
{
  if (notify)
    callPreStepProcs();

  if (execData_)
    execData_->preStep();

  int r = 1;
  int t = 4;

  if (! getHalt()) {
    CZ80OpData op_data;

    readOpData(&op_data);

    if (dump_)
      op_data.dump(dump_file_);

    op_data.execute();

    r = op_data.op->r;
    t = op_data.op->t;
  }

  incR(r);
  incT(t);

  if (execData_)
    execData_->postStep();

  if (screen_)
    screen_->screenStep(t);

  if (notify)
    callPostStepProcs();

  return true;
}

void
CZ80::
stop()
{
  setStop(true);
}
