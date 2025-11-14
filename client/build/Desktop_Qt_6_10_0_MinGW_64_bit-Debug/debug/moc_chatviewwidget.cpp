/****************************************************************************
** Meta object code from reading C++ file 'chatviewwidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../ui/chatviewwidget.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'chatviewwidget.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN14ChatViewWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto ChatViewWidget::qt_create_metaobjectdata<qt_meta_tag_ZN14ChatViewWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ChatViewWidget",
        "sendMessageRequested",
        "",
        "text",
        "headerClicked",
        "searchButtonClicked",
        "replyToMessageRequested",
        "messageId",
        "replyCancelled",
        "searchTextEntered",
        "scrollToUnreadRequested",
        "scrollToBottomRequested",
        "callRequested",
        "editMessageRequested",
        "oldText",
        "deleteMessageRequested",
        "onSearchTriggered",
        "onChatContextMenuRequested",
        "QPoint",
        "pos",
        "onMessageDoubleClicked",
        "QModelIndex",
        "index",
        "onChatScrolled",
        "value",
        "showSearchUI",
        "hideSearchUI",
        "updateHeader",
        "User",
        "chatPartner",
        "setEditMode",
        "enabled",
        "clearReplyUI",
        "showReplyUI",
        "name",
        "hideReplyUI",
        "onNewMessageReceived",
        "scrollToBottom",
        "scrollToMessage",
        "onScrollDownButtonClicked",
        "onCallButtonClicked"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'sendMessageRequested'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'headerClicked'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'searchButtonClicked'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'replyToMessageRequested'
        QtMocHelpers::SignalData<void(qint64)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 7 },
        }}),
        // Signal 'replyCancelled'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'searchTextEntered'
        QtMocHelpers::SignalData<void(const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'scrollToUnreadRequested'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'scrollToBottomRequested'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'callRequested'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'editMessageRequested'
        QtMocHelpers::SignalData<void(qint64, const QString &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 7 }, { QMetaType::QString, 14 },
        }}),
        // Signal 'deleteMessageRequested'
        QtMocHelpers::SignalData<void(qint64)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 7 },
        }}),
        // Slot 'onSearchTriggered'
        QtMocHelpers::SlotData<void(const QString &)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Slot 'onChatContextMenuRequested'
        QtMocHelpers::SlotData<void(const QPoint &)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 18, 19 },
        }}),
        // Slot 'onMessageDoubleClicked'
        QtMocHelpers::SlotData<void(const QModelIndex &)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 21, 22 },
        }}),
        // Slot 'onChatScrolled'
        QtMocHelpers::SlotData<void(int)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 24 },
        }}),
        // Slot 'showSearchUI'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'hideSearchUI'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'updateHeader'
        QtMocHelpers::SlotData<void(const User &)>(27, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 28, 29 },
        }}),
        // Slot 'setEditMode'
        QtMocHelpers::SlotData<void(bool, const QString &)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 31 }, { QMetaType::QString, 3 },
        }}),
        // Slot 'setEditMode'
        QtMocHelpers::SlotData<void(bool)>(30, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::Bool, 31 },
        }}),
        // Slot 'clearReplyUI'
        QtMocHelpers::SlotData<void()>(32, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'showReplyUI'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(33, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 34 }, { QMetaType::QString, 3 },
        }}),
        // Slot 'hideReplyUI'
        QtMocHelpers::SlotData<void()>(35, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onNewMessageReceived'
        QtMocHelpers::SlotData<void()>(36, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'scrollToBottom'
        QtMocHelpers::SlotData<void()>(37, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'scrollToMessage'
        QtMocHelpers::SlotData<void(const QModelIndex &)>(38, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 21, 22 },
        }}),
        // Slot 'onScrollDownButtonClicked'
        QtMocHelpers::SlotData<void()>(39, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onCallButtonClicked'
        QtMocHelpers::SlotData<void()>(40, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ChatViewWidget, qt_meta_tag_ZN14ChatViewWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ChatViewWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14ChatViewWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14ChatViewWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14ChatViewWidgetE_t>.metaTypes,
    nullptr
} };

void ChatViewWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ChatViewWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->sendMessageRequested((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->headerClicked(); break;
        case 2: _t->searchButtonClicked(); break;
        case 3: _t->replyToMessageRequested((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 4: _t->replyCancelled(); break;
        case 5: _t->searchTextEntered((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->scrollToUnreadRequested(); break;
        case 7: _t->scrollToBottomRequested(); break;
        case 8: _t->callRequested(); break;
        case 9: _t->editMessageRequested((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 10: _t->deleteMessageRequested((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 11: _t->onSearchTriggered((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 12: _t->onChatContextMenuRequested((*reinterpret_cast<std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 13: _t->onMessageDoubleClicked((*reinterpret_cast<std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 14: _t->onChatScrolled((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 15: _t->showSearchUI(); break;
        case 16: _t->hideSearchUI(); break;
        case 17: _t->updateHeader((*reinterpret_cast<std::add_pointer_t<User>>(_a[1]))); break;
        case 18: _t->setEditMode((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 19: _t->setEditMode((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 20: _t->clearReplyUI(); break;
        case 21: _t->showReplyUI((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 22: _t->hideReplyUI(); break;
        case 23: _t->onNewMessageReceived(); break;
        case 24: _t->scrollToBottom(); break;
        case 25: _t->scrollToMessage((*reinterpret_cast<std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 26: _t->onScrollDownButtonClicked(); break;
        case 27: _t->onCallButtonClicked(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ChatViewWidget::*)(const QString & )>(_a, &ChatViewWidget::sendMessageRequested, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChatViewWidget::*)()>(_a, &ChatViewWidget::headerClicked, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChatViewWidget::*)()>(_a, &ChatViewWidget::searchButtonClicked, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChatViewWidget::*)(qint64 )>(_a, &ChatViewWidget::replyToMessageRequested, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChatViewWidget::*)()>(_a, &ChatViewWidget::replyCancelled, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChatViewWidget::*)(const QString & )>(_a, &ChatViewWidget::searchTextEntered, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChatViewWidget::*)()>(_a, &ChatViewWidget::scrollToUnreadRequested, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChatViewWidget::*)()>(_a, &ChatViewWidget::scrollToBottomRequested, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChatViewWidget::*)()>(_a, &ChatViewWidget::callRequested, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChatViewWidget::*)(qint64 , const QString & )>(_a, &ChatViewWidget::editMessageRequested, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChatViewWidget::*)(qint64 )>(_a, &ChatViewWidget::deleteMessageRequested, 10))
            return;
    }
}

const QMetaObject *ChatViewWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ChatViewWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14ChatViewWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ChatViewWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 28)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 28;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 28)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 28;
    }
    return _id;
}

// SIGNAL 0
void ChatViewWidget::sendMessageRequested(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void ChatViewWidget::headerClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void ChatViewWidget::searchButtonClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void ChatViewWidget::replyToMessageRequested(qint64 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void ChatViewWidget::replyCancelled()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void ChatViewWidget::searchTextEntered(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void ChatViewWidget::scrollToUnreadRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void ChatViewWidget::scrollToBottomRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void ChatViewWidget::callRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void ChatViewWidget::editMessageRequested(qint64 _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1, _t2);
}

// SIGNAL 10
void ChatViewWidget::deleteMessageRequested(qint64 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1);
}
QT_WARNING_POP
