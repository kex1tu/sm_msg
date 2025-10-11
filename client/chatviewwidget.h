// chatviewwidget.h

#ifndef CHATVIEWWIDGET_H
#define CHATVIEWWIDGET_H

#include <QWidget>
#include "structures.h"


class QStackedWidget;
class QLabel;
class QToolButton;
class QLineEdit;
class QListView;

namespace Ui {
class ChatViewWidget;
}

class ChatViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatViewWidget(QWidget *parent = nullptr);
    ~ChatViewWidget();


    QListView* chatHistoryView() const;
    QLineEdit* messageLineEdit() const;

public slots:
    void updateHeader(const User& chatPartner, bool isTyping);
    void setEditMode(bool enabled, const QString& text = QString());

signals:
    void sendMessageRequested(const QString& text);
    void headerClicked();
    void searchButtonClicked(); // Сигнал для MainWindow
    void replyToMessageRequested(qint64 messageId);
    void editMessageRequested(qint64 messageId, const QString& oldText);
    void deleteMessageRequested(qint64 messageId);

private slots:
    void onSearchTriggered(const QString& text);
    void onChatContextMenuRequested(const QPoint &pos);
    void onMessageDoubleClicked(const QModelIndex &index);


private:
    QString formatLastSeen(const User &user);
    void setupHeaderUI();
    Ui::ChatViewWidget *ui;

    QLabel* m_nameLabel;
    QLabel* m_statusLabel;
    QToolButton* m_searchButton;
    QToolButton* m_callButton;
    QToolButton* m_videoCallButton;
    QToolButton* m_moreOptionsButton;

    QLineEdit* m_searchLineEdit;
    QToolButton* m_closeSearchButton;
};

#endif // CHATVIEWWIDGET_H
