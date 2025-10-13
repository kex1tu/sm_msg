#include "contactlistdelegate.h"
#include <QPainter>
#include <QApplication>  

ContactListDelegate::ContactListDelegate(const QMap<QString, int> *unreadCounts, QObject *parent)
    : QStyledItemDelegate(parent), m_unreadCounts(unreadCounts)
{}

void ContactListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
     
    QStyledItemDelegate::paint(painter, option, index);

     
    QString username = index.data(Qt::UserRole).toString();
    int count = m_unreadCounts->value(username, 0);

     
    if (count > 0) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

         
        int badgeSize = 20;
        int margin = 8;
        QColor badgeColor = option.palette.highlight().color();  
        QColor textColor = option.palette.highlightedText().color();

         
        QRect badgeRect = QRect(
            option.rect.right() - badgeSize - margin,
            option.rect.top() + (option.rect.height() - badgeSize) / 2,
            badgeSize,
            badgeSize
            );

         
        painter->setBrush(badgeColor);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(badgeRect);

        painter->setPen(textColor);
        painter->setFont(QFont("Segoe UI", 8, QFont::Bold));
        painter->drawText(badgeRect, Qt::AlignCenter, QString::number(count));

        painter->restore();
    }
}
