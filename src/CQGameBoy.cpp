#include <CQGameBoy.h>
#include <CQGameBoyScreen.h>
#include <CQGameBoyVideo.h>
#include <CQGameBoyKeys.h>
#include <CQGameBoyInterrupt.h>
#include <CQGameBoyTimer.h>
#include <CQGameBoyInfo.h>
#include <CQGameBoyDbg.h>

CQGameBoy::
CQGameBoy() :
 CGameBoy()
{
  setObjectName("gameBoy");
}

CQGameBoyScreen *
CQGameBoy::
screen() const
{
  return screen_;
}

void
CQGameBoy::
createScreen()
{
  assert(! screen_);

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

CQGameBoyInfo *
CQGameBoy::
addInfo()
{
  if (! info_) {
    info_ = new CQGameBoyInfo(this);

    //info_->setFixedFont(fixedFont_);
  }

  info_->show();
  info_->raise();

  return info_;
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
updateTiles()
{
  if (video_)
    video_->updateTiles();
}

void
CQGameBoy::
updateScreen()
{
  if (video_)
    video_->updateScreen();
}

void
CQGameBoy::
updateSprites()
{
  if (video_)
    video_->updateSprites();
}

void
CQGameBoy::
updatePalette()
{
  if (video_)
    video_->updatePalette();
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

void
CQGameBoy::
execStop(bool b)
{
  if (b)
    screen_->stopTimer();
}

QColor
CQGameBoy::
vramSpritePaletteColor(uchar palette, uchar ind) const
{
  uchar r, g, b;

  spritePaletteColor(palette, ind, r, g, b);

  return QColor(r, g, b);
}

QColor
CQGameBoy::
vramBgPaletteColor(uchar palette, uchar ind) const
{
  uchar r, g, b;

  bgPaletteColor(palette, ind, r, g, b);

  return QColor(r, g, b);
}

const QColor &
CQGameBoy::
mappedPaletteColor(uchar palette, uchar ind) const
{
  int ind1 = 0;

  switch (ind) {
    case 0: ind1 =  palette & 0x03      ; break;
    case 1: ind1 = (palette & 0x0c) >> 2; break;
    case 2: ind1 = (palette & 0x30) >> 4; break;
    case 3: ind1 = (palette & 0xc0) >> 6; break;
  }

  return paletteColor(ind1);
}

const QColor &
CQGameBoy::
paletteColor(uchar ind) const
{
  switch (ind) {
    default: assert(false);
    case 0: return color0();
    case 1: return color1();
    case 2: return color2();
    case 3: return color3();
  }
}
