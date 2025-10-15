 

#ifndef CHATVIEWWIDGET_H
#define CHATVIEWWIDGET_H

#include <QWidget>
#include "structures.h"



class QStackedWidget;
class QLabel;
class QToolButton;
class QLineEdit;
class QListView;
class QTextEdit;
class QPropertyAnimation;



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
    QTextEdit* messageTextEdit() const;
    bool isScrolledToBottom() const;

public slots:
    void updateHeader(const User& chatPartner);
    void setEditMode(bool enabled, const QString& text = QString());
    void clearReplyUI();
    void showReplyUI(const QString& name, const QString& text);
    void hideReplyUI();
    void onNewMessageReceived();
    void scrollToBottom();


signals:
    void sendMessageRequested(const QString& text);
    void headerClicked();
    void searchButtonClicked();
    void replyToMessageRequested(qint64 messageId);
    void editMessageRequested(qint64 messageId, const QString& oldText);
    void deleteMessageRequested(qint64 messageId);
    void replyCancelled();

private slots:
    void onSearchTriggered(const QString& text);
    void onChatContextMenuRequested(const QPoint &pos);
    void onMessageDoubleClicked(const QModelIndex &index);
     
    void onChatScrolled(int value);
protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;



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

    QToolButton* m_scrollToBottomButton;
    QLabel* m_unreadCountLabel;
    int m_unreadMessageCount;
    QPropertyAnimation* m_replyAnimation;
    void updateScrollToBottomButton();  
};

#endif  
