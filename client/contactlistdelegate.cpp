#include "contactlistdelegate.h"
#include "mainwindow.h"  
#include <QPainter>
#include <QApplication>

ContactListDelegate::ContactListDelegate(
    const QMap<QString, User> *userCache,
    const QMap<QString, ChatCache> *chatCache,
    const QMap<QString, int> *unreadCounts,
    const QString *currentChatUsername,
    QObject *parent)
    : QStyledItemDelegate(parent),
    m_userCache(userCache),
    m_chatCache(chatCache),
    m_unreadCounts(unreadCounts),
    m_currentChatUsername(currentChatUsername)
{}

void ContactListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

     
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(option.rect, option.palette.color(QPalette::AlternateBase));
    }

     
    QString username = index.data(Qt::UserRole).toString();
    if (!m_userCache->contains(username)) {
        painter->restore();
        return;
    }
    const User& user = m_userCache->value(username);

     
    QString bottomText;
    QColor bottomTextColor = Qt::gray;
    bool isTyping = user.isTyping;
    if (username == *m_currentChatUsername) {
        isTyping = false;  
    }

    if (isTyping ) {
        bottomText = "печатает...";
        bottomTextColor = QColor("#F4ABC4");  
    } else if (m_chatCache->contains(username) && !m_chatCache->value(username).messages.isEmpty()) {
        bottomText = m_chatCache->value(username).messages.last().payload;
    }

     
    int padding = 12;
    QRect contentRect = option.rect.adjusted(padding, padding / 2, -padding, -padding / 2);
    QRect topRect(contentRect.left(), contentRect.top(), contentRect.width(), contentRect.height() / 2);
    QRect bottomRect(contentRect.left(), contentRect.top() + contentRect.height() / 2, contentRect.width(), contentRect.height() / 2);

     
    QFont nameFont = painter->font();
    nameFont.setBold(user.isOnline);
    painter->setFont(nameFont);
    painter->setPen(user.isOnline ? Qt::white : QColor("#EAEAEA"));
    painter->drawText(topRect, Qt::AlignLeft | Qt::AlignBottom, user.displayName);

     
    QFontMetrics fm(painter->font());
    QString elidedBottomText = fm.elidedText(bottomText, Qt::ElideRight, bottomRect.width());
    painter->setFont(QApplication::font());  
    painter->setPen(bottomTextColor);
    painter->drawText(bottomRect, Qt::AlignLeft | Qt::AlignTop, elidedBottomText);

     
    int count = m_unreadCounts->value(username, 0);
    if (count > 0) {
        int badgeSize = 20;
        QColor badgeColor = option.palette.highlight().color();
        QColor textColor = option.palette.highlightedText().color();
        QRect badgeRect(
            option.rect.right() - badgeSize - padding,
            option.rect.top() + (option.rect.height() - badgeSize) / 2,
            badgeSize, badgeSize
            );
        painter->setBrush(badgeColor);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(badgeRect);
        painter->setPen(textColor);
        painter->setFont(QFont("Segoe UI", 8, QFont::Bold));
        painter->drawText(badgeRect, Qt::AlignCenter, QString::number(count));
    }

    painter->restore();
}

QSize ContactListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
     
    Q_UNUSED(option);
    Q_UNUSED(index);
    QFontMetrics fm(QApplication::font());
    int singleLineHeight = fm.height();
    return QSize(200, singleLineHeight * 2 + 16);  
}
