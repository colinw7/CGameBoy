#ifndef CQHexdump_H
#define CQHexdump_H

#include <QFrame>

class CQHexdumpText;

class CQHexdumpDataIFace {
 public:
  CQHexdumpDataIFace() { }

  virtual ~CQHexdumpDataIFace() { }

  virtual std::size_t offset() const { return 0; }

  virtual std::size_t size() const = 0;

  virtual uchar data(uint) const = 0;
};

//---

class CQHexdumpData : public CQHexdumpDataIFace {
 public:
  CQHexdumpData(std::size_t size);
  CQHexdumpData(const uchar* data, std::size_t size);

 ~CQHexdumpData();

  std::size_t size() const override { return size_; }

  uchar data(uint i) const override { return data_[i]; }

  uchar *data() const { return data_; }

 public:
  uchar*      data_ { nullptr };
  std::size_t size_ { 0 };
};

//---

class CQHexdump : public QFrame {
  Q_OBJECT

  Q_PROPERTY(QString filename      READ filename)
  Q_PROPERTY(uint    width         READ width         WRITE setWidth)
  Q_PROPERTY(bool    showAddress   READ showAddress   WRITE setShowAddress)
  Q_PROPERTY(bool    showAscii     READ showAscii     WRITE setShowAscii)
  Q_PROPERTY(bool    showPosition  READ showPosition  WRITE setShowPosition)
  Q_PROPERTY(uint    addrWidth     READ addrWidth     WRITE setAddrWidth)
  Q_PROPERTY(uint    position      READ position      WRITE setPosition)
  Q_PROPERTY(QColor  positionColor READ positionColor WRITE setPositionColor)

 public:
  struct MemRange {
    uint pos;
    uint len;

    MemRange(uint pos, uint len) :
     pos(pos), len(len) {
    }

    bool contains(uint pos1, uint len1) const {
      return ! (pos1 > (pos + len - 1) || pos1 + len1 - 1 < pos);
    }

    friend bool operator<(const MemRange &lhs, const MemRange &rhs) {
      if (lhs.pos < rhs.pos) return true;
      if (lhs.pos > rhs.pos) return false;

      return (lhs.len < rhs.len);
    }
  };

 public:
  CQHexdump(QWidget *parent=nullptr);
 ~CQHexdump();

  const QString &filename() const { return filename_; }

  uint width() const { return width_; }
  void setWidth(uint w);

  bool showAddress() const { return showAddress_; }
  void setShowAddress(bool b);

  bool showAscii() const { return showAscii_; }
  void setShowAscii(bool b);

  bool showPosition() const { return showPosition_; }
  void setShowPosition(bool b);

  uint addrWidth() const { return addrWidth_; }
  void setAddrWidth(uint v);

  uint position() const { return position_; }
  void setPosition(uint pos);

  const QColor &positionColor() const { return positionColor_; }
  void setPositionColor(const QColor &c) { positionColor_ = c; }

  void setFont(const QFont &font);

  void resetMemColors();

  void addMemColor(uint pos, uint len, const QColor &c);

  bool getMemColor(uint pos, uint len, QColor &c) const;

  bool loadFile(const QString &filename);

  void loadData(const uchar *data, std::size_t size);

  CQHexdumpDataIFace *data() const { return data_; }
  void setData(CQHexdumpDataIFace *data);

  void updateText();

  QSize sizeHint() const override;

 private:
  typedef std::map<MemRange,QColor> MemColors;

  QString             filename_;
  CQHexdumpDataIFace* data_          { nullptr };
  uint                width_         { 16 };
  bool                showAddress_   { false };
  bool                showAscii_     { true };
  bool                showPosition_  { true };
  uint                addrWidth_     { 4 };
  uint                position_      { 0 };
  QColor              positionColor_ { 220, 0, 0 };
  CQHexdumpText*      text_          { nullptr };
  MemColors           memColors_;
};

//---

#if 0
#include <QTextEdit>

class CQHexdumpText : public QTextEdit {
  Q_OBJECT

 public:
  CQHexdumpText(CQHexdump *hexdump);

 private:
  CQHexdump *hexdump_ { nullptr };
};
#endif

//---

class QScrollBar;

class CQHexdumpText : public QFrame {
  Q_OBJECT

 public:
  CQHexdumpText(CQHexdump *hexdump);

  void updateText();
  void updateScrollBars();

  void resizeEvent(QResizeEvent *) override;

  void contextMenuEvent(QContextMenuEvent *event) override;

  void paintEvent(QPaintEvent *) override;

 public slots:
  void dumpSlot();
  void dumpBytesSlot();

 private:
  QString dataLine(int i) const;

  void dataLine(int i, QString &addrStr, QString &hexStr, QString &asciiStr) const;

  QString dataBytesLine(int i) const;

 private:
  CQHexdump*  hexdump_ { nullptr };
  QScrollBar* hbar_    { nullptr };
  QScrollBar* vbar_    { nullptr };
  int         vw_      { 0 };
  int         hh_      { 0 };
};

#endif
