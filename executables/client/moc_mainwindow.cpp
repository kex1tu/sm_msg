/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../mainwindow.h"
#include <QtCore/qmetatype.h>

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
        "onConnected",
        "onDisconnected",
        "onReadyRead",
        "onLoginRequested",
        "username",
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
        "messageId",
        "oldText",
        "onDeleteMessageRequested",
        "onChatSearchTriggered",
        "showProfileView",
        "onReplyToMessage",
        "onSendMessageReadReceipt",
        "onGlobalSearchTriggered",
        "onChatScroll",
        "value",
        "onTypingNotificationFired",
        "handleUnreadCounts",
        "QJsonObject",
        "response",
        "updateMessageStatusInCacheAndModel",
        "ChatMessage::MessageStatus",
        "newStatus",
        "processVisibleMessages"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'newMessageForCurrentChat'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onConnected'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDisconnected'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onReadyRead'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onLoginRequested'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(6, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 7 }, { QMetaType::QString, 8 },
        }}),
        // Slot 'onRegisterRequested'
        QtMocHelpers::SlotData<void(const QString &, const QString &, const QString &)>(9, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 7 }, { QMetaType::QString, 10 }, { QMetaType::QString, 8 },
        }}),
        // Slot 'onSendMessageRequested'
        QtMocHelpers::SlotData<void(const QString &)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 12 },
        }}),
        // Slot 'onUserSelectionChanged'
        QtMocHelpers::SlotData<void(const QModelIndex &)>(13, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 14, 15 },
        }}),
        // Slot 'onLogoutButtonClicked'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAddContactRequested'
        QtMocHelpers::SlotData<void(const QString &)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 7 },
        }}),
        // Slot 'onEditMessageRequested'
        QtMocHelpers::SlotData<void(qint64, const QString &)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 19 }, { QMetaType::QString, 20 },
        }}),
        // Slot 'onDeleteMessageRequested'
        QtMocHelpers::SlotData<void(qint64)>(21, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 19 },
        }}),
        // Slot 'onChatSearchTriggered'
        QtMocHelpers::SlotData<void(const QString &)>(22, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 12 },
        }}),
        // Slot 'showProfileView'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onReplyToMessage'
        QtMocHelpers::SlotData<void(qint64)>(24, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 19 },
        }}),
        // Slot 'onSendMessageReadReceipt'
        QtMocHelpers::SlotData<void(qint64)>(25, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 19 },
        }}),
        // Slot 'onGlobalSearchTriggered'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onChatScroll'
        QtMocHelpers::SlotData<void(int)>(27, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 28 },
        }}),
        // Slot 'onTypingNotificationFired'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'handleUnreadCounts'
        QtMocHelpers::SlotData<void(const QJsonObject &)>(30, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 31, 32 },
        }}),
        // Slot 'updateMessageStatusInCacheAndModel'
        QtMocHelpers::SlotData<void(qint64, ChatMessage::MessageStatus)>(33, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 19 }, { 0x80000000 | 34, 35 },
        }}),
        // Slot 'processVisibleMessages'
        QtMocHelpers::SlotData<void()>(36, 2, QMC::AccessPrivate, QMetaType::Void),
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
        case 1: _t->onConnected(); break;
        case 2: _t->onDisconnected(); break;
        case 3: _t->onReadyRead(); break;
        case 4: _t->onLoginRequested((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 5: _t->onRegisterRequested((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 6: _t->onSendMessageRequested((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->onUserSelectionChanged((*reinterpret_cast<std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 8: _t->onLogoutButtonClicked(); break;
        case 9: _t->onAddContactRequested((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: _t->onEditMessageRequested((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 11: _t->onDeleteMessageRequested((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 12: _t->onChatSearchTriggered((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 13: _t->showProfileView(); break;
        case 14: _t->onReplyToMessage((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 15: _t->onSendMessageReadReceipt((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 16: _t->onGlobalSearchTriggered(); break;
        case 17: _t->onChatScroll((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 18: _t->onTypingNotificationFired(); break;
        case 19: _t->handleUnreadCounts((*reinterpret_cast<std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 20: _t->updateMessageStatusInCacheAndModel((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<ChatMessage::MessageStatus>>(_a[2]))); break;
        case 21: _t->processVisibleMessages(); break;
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
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 22)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 22;
    }
    return _id;
}

// SIGNAL 0
void MainWindow::newMessageForCurrentChat()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
