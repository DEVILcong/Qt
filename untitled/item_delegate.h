#ifndef ITEM_DELEGATE_H
#define ITEM_DELEGATE_H

#include <QStyledItemDelegate>
#include <QModelIndex>
#include <QStandardItem>
#include <QPainter>


class item_delegate :public QStyledItemDelegate
{
    Q_OBJECT
public:
    item_delegate(QObject* parent);
    ~item_delegate();
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // ITEM_DELEGATE_H
