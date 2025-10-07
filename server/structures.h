#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QString>
struct User {
    qint64 id = 0;
    QString username;
    QString displayName;
    QString lastSeen;
    QString avatarUrl;
    QString statusMessage;

    bool isTyping = false;
    bool isOnline = false;
};

struct ChatMessage {

    enum MessageType {
        Text,
        Image,
        File,
        Sticker,
        System
    };
    enum MessageStatus{
        Sending, //0
        Sent, //1
        Delivered, //2
        Read, //3
        Error //4
    };

    quint64 id = 0;
    QString tempId;
    QString fromUser;
    QString toUser;
    QString payload;
    QString timestamp;

    bool isEdited;
    qint64 replyToId = 0;
    User forwardedFrom;
    MessageType messageType;
    MessageStatus status;
    QString mediaUrl;
    bool isOutgoing;
};



#endif // STRUCTURES_H
