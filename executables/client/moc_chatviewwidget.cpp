/****************************************************************************
** Meta object code from reading C++ file 'chatviewwidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../chatviewwidget.h"
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
        "editMessageRequested",
        "oldText",
        "deleteMessageRequested",
        "replyCancelled",
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
        "onSearchTriggered",
        "onChatContextMenuRequested",
        "QPoint",
        "pos",
        "onMessageDoubleClicked",
        "QModelIndex",
        "index",
        "onChatScrolled",
        "value"
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
        // Signal 'editMessageRequested'
        QtMocHelpers::SignalData<void(qint64, const QString &)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 7 }, { QMetaType::QString, 9 },
        }}),
        // Signal 'deleteMessageRequested'
        QtMocHelpers::SignalData<void(qint64)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 7 },
        }}),
        // Signal 'replyCancelled'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'updateHeader'
        QtMocHelpers::SlotData<void(const User &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 13, 14 },
        }}),
        // Slot 'setEditMode'
        QtMocHelpers::SlotData<void(bool, const QString &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 16 }, { QMetaType::QString, 3 },
        }}),
        // Slot 'setEditMode'
        QtMocHelpers::SlotData<void(bool)>(15, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::Bool, 16 },
        }}),
        // Slot 'clearReplyUI'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'showReplyUI'
        QtMocHelpers::SlotData<void(const QString &, const QString &)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 19 }, { QMetaType::QString, 3 },
        }}),
        // Slot 'hideReplyUI'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onNewMessageReceived'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'scrollToBottom'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onSearchTriggered'
        QtMocHelpers::SlotData<void(const QString &)>(23, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Slot 'onChatContextMenuRequested'
        QtMocHelpers::SlotData<void(const QPoint &)>(24, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 25, 26 },
        }}),
        // Slot 'onMessageDoubleClicked'
        QtMocHelpers::SlotData<void(const QModelIndex &)>(27, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 28, 29 },
        }}),
        // Slot 'onChatScrolled'
        QtMocHelpers::SlotData<void(int)>(30, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 31 },
        }}),
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
        case 4: _t->editMessageRequested((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 5: _t->deleteMessageRequested((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 6: _t->replyCancelled(); break;
        case 7: _t->updateHeader((*reinterpret_cast<std::add_pointer_t<User>>(_a[1]))); break;
        case 8: _t->setEditMode((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 9: _t->setEditMode((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 10: _t->clearReplyUI(); break;
        case 11: _t->showReplyUI((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 12: _t->hideReplyUI(); break;
        case 13: _t->onNewMessageReceived(); break;
        case 14: _t->scrollToBottom(); break;
        case 15: _t->onSearchTriggered((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: _t->onChatContextMenuRequested((*reinterpret_cast<std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 17: _t->onMessageDoubleClicked((*reinterpret_cast<std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 18: _t->onChatScrolled((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
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
        if (QtMocHelpers::indexOfMethod<void (ChatViewWidget::*)(qint64 , const QString & )>(_a, &ChatViewWidget::editMessageRequested, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChatViewWidget::*)(qint64 )>(_a, &ChatViewWidget::deleteMessageRequested, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (ChatViewWidget::*)()>(_a, &ChatViewWidget::replyCancelled, 6))
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
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 19;
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
void ChatViewWidget::editMessageRequested(qint64 _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void ChatViewWidget::deleteMessageRequested(qint64 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void ChatViewWidget::replyCancelled()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}
QT_WARNING_POP
