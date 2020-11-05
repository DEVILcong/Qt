#include "item_delegate.h"

item_delegate::item_delegate(QObject* parent) :QStyledItemDelegate(parent)
{

}

item_delegate::~item_delegate(){

}

void item_delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const{
    if(index.isValid()){
        painter->save();

        QVariant raw_item = index.data(Qt::UserRole);
        QString name = raw_item.toString();
        raw_item = index.data(Qt::UserRole + 1);
        int has_info = raw_item.toInt();


        QRectF rect;
        rect.setX(option.rect.x());
        rect.setY(option.rect.y());
        rect.setWidth(option.rect.width()-1);
        rect.setHeight(option.rect.height()-1);

        const qreal redius = 7;
        QPainterPath path;

        path.moveTo(rect.topLeft() + QPointF(redius, 0));
        path.lineTo(rect.topRight() - QPointF(redius, 0));
        path.quadTo(rect.topRight(), rect.topRight() + QPointF(0, redius));
        path.lineTo(rect.bottomRight() - QPointF(0, redius));
        path.quadTo(rect.bottomRight(), rect.bottomRight() - QPointF(redius, 0));
        path.lineTo(rect.bottomLeft() + QPointF(redius, 0));
        path.quadTo(rect.bottomLeft(), rect.bottomLeft() - QPointF(0, redius));
        path.lineTo(rect.topLeft() + QPointF(0, redius));
        path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(redius, 0));

        if(option.state.testFlag(QStyle::State_Selected)){
            painter->setPen(QColor(Qt::blue));
            painter->setBrush(QColor(229, 241, 255));
            painter->drawPath(path);
        }else if(has_info == 1){
            painter->setPen(QColor(Qt::gray));
            painter->setBrush(QColor(255, 99, 71));
            painter->drawPath(path);
        }else{
            painter->setPen(Qt::gray);
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(path);
        }

        painter->setPen(QColor(Qt::black));
        painter->setFont(QFont("Times", 12, QFont::Bold));
        painter->drawText(rect, Qt::AlignCenter, name);

        painter->restore();
    }
}

QSize item_delegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const{
    return QSize(80, 30);
}
