
#include <QAbstractListModel>
#include "structures.h"


Q_DECLARE_METATYPE(ChatMessage)

class ChatMessageModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ChatMessageModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;


public slots:

    void addMessage(const ChatMessage &message);
    void addMessages(const QList<ChatMessage> &messages);
    void prependMessages(const QList<ChatMessage> &messages);
    void clearMessages();
    void removeMessage(qint64 messageId);
    void confirmMessage(const QString& tempId, const ChatMessage& confirmedMessage);
    void updateMessageStatus(qint64 messageId, ChatMessage::MessageStatus newStatus);
    void editMessage(qint64 messageId, const QString& newPayload);
    bool getMessageById(qint64 id, ChatMessage &msg) const;

private:
    QList<ChatMessage> m_messages;
    QMap<qint64, ChatMessage> m_messageMap;
};
