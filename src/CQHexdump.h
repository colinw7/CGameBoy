#ifndef CQHexdump_H
#define CQHexdump_H

#include <QFrame>

class QTextEdit;

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

  Q_PROPERTY(QString filename    READ filename)
  Q_PROPERTY(uint    width       READ width       WRITE setWidth)
  Q_PROPERTY(bool    showAddress READ showAddress WRITE setShowAddress)
  Q_PROPERTY(bool    showAscii   READ showAscii   WRITE setShowAscii)

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

  void setFont(const QFont &font);

  bool loadFile(const QString &filename);

  void loadData(const uchar *data, std::size_t size);

  CQHexdumpDataIFace *data() const { return data_; }
  void setData(CQHexdumpDataIFace *data);

  void updateText();

  QSize sizeHint() const;

 private:
  QString             filename_;
  CQHexdumpDataIFace* data_        { nullptr };
  uint                width_       { 16 };
  bool                showAddress_ { false };
  bool                showAscii_   { true };
  QTextEdit*          text_        { nullptr };
};

#endif
