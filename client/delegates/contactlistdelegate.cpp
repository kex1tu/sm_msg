#include "contactlistdelegate.h"
#include "contactlistmodel.h"
#include <QPainter>
#include <QApplication>



ContactListDelegate::ContactListDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}


void ContactListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    // Стиль выделения, мышиного наведения — для UX
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(option.rect, option.palette.color(QPalette::AlternateBase));
    }

    // Основная информация о контакте
    QString displayName = index.data(Qt::DisplayRole).toString();
    QString lastMessage = index.data(ContactListModel::LastMessageRole).toString();
    bool isOnline = index.data(ContactListModel::IsOnlineRole).toBool();
    bool isTyping = index.data(ContactListModel::IsTypingRole).toBool();
    int unreadCount = index.data(ContactListModel::UnreadCountRole).toInt();

    // Вторая строка — исходя из статуса/типинга, меняем цвет
    QString bottomText = lastMessage;
    QColor bottomTextColor = Qt::gray;
    if (isTyping) {
        bottomText = "печатает...";
        bottomTextColor = QColor("#F4ABC4");  // розовая подсветка
    }

    int padding = 12;
    QRect contentRect = option.rect.adjusted(padding, padding / 2, -padding, -padding / 2);
    QRect topRect(contentRect.left(), contentRect.top(), contentRect.width(), contentRect.height() / 2);
    QRect bottomRect(contentRect.left(), contentRect.top() + contentRect.height() / 2, contentRect.width(), contentRect.height() / 2);

    // DisplayName: online — bold белым, offline — обычным светлым
    QFont nameFont = painter->font();
    nameFont.setBold(isOnline);
    painter->setFont(nameFont);
    painter->setPen(isOnline ? Qt::white : QColor("#EAEAEA"));
    painter->drawText(topRect, Qt::AlignLeft | Qt::AlignBottom, displayName);

    // Bottom text с elide (короткое сообщение или "печатает...")
    QFontMetrics fm(painter->font());
    QString elidedBottomText = fm.elidedText(bottomText, Qt::ElideRight, bottomRect.width());
    painter->setFont(QApplication::font());  // обычный размер
    painter->setPen(bottomTextColor);
    painter->drawText(bottomRect, Qt::AlignLeft | Qt::AlignTop, elidedBottomText);

    // Badge непрочитанных сообщений — рисуется справа, если есть ненулевые
    if (unreadCount > 0) {
        painter->setRenderHint(QPainter::Antialiasing);
        int badgeSize = 20;
        QColor badgeColor = option.palette.highlight().color();
        QColor textColor = option.palette.highlightedText().color();
        QRect badgeRect(option.rect.right() - badgeSize - padding, option.rect.top() + (option.rect.height() - badgeSize) / 2, badgeSize, badgeSize);

        painter->setBrush(badgeColor);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(badgeRect);

        painter->setPen(textColor);
        painter->setFont(QFont("Segoe UI", 8, QFont::Bold));
        painter->drawText(badgeRect, Qt::AlignCenter, QString::number(unreadCount));
    }
    painter->restore();
    qDebug() << "[ContactListDelegate] Нарисован контакт:" << displayName << "online:" << isOnline << "unread:" << unreadCount;
}


QSize ContactListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QFontMetrics fm(QApplication::font());
    int singleLineHeight = fm.height();
    return QSize(200, singleLineHeight * 2 + 16); // две строки + небольшой padding
}
