#include <CQGameBoy.h>
#include <CQGameBoyScreen.h>
#include <CQGameBoyVideo.h>
#include <CQGameBoyKeys.h>
#include <CQGameBoyInterrupt.h>
#include <CQGameBoyTimer.h>
#include <CQGameBoyDbg.h>

CQGameBoy::
CQGameBoy() :
 CGameBoy()
{
  screen_ = new CQGameBoyScreen(this);
}

CQGameBoyVideo *
CQGameBoy::
addVideo()
{
  if (! video_) {
    video_ = new CQGameBoyVideo(screen_);

    video_->setFixedFont(fixedFont_);
  }

  video_->show();
  video_->raise();

  return video_;
}

CQGameBoyKeys *
CQGameBoy::
addKeys()
{
  if (! keys_) {
    keys_ = new CQGameBoyKeys(this);

    //keys_->setFixedFont(fixedFont_);
  }

  keys_->show();
  keys_->raise();

  return keys_;
}

CQGameBoyInterrupt *
CQGameBoy::
addInterrupt()
{
  if (! interrupt_) {
    interrupt_ = new CQGameBoyInterrupt(this);

    //interrupt_->setFixedFont(fixedFont_);
  }

  interrupt_->show();
  interrupt_->raise();

  return interrupt_;
}

CQGameBoyTimer *
CQGameBoy::
addTimer()
{
  if (! timer_) {
    timer_ = new CQGameBoyTimer(this);

    //timer_->setFixedFont(fixedFont_);
  }

  timer_->show();
  timer_->raise();

  return timer_;
}

CQGameBoyDbg *
CQGameBoy::
addDebug()
{
  if (! dbg_) {
    dbg_ = new CQGameBoyDbg(this);

    dbg_->init();

    dbg_->setFixedFont(fixedFont_);
  }

  dbg_->show();
  dbg_->raise();

  return dbg_;
}

void
CQGameBoy::
setFixedFont(const QFont &font)
{
  fixedFont_ = font;

  if (video_)
    video_->setFixedFont(fixedFont_);

  if (dbg_)
    dbg_->setFixedFont(fixedFont_);
}

void
CQGameBoy::
updateKeys()
{
  if (keys_)
    keys_->update();
}

void
CQGameBoy::
keyPress(CKeyType key)
{
  CGameBoy::keyPress(key);

  if (keys_)
    keys_->update();
}

void
CQGameBoy::
keyRelease(CKeyType key)
{
  CGameBoy::keyRelease(key);

  if (keys_)
    keys_->update();
}
