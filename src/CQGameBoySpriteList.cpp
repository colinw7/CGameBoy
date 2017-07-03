#include <CQGameBoySpriteList.h>
#include <CQGameBoyVideo.h>
#include <CQGameBoyScreen.h>
#include <CQGameBoy.h>
#include <QItemDelegate>

class CQGameBoySpriteItem : public QListWidgetItem {
 public:
  CQGameBoySpriteItem(CQGameBoySpriteList *list, int ind) :
   list_(list), ind_(ind) {
  }

  int ind() const { return ind_; }

 private:
  CQGameBoySpriteList *list_ { nullptr };
  int                  ind_  { 0 };
};

//---

class CQGameBoySpriteDelegate : public QItemDelegate {
 public:
  CQGameBoySpriteDelegate(CQGameBoySpriteList *list) :
   list_(list) {
  }

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const {
    const QChar UpArrow   (0x2191);
    const QChar DownArrow (0x2193);
    const QChar LeftArrow (0x2190);
    const QChar RightArrow(0x2192);

    QItemDelegate::drawBackground(painter, option, index);

    CQGameBoySpriteItem *item = static_cast<CQGameBoySpriteItem *>(list_->item(index.row()));

    CGameBoySprite sprite;

    list_->video()->screen()->gameboy()->getSprite(item->ind(), sprite);

    QString text = QString("%1) x:%2 y:%3").arg(sprite.i).arg(sprite.x).arg(sprite.y);

    text += QString(" T:%2 X:%3 Y:%4").arg(sprite.t).
                     arg(sprite.xflip ? LeftArrow : RightArrow).
                     arg(sprite.yflip ? UpArrow   : DownArrow );

    QItemDelegate::drawDisplay(painter, option, option.rect, text);
  }

  QSize sizeHint(const QStyleOptionViewItem &item, const QModelIndex &index) const {
    //CQGameBoySpriteItem *item = static_cast<CQGameBoySpriteItem *>(list_->item(index.row()));

    return QItemDelegate::sizeHint(item, index);
  }

 private:
  CQGameBoySpriteList *list_ { nullptr };
};

//---

CQGameBoySpriteList::
CQGameBoySpriteList(CQGameBoyVideo *video) :
 QListWidget(), video_(video)
{
  setItemDelegate(new CQGameBoySpriteDelegate(this));

  for (int i = 0; i < 40; ++i) {
    CQGameBoySpriteItem *item = new CQGameBoySpriteItem(this, i);

    items_.push_back(item);

    addItem(item);
  }
}

void
CQGameBoySpriteList::
update()
{
  viewport()->update();

  //for (auto &item : items_)
  //  item->update();
}
