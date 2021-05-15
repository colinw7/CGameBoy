#include <CQHexdump.h>
#include <CFile.h>

#include <QScrollBar>
#include <QVBoxLayout>
#include <QMenu>
#include <QContextMenuEvent>
#include <QPainter>
#include <QStyle>

CQHexdump::
CQHexdump(QWidget *parent) :
 QFrame(parent)
{
  setObjectName("hexdump");

  QVBoxLayout *layout = new QVBoxLayout(this);

  layout->setMargin(0); layout->setSpacing(0);

  text_ = new CQHexdumpText(this);

  layout->addWidget(text_);

  //---

  QFont font("Courier New", 12);

  setFont(font);
}

CQHexdump::
~CQHexdump()
{
  delete data_;
}

void
CQHexdump::
setFont(const QFont &font)
{
  text_->setFont(font);
}

void
CQHexdump::
setWidth(uint w)
{
  if (w != width_) {
    width_ = w;

    updateText();
  }
}

void
CQHexdump::
setShowAddress(bool b)
{
  if (b != showAddress_) {
    showAddress_ = b;

    updateText();
  }
}

void
CQHexdump::
setShowAscii(bool b)
{
  if (b != showAscii_) {
    showAscii_ = b;

    updateText();
  }
}

void
CQHexdump::
setShowPosition(bool b)
{
  if (b != showPosition_) {
    showPosition_ = b;

    updateText();
  }
}

void
CQHexdump::
setAddrWidth(uint w)
{
  if (w != addrWidth_) {
    addrWidth_ = w;

    updateText();
  }
}

void
CQHexdump::
setPosition(uint pos)
{
  if (pos != position_) {
    position_ = pos;

    updateText();
  }
}

void
CQHexdump::
resetMemColors()
{
  memColors_.clear();
}

void
CQHexdump::
addMemColor(uint pos, uint len, const QColor &c)
{
  memColors_[MemRange(pos, len)] = c;
}

bool
CQHexdump::
getMemColor(uint pos, uint len, QColor &c) const
{
  for (const auto &memRange : memColors_) {
    if (memRange.first.contains(pos, len)) {
      c = memRange.second;
      return true;
    }
  }

  return false;
}

bool
CQHexdump::
loadFile(const QString &filename)
{
  if (filename == "")
    return false;

  CFile file(filename.toLatin1().constData());

  if (! file.isRegular())
    return false;

  //---

  filename_ = filename;

  CQHexdumpData *hexData = new CQHexdumpData(file.getSize());

  file.read(hexData->data(), hexData->size());

  setData(hexData);

  return true;
}

void
CQHexdump::
loadData(const uchar *data, std::size_t size)
{
  filename_ = "";

  CQHexdumpData *hexData = new CQHexdumpData(data, size);

  setData(hexData);
}

void
CQHexdump::
setData(CQHexdumpDataIFace *data)
{
  delete data_;

  data_ = data;

  updateText();
}

void
CQHexdump::
updateText()
{
  text_->updateText();
}

QSize
CQHexdump::
sizeHint() const
{
  int b = 2;

  QFontMetrics fm(font());

  int numLines = 25;

  int xw = fm.width("X");

  int w = 0;

  if (showAddress())
    w += 5*xw;

  w += (2*(width() + 1) - 1)*xw;

  if (showAscii())
    w += (width() + 1)*xw;

  return QSize(w + 2*b, numLines*fm.height() + 2*b);
}

//------

CQHexdumpData::
CQHexdumpData(std::size_t size)
{
  size_ = size;
  data_ = new uchar [size_];
}

CQHexdumpData::
CQHexdumpData(const uchar* data, std::size_t size)
{
  size_ = size;
  data_ = new uchar [size_];

  memcpy(data_, data, size_);
}

CQHexdumpData::
~CQHexdumpData()
{
  delete [] data_;
}

//------

CQHexdumpText::
CQHexdumpText(CQHexdump *hexdump) :
 hexdump_(hexdump), vw_(16), hh_(16)
{
  setObjectName("text");

  vw_ = style()->pixelMetric(QStyle::PM_ScrollBarExtent);
  hh_ = style()->pixelMetric(QStyle::PM_ScrollBarExtent);

  setContentsMargins(0, 0, vw_, hh_);

  vbar_ = new QScrollBar(Qt::Vertical  , this);
  hbar_ = new QScrollBar(Qt::Horizontal, this);

  vbar_->setObjectName("vbar");
  hbar_->setObjectName("hbar");

  connect(vbar_, SIGNAL(valueChanged(int)), this, SLOT(update()));
  connect(hbar_, SIGNAL(valueChanged(int)), this, SLOT(update()));
}

void
CQHexdumpText::
updateText()
{
  updateScrollBars();

  update();
}

void
CQHexdumpText::
resizeEvent(QResizeEvent *)
{
  vbar_->move  (width() - vw_, 0);
  vbar_->resize(vw_, height() - hh_);

  hbar_->move  (0, height() - hh_);
  hbar_->resize(width() - vw_, hh_);

  updateScrollBars();
}

void
CQHexdumpText::
updateScrollBars()
{
  QFontMetrics fm(font());

  int tw = fm.width("X");
  int th = fm.height();

  //---

  uint charsPerLine = hexdump_->width();

  int lineChars = charsPerLine*3 - 1;

  if (hexdump_->showAddress())
    lineChars += 5;

  if (hexdump_->showAscii())
    lineChars += charsPerLine + 1;

  //---

  CQHexdumpDataIFace *data = hexdump_->data();

  if (data) {
    int numDataLines = (data->size() + charsPerLine - 1)/charsPerLine;

  //int numVisibleLines = (height() - hh_ + th - 1)/th;
  //int numVisibleChars = (width () - vw_ + tw - 1)/tw;
    int numVisibleLines = (height() - hh_)/th;
    int numVisibleChars = (width () - vw_)/tw;

    hbar_->setRange(0, lineChars - numVisibleChars);
    hbar_->setPageStep(numVisibleChars);

    vbar_->setRange(0, numDataLines - numVisibleLines);
    vbar_->setPageStep(numVisibleLines);
  }
  else {
    hbar_->setRange(0, 0);
    vbar_->setRange(0, 0);
  }
}

void
CQHexdumpText::
contextMenuEvent(QContextMenuEvent *event)
{
  QMenu *menu = new QMenu;

  QAction *dumpAction = menu->addAction("Dump");

  connect(dumpAction, SIGNAL(triggered()), this, SLOT(dumpSlot()));

  QAction *dumpBytesAction = menu->addAction("Dump Bytes");

  connect(dumpBytesAction, SIGNAL(triggered()), this, SLOT(dumpBytesSlot()));

  menu->exec(event->globalPos());

  delete menu;
}

void
CQHexdumpText::
dumpSlot()
{
  FILE *fp = fopen("memory.txt", "w");
  if (! fp) return;

  CQHexdumpDataIFace *data = hexdump_->data();

  int i = 0;

  uint charsPerLine = hexdump_->width();

  uint numDataLines = (data->size() + charsPerLine - 1)/charsPerLine;

  for (uint iy = 0; iy < numDataLines; ++iy) {
    if (i >= int(data->size()))
      break;

    QString line = dataLine(i);

    fprintf(fp, "%s\n", line.toLatin1().constData());

    i += charsPerLine;
  }

  fclose(fp);
}

void
CQHexdumpText::
dumpBytesSlot()
{
  FILE *fp = fopen("memory.txt", "w");
  if (! fp) return;

  CQHexdumpDataIFace *data = hexdump_->data();

  int i = 0;

  uint charsPerLine = hexdump_->width();

  uint numDataLines = (data->size() + charsPerLine - 1)/charsPerLine;

  for (uint iy = 0; iy < numDataLines; ++iy) {
    if (i >= int(data->size()))
      break;

    QString line = dataBytesLine(i);

    fprintf(fp, "%s\n", line.toLatin1().constData());

    i += charsPerLine;
  }

  fclose(fp);
}

void
CQHexdumpText::
paintEvent(QPaintEvent *)
{
  QColor bg, fg;

  if (isEnabled()) {
    bg = palette().window    ().color();
    fg = palette().windowText().color();
  }
  else {
    bg = Qt::white;
    fg = palette().color(QPalette::Disabled, QPalette::WindowText);
  }

  QPainter painter(this);

  painter.fillRect(rect(), bg);

  CQHexdumpDataIFace *data = hexdump_->data();

  if (! data)
    return;

  QFontMetrics fm(font());

  int tw = fm.width("X");
  int th = fm.height();

  uint numVisibleLines = (height() - hh_ + th - 1)/th;

  uint charsPerLine = hexdump_->width();

  //---

  int lposl = 0;
  int lposr = lposl + (hexdump_->showAddress() ? tw*hexdump_->addrWidth() : 0);
  int mposl = lposr + (hexdump_->showAddress() ? tw : 0);
  int mposr = mposl + (charsPerLine*3 - 1)*tw;
  int rposl = mposr + tw;
//int rposr = rposl + (hexdump_->showAscii() ? charsPerLine*tw : 0);

  //---

  int xo = -hbar_->value()*tw;

  int x, y = 0;

  auto drawTextX = [&] (int x, const QString &str, const QColor &c) -> void {
    painter.setPen(c);

    int len = str.size();

    for (int i = 0; i < len; ++i) {
      painter.drawText(xo + x, y + fm.ascent(), str[i]);

      x += tw;
    }
  };

  auto drawText = [&] (const QString &str, const QColor &c) -> void {
    drawTextX(x, str, c);

    x += str.length()*tw;
  };

  //---

  uint i = vbar_->value()*charsPerLine;

  painter.setPen(fg);

  for (uint iy = 0; iy < numVisibleLines; ++iy) {
    if (i >= data->size())
      break;

    x = lposl;

    if (isEnabled()) {
      QColor c;

      if (hexdump_->getMemColor(i, charsPerLine, c)) {
        painter.fillRect(QRect(xo + mposl, y, mposr - mposl, th), c);
      }

#if 0
      for (uint i = 0; i < charsPerLine; ++i)
        painter.drawRect(QRect(xo + x + 3*i*tw, y, 2*tw, th));
#endif

      if (hexdump_->position() >= i && hexdump_->position() < i + charsPerLine) {
        int i1 = hexdump_->position() - i;

        QString addrStr, hexStr, asciiStr;

        dataLine(i, addrStr, hexStr, asciiStr);

        if (addrStr.length())
          drawTextX(lposl, addrStr + " ", fg);

        QString lhexStr = hexStr.mid(0, i1*3);
        QString mhexStr = hexStr.mid(i1*3, 3);
        QString rhexStr = hexStr.mid((i1 + 1)*3);

        x = mposl;

        drawText(lhexStr, fg);
        drawText(mhexStr, hexdump_->positionColor());
        drawText(rhexStr, fg);

        if (asciiStr.length()) {
          QString lasciiStr = asciiStr.mid(0, i1);
          QString masciiStr = asciiStr.mid(i1, 1);
          QString rasciiStr = asciiStr.mid(i1 + 1);

          x = rposl;

          drawText(lasciiStr, fg);
          drawText(masciiStr, hexdump_->positionColor());
          drawText(rasciiStr, fg);
        }
      }
      else {
        QString addrStr, hexStr, asciiStr;

        dataLine(i, addrStr, hexStr, asciiStr);

        if (addrStr != "")
          drawTextX(lposl, addrStr, fg);

        drawTextX(mposl, hexStr, fg);

        if (asciiStr != "")
          drawTextX(rposl, asciiStr, fg);
      }
    }
    else {
      QString addrStr, hexStr, asciiStr;

      dataLine(i, addrStr, hexStr, asciiStr);

      if (addrStr != "")
        drawTextX(lposl, addrStr, fg);

      drawTextX(mposl, hexStr, fg);

      if (asciiStr != "")
        drawTextX(rposl, asciiStr, fg);
    }

    i += charsPerLine;
    y += th;
  }
}

QString
CQHexdumpText::
dataLine(int i) const
{
  QString addrStr, hexStr, asciiStr;

  dataLine(i, addrStr, hexStr, asciiStr);

  QString line;

  if (addrStr.length())
    line += addrStr + " ";

  line += hexStr;

  if (asciiStr.length())
    line += " " + asciiStr;

  return line;
}

void
CQHexdumpText::
dataLine(int i, QString &addrStr, QString &hexStr, QString &asciiStr) const
{
  CQHexdumpDataIFace *data = hexdump_->data();

  uint charsPerLine = hexdump_->width();

  QString line;

  if (hexdump_->showAddress()) {
    QString addrFmt = QString("%0") + QString("%1X").arg(hexdump_->addrWidth());

    addrStr.sprintf(addrFmt.toLatin1().constData(), uint(data->offset() + i));
  }

  for (uint ix = 0; ix < charsPerLine; ++ix) {
    if (ix > 0)
      hexStr += " ";

    if (i < int(data->size())) {
      uchar c = data->data(i);

      QString hexStr1;

      hexStr1.sprintf("%02X", c);

      hexStr += hexStr1;
    }
    else
      hexStr += "  ";

    ++i;
  }

  if (hexdump_->showAscii()) {
    i -= charsPerLine;

    for (uint ix = 0; ix < charsPerLine; ++ix) {
      if (i < int(data->size())) {
        uchar c = data->data(i);

        QString charStr;

        if      (c >= 0x20 && c <= 0x7e)
          charStr.sprintf("%c", c);
        else
          charStr.sprintf(".");

        asciiStr += charStr;
      }
      else
        asciiStr += " ";

      ++i;
    }
  }
}

QString
CQHexdumpText::
dataBytesLine(int i) const
{
  CQHexdumpDataIFace *data = hexdump_->data();

  uint charsPerLine = hexdump_->width();

  QString line;

  for (uint ix = 0; ix < charsPerLine; ++ix) {
    if (i < int(data->size())) {
      uchar c = data->data(i);

      QString hexStr1;

      hexStr1.sprintf("0x%02X,", c);

      line += hexStr1;
    }

    ++i;
  }

  return line;
}
