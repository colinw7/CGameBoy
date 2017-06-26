#include <CQGameBoyKeys.h>
#include <CQGameBoy.h>
#include <CQUtil.h>
#include <QPainter>

#include <iostream>

CQGameBoyKeys::
CQGameBoyKeys(CQGameBoy *gameboy) :
 gameboy_(gameboy)
{
  setObjectName("keys");

  setWindowTitle("GameBoy Keys");
}

CQGameBoyKeys::
~CQGameBoyKeys()
{
}

void
CQGameBoyKeys::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  QFontMetrics fm(font());

  uchar key0 = gameboy()->key0();
  uchar key1 = gameboy()->key1();

  painter.fillRect(rect(), Qt::white);

  int border = 4;

  int bx  = fm.ascent();
  int by  = fm.ascent();
  int dy  = fm.height() + 2;
  int cw1 = fm.width("Right");

  uchar sel = gameboy()->keySel();

  int x = cw1 + 2*border;
  int y = border;

  drawKey(&painter, QRect(x, y, bx, by), sel == 0x10); x += bx + border;
  drawKey(&painter, QRect(x, y, bx, by), sel == 0x20);

  x = border;
  y = dy + fm.ascent() + border;

  painter.setPen(Qt::black);

  painter.drawText(x, y, "Right"); y += dy;
  painter.drawText(x, y, "Left" ); y += dy;
  painter.drawText(x, y, "Up"   ); y += dy;
  painter.drawText(x, y, "Down" ); y += dy;

  x = cw1 + 4*border + 2*bx;
  y = dy + fm.ascent() + border;

  painter.drawText(x, y, "A"     ); y += dy;
  painter.drawText(x, y, "B"     ); y += dy;
  painter.drawText(x, y, "Select"); y += dy;
  painter.drawText(x, y, "Start" ); y += dy;

  x = cw1 + 2*border;
  y = dy + border;

  drawKey(&painter, QRect(x, y, bx, by), TST_BIT(key1, 0)); y += dy;
  drawKey(&painter, QRect(x, y, bx, by), TST_BIT(key1, 1)); y += dy;
  drawKey(&painter, QRect(x, y, bx, by), TST_BIT(key1, 2)); y += dy;
  drawKey(&painter, QRect(x, y, bx, by), TST_BIT(key1, 3)); y += dy;

  x = cw1 + 3*border + bx;
  y = dy + border;

  drawKey(&painter, QRect(x, y, bx, by), TST_BIT(key0, 0)); y += dy;
  drawKey(&painter, QRect(x, y, bx, by), TST_BIT(key0, 1)); y += dy;
  drawKey(&painter, QRect(x, y, bx, by), TST_BIT(key0, 2)); y += dy;
  drawKey(&painter, QRect(x, y, bx, by), TST_BIT(key0, 3)); y += dy;
}

void
CQGameBoyKeys::
drawKey(QPainter *painter, const QRect &rect, bool on)
{
  painter->fillRect(rect, (on ? Qt::red : Qt::green));
}

QSize
CQGameBoyKeys::
sizeHint() const
{
  return QSize(256, 256);
}
