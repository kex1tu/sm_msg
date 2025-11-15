/****************************************************************************
** Meta object code from reading C++ file 'dataservice.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../core/dataservice.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dataservice.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN11DataServiceE_t {};
} // unnamed namespace

template <> constexpr inline auto DataService::qt_create_metaobjectdata<qt_meta_tag_ZN11DataServiceE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "DataService",
        "contactsUpdated",
        "",
        "sortedUsernames",
        "onlineStatusUpdated",
        "olderHistoryChunkPrepended",
        "chatPartner",
        "QList<ChatMessage>",
        "messages",
        "historyLoaded",
        "loginSuccess",
        "QJsonObject",
        "response",
        "loginFailure",
        "reason",
        "registerSuccess",
        "registerFailure",
        "newMessageReceived",
        "ChatMessage",
        "message",
        "messageStatusChanged",
        "messageId",
        "ChatMessage::MessageStatus",
        "newStatus",
        "unreadCountChanged",
        "messageEdited",
        "newPayload",
        "messageDeleted",
        "searchResultsReceived",
        "QJsonArray",
        "users",
        "addContactSuccess",
        "username",
        "addContactFailure",
        "contactRequestReceived",
        "request",
        "pendingContactRequestsUpdated",
        "requests",
        "logoutSuccess",
        "logoutFailure",
        "typingStatusChanged",
        "isTyping",
        "confirmMessageSent",
        "tempId",
        "msg",
        "callRequestSent",
        "toUser",
        "callId",
        "incomingCall",
        "fromUser",
        "callerIp",
        "callerPort",
        "callAccepted",
        "calleeIp",
        "calleePort",
        "callRejected",
        "callEnded",
        "callHistoryReceived",
        "calls",
        "callStatsReceived",
        "stats",
        "profileUpdateResult",
        "requestServerHistory",
        "afterId",
        "processResponse",
        "handleLoginSuccess",
        "handleLoginFailure",
        "handleRegisterSuccess",
        "handleRegisterFailure",
        "handleContactList",
        "handleHistoryData",
        "handleOldHistoryData",
        "handlePrivateMessage",
        "handleUserList",
        "handleMessageDelivered",
        "handleMessageRead",
        "handleEditMessage",
        "handleDeleteMessage",
        "handleSearchResults",
        "handleAddContactSuccess",
        "handleAddContactFailure",
        "handleIncomingContactRequest",
        "handlePendingRequestsList",
        "handleLogoutSuccess",
        "handleLogoutFailure",
        "handleTypingResponse",
        "handleUnreadCounts",
        "handleCallRequestSent",
        "handleIncomingCall",
        "handleCallAccepted",
        "handleCallRejected",
        "handleCallEnd",
        "handleCallHistory",
        "handleCallStats",
        "handleUpdateProfileResult",
        "requestCallHistory",
        "syncChatHistory",
        "getChatCacheRef",
        "clearAllData"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'contactsUpdated'
        QtMocHelpers::SignalData<void(const QStringList &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QStringList, 3 },
        }}),
        // Signal 'onlineStatusUpdated'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'olderHistoryChunkPrepended'
        QtMocHelpers::SignalData<void(const QString &, const QList<ChatMessage> &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 }, { 0x80000000 | 7, 8 },
        }}),
        // Signal 'historyLoaded'
        QtMocHelpers::SignalData<void(const QString &, const QList<ChatMessage> &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 }, { 0x80000000 | 7, 8 },
        }}),
        // Signal 'loginSuccess'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Signal 'loginFailure'
        QtMocHelpers::SignalData<void(const QString &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 14 },
        }}),
        // Signal 'registerSuccess'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'registerFailure'
        QtMocHelpers::SignalData<void(const QString &)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 14 },
        }}),
        // Signal 'newMessageReceived'
        QtMocHelpers::SignalData<void(const ChatMessage &)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 18, 19 },
        }}),
        // Signal 'messageStatusChanged'
        QtMocHelpers::SignalData<void(qint64, ChatMessage::MessageStatus)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 21 }, { 0x80000000 | 22, 23 },
        }}),
        // Signal 'unreadCountChanged'
        QtMocHelpers::SignalData<void()>(24, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'messageEdited'
        QtMocHelpers::SignalData<void(const QString &, qint64, const QString &)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 }, { QMetaType::LongLong, 21 }, { QMetaType::QString, 26 },
        }}),
        // Signal 'messageDeleted'
        QtMocHelpers::SignalData<void(const QString &, qint64)>(27, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 }, { QMetaType::LongLong, 21 },
        }}),
        // Signal 'searchResultsReceived'
        QtMocHelpers::SignalData<void(const QJsonArray &)>(28, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 29, 30 },
        }}),
        // Signal 'addContactSuccess'
        QtMocHelpers::SignalData<void(const QString &)>(31, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 32 },
        }}),
        // Signal 'addContactFailure'
        QtMocHelpers::SignalData<void(const QString &)>(33, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 14 },
        }}),
        // Signal 'contactRequestReceived'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(34, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 35 },
        }}),
        // Signal 'pendingContactRequestsUpdated'
        QtMocHelpers::SignalData<void(const QJsonArray &)>(36, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 29, 37 },
        }}),
        // Signal 'logoutSuccess'
        QtMocHelpers::SignalData<void()>(38, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'logoutFailure'
        QtMocHelpers::SignalData<void(const QString &)>(39, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 14 },
        }}),
        // Signal 'typingStatusChanged'
        QtMocHelpers::SignalData<void(const QString &, bool)>(40, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 32 }, { QMetaType::Bool, 41 },
        }}),
        // Signal 'confirmMessageSent'
        QtMocHelpers::SignalData<void(QString, const ChatMessage &)>(42, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 43 }, { 0x80000000 | 18, 44 },
        }}),
        // Signal 'callRequestSent'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(45, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 46 }, { QMetaType::QString, 47 },
        }}),
        // Signal 'incomingCall'
        QtMocHelpers::SignalData<void(const QString &, const QString &, const QString &, quint16)>(48, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 49 }, { QMetaType::QString, 47 }, { QMetaType::QString, 50 }, { QMetaType::UShort, 51 },
        }}),
        // Signal 'callAccepted'
        QtMocHelpers::SignalData<void(const QString &, const QString &, quint16)>(52, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 49 }, { QMetaType::QString, 53 }, { QMetaType::UShort, 54 },
        }}),
        // Signal 'callRejected'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(55, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 49 }, { QMetaType::QString, 14 },
        }}),
        // Signal 'callEnded'
        QtMocHelpers::SignalData<void()>(56, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'callHistoryReceived'
        QtMocHelpers::SignalData<void(const QJsonArray &)>(57, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 29, 58 },
        }}),
        // Signal 'callStatsReceived'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(59, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 60 },
        }}),
        // Signal 'profileUpdateResult'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(61, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Signal 'requestServerHistory'
        QtMocHelpers::SignalData<void(const QString &, int)>(62, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 }, { QMetaType::Int, 63 },
        }}),
        // Slot 'processResponse'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(64, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleLoginSuccess'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(65, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleLoginFailure'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(66, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleRegisterSuccess'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(67, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleRegisterFailure'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(68, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleContactList'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(69, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleHistoryData'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(70, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleOldHistoryData'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(71, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handlePrivateMessage'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(72, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleUserList'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(73, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleMessageDelivered'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(74, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleMessageRead'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(75, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleEditMessage'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(76, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleDeleteMessage'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(77, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleSearchResults'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(78, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleAddContactSuccess'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(79, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleAddContactFailure'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(80, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleIncomingContactRequest'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(81, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handlePendingRequestsList'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(82, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleLogoutSuccess'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(83, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleLogoutFailure'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(84, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleTypingResponse'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(85, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleUnreadCounts'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(86, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleCallRequestSent'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(87, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleIncomingCall'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(88, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleCallAccepted'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(89, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleCallRejected'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(90, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleCallEnd'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(91, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleCallHistory'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(92, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleCallStats'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(93, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'handleUpdateProfileResult'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(94, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'requestCallHistory'
        QtMocHelpers::SlotData<void()>(95, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'syncChatHistory'
        QtMocHelpers::SlotData<void(const QString &)>(96, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 6 },
        }}),
        // Slot 'getChatCacheRef'
        QtMocHelpers::SlotData<void(const QString &)>(97, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 32 },
        }}),
        // Slot 'clearAllData'
        QtMocHelpers::SlotData<void()>(98, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DataService, qt_meta_tag_ZN11DataServiceE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject DataService::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11DataServiceE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11DataServiceE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11DataServiceE_t>.metaTypes,
    nullptr
} };

void DataService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DataService *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->contactsUpdated((*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[1]))); break;
        case 1: _t->onlineStatusUpdated(); break;
        case 2: _t->olderHistoryChunkPrepended((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QList<ChatMessage>>>(_a[2]))); break;
        case 3: _t->historyLoaded((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QList<ChatMessage>>>(_a[2]))); break;
        case 4: _t->loginSuccess((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 5: _t->loginFailure((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->registerSuccess(); break;
        case 7: _t->registerFailure((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->newMessageReceived((*reinterpret_cast<std::add_pointer_t<ChatMessage>>(_a[1]))); break;
        case 9: _t->messageStatusChanged((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<ChatMessage::MessageStatus>>(_a[2]))); break;
        case 10: _t->unreadCountChanged(); break;
        case 11: _t->messageEdited((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<qint64>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 12: _t->messageDeleted((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<qint64>>(_a[2]))); break;
        case 13: _t->searchResultsReceived((*reinterpret_cast<std::add_pointer_t<QJsonArray>>(_a[1]))); break;
        case 14: _t->addContactSuccess((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 15: _t->addContactFailure((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: _t->contactRequestReceived((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 17: _t->pendingContactRequestsUpdated((*reinterpret_cast<std::add_pointer_t<QJsonArray>>(_a[1]))); break;
        case 18: _t->logoutSuccess(); break;
        case 19: _t->logoutFailure((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 20: _t->typingStatusChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2]))); break;
        case 21: _t->confirmMessageSent((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<ChatMessage>>(_a[2]))); break;
        case 22: _t->callRequestSent((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 23: _t->incomingCall((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<quint16>>(_a[4]))); break;
        case 24: _t->callAccepted((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<quint16>>(_a[3]))); break;
        case 25: _t->callRejected((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 26: _t->callEnded(); break;
        case 27: _t->callHistoryReceived((*reinterpret_cast<std::add_pointer_t<QJsonArray>>(_a[1]))); break;
        case 28: _t->callStatsReceived((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 29: _t->profileUpdateResult((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 30: _t->requestServerHistory((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 31: _t->processResponse((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 32: _t->handleLoginSuccess((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 33: _t->handleLoginFailure((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 34: _t->handleRegisterSuccess((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 35: _t->handleRegisterFailure((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 36: _t->handleContactList((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 37: _t->handleHistoryData((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 38: _t->handleOldHistoryData((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 39: _t->handlePrivateMessage((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 40: _t->handleUserList((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 41: _t->handleMessageDelivered((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 42: _t->handleMessageRead((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 43: _t->handleEditMessage((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 44: _t->handleDeleteMessage((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 45: _t->handleSearchResults((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 46: _t->handleAddContactSuccess((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 47: _t->handleAddContactFailure((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 48: _t->handleIncomingContactRequest((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 49: _t->handlePendingRequestsList((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 50: _t->handleLogoutSuccess((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 51: _t->handleLogoutFailure((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 52: _t->handleTypingResponse((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 53: _t->handleUnreadCounts((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 54: _t->handleCallRequestSent((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 55: _t->handleIncomingCall((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 56: _t->handleCallAccepted((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 57: _t->handleCallRejected((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 58: _t->handleCallEnd((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 59: _t->handleCallHistory((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 60: _t->handleCallStats((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 61: _t->handleUpdateProfileResult((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 62: _t->requestCallHistory(); break;
        case 63: _t->syncChatHistory((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 64: _t->getChatCacheRef((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 65: _t->clearAllData(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QStringList & )>(_a, &DataService::contactsUpdated, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)()>(_a, &DataService::onlineStatusUpdated, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QString & , const QList<ChatMessage> & )>(_a, &DataService::olderHistoryChunkPrepended, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QString & , const QList<ChatMessage> & )>(_a, &DataService::historyLoaded, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QJsonObject & )>(_a, &DataService::loginSuccess, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QString & )>(_a, &DataService::loginFailure, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)()>(_a, &DataService::registerSuccess, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QString & )>(_a, &DataService::registerFailure, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const ChatMessage & )>(_a, &DataService::newMessageReceived, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(qint64 , ChatMessage::MessageStatus )>(_a, &DataService::messageStatusChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)()>(_a, &DataService::unreadCountChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QString & , qint64 , const QString & )>(_a, &DataService::messageEdited, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QString & , qint64 )>(_a, &DataService::messageDeleted, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QJsonArray & )>(_a, &DataService::searchResultsReceived, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QString & )>(_a, &DataService::addContactSuccess, 14))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QString & )>(_a, &DataService::addContactFailure, 15))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QJsonObject & )>(_a, &DataService::contactRequestReceived, 16))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QJsonArray & )>(_a, &DataService::pendingContactRequestsUpdated, 17))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)()>(_a, &DataService::logoutSuccess, 18))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QString & )>(_a, &DataService::logoutFailure, 19))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QString & , bool )>(_a, &DataService::typingStatusChanged, 20))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(QString , const ChatMessage & )>(_a, &DataService::confirmMessageSent, 21))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QString & , const QString & )>(_a, &DataService::callRequestSent, 22))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QString & , const QString & , const QString & , quint16 )>(_a, &DataService::incomingCall, 23))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QString & , const QString & , quint16 )>(_a, &DataService::callAccepted, 24))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QString & , const QString & )>(_a, &DataService::callRejected, 25))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)()>(_a, &DataService::callEnded, 26))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QJsonArray & )>(_a, &DataService::callHistoryReceived, 27))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QJsonObject & )>(_a, &DataService::callStatsReceived, 28))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QJsonObject & )>(_a, &DataService::profileUpdateResult, 29))
            return;
        if (QtMocHelpers::indexOfMethod<void (DataService::*)(const QString & , int )>(_a, &DataService::requestServerHistory, 30))
            return;
    }
}

const QMetaObject *DataService::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DataService::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11DataServiceE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DataService::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 66)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 66;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 66)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 66;
    }
    return _id;
}

// SIGNAL 0
void DataService::contactsUpdated(const QStringList & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void DataService::onlineStatusUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void DataService::olderHistoryChunkPrepended(const QString & _t1, const QList<ChatMessage> & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void DataService::historyLoaded(const QString & _t1, const QList<ChatMessage> & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void DataService::loginSuccess(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void DataService::loginFailure(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void DataService::registerSuccess()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void DataService::registerFailure(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}

// SIGNAL 8
void DataService::newMessageReceived(const ChatMessage & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1);
}

// SIGNAL 9
void DataService::messageStatusChanged(qint64 _t1, ChatMessage::MessageStatus _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1, _t2);
}

// SIGNAL 10
void DataService::unreadCountChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void DataService::messageEdited(const QString & _t1, qint64 _t2, const QString & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 11, nullptr, _t1, _t2, _t3);
}

// SIGNAL 12
void DataService::messageDeleted(const QString & _t1, qint64 _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 12, nullptr, _t1, _t2);
}

// SIGNAL 13
void DataService::searchResultsReceived(const QJsonArray & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 13, nullptr, _t1);
}

// SIGNAL 14
void DataService::addContactSuccess(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 14, nullptr, _t1);
}

// SIGNAL 15
void DataService::addContactFailure(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 15, nullptr, _t1);
}

// SIGNAL 16
void DataService::contactRequestReceived(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 16, nullptr, _t1);
}

// SIGNAL 17
void DataService::pendingContactRequestsUpdated(const QJsonArray & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 17, nullptr, _t1);
}

// SIGNAL 18
void DataService::logoutSuccess()
{
    QMetaObject::activate(this, &staticMetaObject, 18, nullptr);
}

// SIGNAL 19
void DataService::logoutFailure(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 19, nullptr, _t1);
}

// SIGNAL 20
void DataService::typingStatusChanged(const QString & _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 20, nullptr, _t1, _t2);
}

// SIGNAL 21
void DataService::confirmMessageSent(QString _t1, const ChatMessage & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 21, nullptr, _t1, _t2);
}

// SIGNAL 22
void DataService::callRequestSent(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 22, nullptr, _t1, _t2);
}

// SIGNAL 23
void DataService::incomingCall(const QString & _t1, const QString & _t2, const QString & _t3, quint16 _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 23, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 24
void DataService::callAccepted(const QString & _t1, const QString & _t2, quint16 _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 24, nullptr, _t1, _t2, _t3);
}

// SIGNAL 25
void DataService::callRejected(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 25, nullptr, _t1, _t2);
}

// SIGNAL 26
void DataService::callEnded()
{
    QMetaObject::activate(this, &staticMetaObject, 26, nullptr);
}

// SIGNAL 27
void DataService::callHistoryReceived(const QJsonArray & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 27, nullptr, _t1);
}

// SIGNAL 28
void DataService::callStatsReceived(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 28, nullptr, _t1);
}

// SIGNAL 29
void DataService::profileUpdateResult(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 29, nullptr, _t1);
}

// SIGNAL 30
void DataService::requestServerHistory(const QString & _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 30, nullptr, _t1, _t2);
}
QT_WARNING_POP
