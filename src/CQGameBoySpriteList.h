#ifndef CQGameBoySpriteList_H
#define CQGameBoySpriteList_H

#include <QListWidget>

class CQGameBoyVideo;
class CQGameBoySpriteItem;

class CQGameBoySpriteList : public QListWidget {
  Q_OBJECT

 public:
  CQGameBoySpriteList(CQGameBoyVideo *video);

  CQGameBoyVideo *video() const { return video_; }

  void update();

 private:
  typedef std::vector<CQGameBoySpriteItem *> Items;

  CQGameBoyVideo *video_ { nullptr };
  Items           items_;
};

#endif
