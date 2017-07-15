#include <CQHexdump.h>
#include <CFile.h>

#include <QTextEdit>
#include <QVBoxLayout>

CQHexdump::
CQHexdump(QWidget *parent) :
 QFrame(parent)
{
  setObjectName("hexdump");

  QVBoxLayout *layout = new QVBoxLayout(this);

  layout->setMargin(2); layout->setSpacing(2);

  text_ = new QTextEdit;

  text_->setObjectName("text");
  text_->setReadOnly(true);
  text_->setWordWrapMode(QTextOption::NoWrap);

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
  text_->clear();

  if (! data_)
    return;

  QString addr_line, hex_line, char_line;

  uint x = 0;

  for (uint i = 0; i < data_->size(); ++i) {
    uchar c = data_->data(i);

    QString hex_str, char_str;

    hex_str.sprintf("%02X", c);

    if      (c >= 0x20 && c <= 0x7e)
      char_str.sprintf("%c", c);
    else
      char_str.sprintf(".");

    if (x == 0) {
      // flush last line
      if (hex_line != "") {
        QString line;

        if (showAddress())
          line += addr_line + " ";

        line += hex_line;

        if (showAscii())
          line += "  " + char_line;

        text_->append(line);

        addr_line = "";
        hex_line  = "";
        char_line = "";
      }

      if (showAddress())
        addr_line.sprintf("%04X", uint(data_->offset() + i));
      else
        addr_line = "";
    }

    if (x > 0)
      hex_line += " ";

    hex_line  += hex_str;
    char_line += char_str;

    ++x;

    if (x == width())
      x = 0;
  }

  if (x > 0) {
    while (x < width()) {
      hex_line += "   ";

      ++x;
    }

    QString line;

    if (showAddress())
      line += addr_line + " ";

    line += hex_line;

    if (showAscii())
      line += "  " + char_line;

    text_->append(line);
  }
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

  w += 2*(width() + 1)*xw;

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
