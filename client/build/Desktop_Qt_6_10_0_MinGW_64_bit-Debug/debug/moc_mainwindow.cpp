/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../mainwindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "newMessageForCurrentChat",
        "",
        "toggleFullScreen",
        "onJsonReceived",
        "QJsonObject",
        "response",
        "onContactsUpdated",
        "sortedUsernames",
        "onOnlineStatusUpdated",
        "onOlderHistoryChunkPrepended",
        "chatPartner",
        "QList<ChatMessage>",
        "messages",
        "onHistoryLoaded",
        "onLoginSuccess",
        "onLoginFailure",
        "reason",
        "onRegisterSuccess",
        "onRegisterFailure",
        "onLogoutSuccess",
        "onLogoutFailure",
        "onNewMessageReceived",
        "ChatMessage",
        "incomingMsg",
        "onMessageStatusChanged",
        "messageId",
        "ChatMessage::MessageStatus",
        "newStatus",
        "onUnreadCountChanged",
        "onMessageEdited",
        "newPayload",
        "onMessageDeleted",
        "onConfirmMessageSent",
        "tempId",
        "msg",
        "onSearchResultsReceived",
        "QJsonArray",
        "users",
        "onAddContactSuccess",
        "username",
        "onAddContactFailure",
        "onPendingContactRequestsUpdated",
        "requests",
        "onLoginRequested",
        "password",
        "onRegisterRequested",
        "displayName",
        "onSendMessageRequested",
        "text",
        "onUserSelectionChanged",
        "QModelIndex",
        "current",
        "onLogoutButtonClicked",
        "onAddContactRequested",
        "onEditMessageRequested",
        "oldText",
        "onDeleteMessageRequested",
        "onChatSearchTriggered",
        "onReplyToMessage",
        "onSendMessageReadReceipt",
        "onGlobalSearchTriggered",
        "onChatScroll",
        "value",
        "onTypingNotificationFired",
        "onTypingStatusChanged",
        "isTyping",
        "onConnected",
        "onDisconnected",
        "onScrollToUnread",
        "onScrollToBottom",
        "onScrollBarRangeChanged",
        "onRequestAccepted",
        "request",
        "onRequestRejected",
        "onCallRequested",
        "onMenuButtonClicked",
        "onCallsButtonClicked",
        "onBackFromMenu",
        "processVisibleMessages",
        "showProfileView",
        "hideProfileView",
        "onMyProfileClicked",
        "onProfileUpdateResult",
        "onRequestServerHistory",
        "afterId"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'newMessageForCurrentChat'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'toggleFullScreen'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onJsonReceived'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(4, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 5, 6 },
        }}),
        // Slot 'onContactsUpdated'
        QtMocHelpers::SlotData<void(const QStringList &)>(7, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QStringList, 8 },
        }}),
        // Slot 'onOnlineStatusUpdated'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onOlderHistoryChunkPrepended'
        QtMocHelpers::SlotData<void(const QString &, const QList<ChatMessage> &)>(10, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 11 }, { 0x80000000 | 12, 13 },
        }}),
        // Slot 'onHistoryLoaded'
        QtMocHelpers::SlotData<void(const QString &, const QList<ChatMessage> &)>(14, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 11 }, { 0x80000000 | 12, 13 },
        }}),
        // Slot 'onLoginSuccess'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 5, 6 },
        }}),
        // Slot 'onLoginFailure'
        QtMocHelpers::SlotData<void(const QString &)>(16, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 17 },
        }}),
        // Slot 'onRegisterSuccess'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRegisterFailure'
        QtMocHelpers::SlotData<void(const QString &)>(19, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 17 },
        }}),
        // Slot 'onLogoutSuccess'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onLogoutFailure'
        QtMocHelpers::SlotData<void(const QString &)>(21, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 17 },
        }}),
        // Slot 'onNewMessageReceived'
        QtMocHelpers::SlotData<void(const ChatMessage &)>(22, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 23, 24 },
        }}),
        // Slot 'onMessageStatusChanged'
        QtMocHelpers::SlotData<void(qint64, ChatMessage::MessageStatus)>(25, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 26 }, { 0x80000000 | 27, 28 },
        }}),
        // Slot 'onUnreadCountChanged'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onMessageEdited'
        QtMocHelpers::SlotData<void(const QString &, qint64, const QString &)>(30, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 11 }, { QMetaType::LongLong, 26 }, { QMetaType::QString, 31 },
        }}),
        // Slot 'onMessageDeleted'
        QtMocHelpers::SlotData<void(const QString &, qint64)>(32, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 11 }, { QMetaType::LongLong, 26 },
        }}),
        // Slot 'onConfirmMessageSent'
        QtMocHelpers::SlotData<void(QString, const ChatMessage &)>(33, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 34 }, { 0x80000000 | 23, 35 },
        }}),
        // Slot 'onSearchResultsReceived'
        QtMocHelpers::SlotData<void(const QJsonArray &)>(36, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 37, 38 },
        }}),
        // Slot 'onAddContactSuccess'
        QtMocHelpers::SlotData<void(const QString &)>(39, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 40 },
        }}),
        // Slot 'onAddContactFailure'
        QtMocHelpers::SlotData<void(const QString &)>(41, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 17 },
        }}),
        // Slot 'onPendingContactRequestsUpdated'
        QtMocHelpers::SlotData<void(const QJsonArray &)>(42, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 37, 43 },
        }}),
        // Slot 'onLoginRequested'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(44, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 40 }, { QMetaType::QString, 45 },
        }}),
        // Slot 'onRegisterRequested'
        QtMocHelpers::SlotData<void(const QString &, const QString &, const QString &)>(46, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 40 }, { QMetaType::QString, 47 }, { QMetaType::QString, 45 },
        }}),
        // Slot 'onSendMessageRequested'
        QtMocHelpers::SlotData<void(const QString &)>(48, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 49 },
        }}),
        // Slot 'onUserSelectionChanged'
        QtMocHelpers::SlotData<void(const QModelIndex &)>(50, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 51, 52 },
        }}),
        // Slot 'onLogoutButtonClicked'
        QtMocHelpers::SlotData<void()>(53, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAddContactRequested'
        QtMocHelpers::SlotData<void(const QString &)>(54, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 40 },
        }}),
        // Slot 'onEditMessageRequested'
        QtMocHelpers::SlotData<void(qint64, const QString &)>(55, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 26 }, { QMetaType::QString, 56 },
        }}),
        // Slot 'onDeleteMessageRequested'
        QtMocHelpers::SlotData<void(qint64)>(57, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 26 },
        }}),
        // Slot 'onChatSearchTriggered'
        QtMocHelpers::SlotData<void(const QString &)>(58, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 49 },
        }}),
        // Slot 'onReplyToMessage'
        QtMocHelpers::SlotData<void(qint64)>(59, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 26 },
        }}),
        // Slot 'onSendMessageReadReceipt'
        QtMocHelpers::SlotData<void(qint64)>(60, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 26 },
        }}),
        // Slot 'onGlobalSearchTriggered'
        QtMocHelpers::SlotData<void()>(61, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onChatScroll'
        QtMocHelpers::SlotData<void(int)>(62, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 63 },
        }}),
        // Slot 'onTypingNotificationFired'
        QtMocHelpers::SlotData<void()>(64, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onTypingStatusChanged'
        QtMocHelpers::SlotData<void(const QString &, bool)>(65, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 40 }, { QMetaType::Bool, 66 },
        }}),
        // Slot 'onConnected'
        QtMocHelpers::SlotData<void()>(67, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDisconnected'
        QtMocHelpers::SlotData<void()>(68, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onScrollToUnread'
        QtMocHelpers::SlotData<void()>(69, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onScrollToBottom'
        QtMocHelpers::SlotData<void()>(70, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onScrollBarRangeChanged'
        QtMocHelpers::SlotData<void()>(71, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRequestAccepted'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(72, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 5, 73 },
        }}),
        // Slot 'onRequestRejected'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(74, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 5, 73 },
        }}),
        // Slot 'onCallRequested'
        QtMocHelpers::SlotData<void()>(75, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onMenuButtonClicked'
        QtMocHelpers::SlotData<void()>(76, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onCallsButtonClicked'
        QtMocHelpers::SlotData<void()>(77, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onBackFromMenu'
        QtMocHelpers::SlotData<void()>(78, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'processVisibleMessages'
        QtMocHelpers::SlotData<void()>(79, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showProfileView'
        QtMocHelpers::SlotData<void()>(80, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'hideProfileView'
        QtMocHelpers::SlotData<void()>(81, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onMyProfileClicked'
        QtMocHelpers::SlotData<void()>(82, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onProfileUpdateResult'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(83, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 5, 6 },
        }}),
        // Slot 'onRequestServerHistory'
        QtMocHelpers::SlotData<void(const QString &, int)>(84, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 11 }, { QMetaType::Int, 85 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->newMessageForCurrentChat(); break;
        case 1: _t->toggleFullScreen(); break;
        case 2: _t->onJsonReceived((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 3: _t->onContactsUpdated((*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[1]))); break;
        case 4: _t->onOnlineStatusUpdated(); break;
        case 5: _t->onOlderHistoryChunkPrepended((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QList<ChatMessage>>>(_a[2]))); break;
        case 6: _t->onHistoryLoaded((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QList<ChatMessage>>>(_a[2]))); break;
        case 7: _t->onLoginSuccess((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 8: _t->onLoginFailure((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->onRegisterSuccess(); break;
        case 10: _t->onRegisterFailure((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->onLogoutSuccess(); break;
        case 12: _t->onLogoutFailure((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 13: _t->onNewMessageReceived((*reinterpret_cast<std::add_pointer_t<ChatMessage>>(_a[1]))); break;
        case 14: _t->onMessageStatusChanged((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<ChatMessage::MessageStatus>>(_a[2]))); break;
        case 15: _t->onUnreadCountChanged(); break;
        case 16: _t->onMessageEdited((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<qint64>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 17: _t->onMessageDeleted((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<qint64>>(_a[2]))); break;
        case 18: _t->onConfirmMessageSent((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<ChatMessage>>(_a[2]))); break;
        case 19: _t->onSearchResultsReceived((*reinterpret_cast<std::add_pointer_t<QJsonArray>>(_a[1]))); break;
        case 20: _t->onAddContactSuccess((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 21: _t->onAddContactFailure((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 22: _t->onPendingContactRequestsUpdated((*reinterpret_cast<std::add_pointer_t<QJsonArray>>(_a[1]))); break;
        case 23: _t->onLoginRequested((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 24: _t->onRegisterRequested((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 25: _t->onSendMessageRequested((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 26: _t->onUserSelectionChanged((*reinterpret_cast<std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 27: _t->onLogoutButtonClicked(); break;
        case 28: _t->onAddContactRequested((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 29: _t->onEditMessageRequested((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 30: _t->onDeleteMessageRequested((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 31: _t->onChatSearchTriggered((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 32: _t->onReplyToMessage((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 33: _t->onSendMessageReadReceipt((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 34: _t->onGlobalSearchTriggered(); break;
        case 35: _t->onChatScroll((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 36: _t->onTypingNotificationFired(); break;
        case 37: _t->onTypingStatusChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2]))); break;
        case 38: _t->onConnected(); break;
        case 39: _t->onDisconnected(); break;
        case 40: _t->onScrollToUnread(); break;
        case 41: _t->onScrollToBottom(); break;
        case 42: _t->onScrollBarRangeChanged(); break;
        case 43: _t->onRequestAccepted((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 44: _t->onRequestRejected((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 45: _t->onCallRequested(); break;
        case 46: _t->onMenuButtonClicked(); break;
        case 47: _t->onCallsButtonClicked(); break;
        case 48: _t->onBackFromMenu(); break;
        case 49: _t->processVisibleMessages(); break;
        case 50: _t->showProfileView(); break;
        case 51: _t->hideProfileView(); break;
        case 52: _t->onMyProfileClicked(); break;
        case 53: _t->onProfileUpdateResult((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 54: _t->onRequestServerHistory((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (MainWindow::*)()>(_a, &MainWindow::newMessageForCurrentChat, 0))
            return;
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 55)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 55;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 55)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 55;
    }
    return _id;
}

// SIGNAL 0
void MainWindow::newMessageForCurrentChat()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
