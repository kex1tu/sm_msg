/****************************************************************************
** Meta object code from reading C++ file 'chatmessagemodel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../models/chatmessagemodel.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'chatmessagemodel.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN16ChatMessageModelE_t {};
} // unnamed namespace

template <> constexpr inline auto ChatMessageModel::qt_create_metaobjectdata<qt_meta_tag_ZN16ChatMessageModelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ChatMessageModel",
        "messageNeedsReadReceipt",
        "",
        "messageId",
        "addMessage",
        "ChatMessage",
        "message",
        "addMessages",
        "QList<ChatMessage>",
        "messages",
        "prependMessages",
        "clearMessages",
        "removeMessage",
        "confirmMessage",
        "tempId",
        "confirmedMessage",
        "updateMessageStatus",
        "ChatMessage::MessageStatus",
        "newStatus",
        "editMessage",
        "newPayload",
        "getMessageById",
        "id",
        "ChatMessage&",
        "msg"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'messageNeedsReadReceipt'
        QtMocHelpers::SignalData<void(qint64)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 3 },
        }}),
        // Slot 'addMessage'
        QtMocHelpers::SlotData<void(const ChatMessage &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 5, 6 },
        }}),
        // Slot 'addMessages'
        QtMocHelpers::SlotData<void(const QList<ChatMessage> &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Slot 'prependMessages'
        QtMocHelpers::SlotData<void(const QList<ChatMessage> &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Slot 'clearMessages'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'removeMessage'
        QtMocHelpers::SlotData<void(qint64)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 3 },
        }}),
        // Slot 'confirmMessage'
        QtMocHelpers::SlotData<void(const QString &, const ChatMessage &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 14 }, { 0x80000000 | 5, 15 },
        }}),
        // Slot 'updateMessageStatus'
        QtMocHelpers::SlotData<void(qint64, ChatMessage::MessageStatus)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 3 }, { 0x80000000 | 17, 18 },
        }}),
        // Slot 'editMessage'
        QtMocHelpers::SlotData<void(qint64, const QString &)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 3 }, { QMetaType::QString, 20 },
        }}),
        // Slot 'getMessageById'
        QtMocHelpers::SlotData<bool(qint64, ChatMessage &) const>(21, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::LongLong, 22 }, { 0x80000000 | 23, 24 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ChatMessageModel, qt_meta_tag_ZN16ChatMessageModelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ChatMessageModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16ChatMessageModelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16ChatMessageModelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16ChatMessageModelE_t>.metaTypes,
    nullptr
} };

void ChatMessageModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ChatMessageModel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->messageNeedsReadReceipt((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 1: _t->addMessage((*reinterpret_cast<std::add_pointer_t<ChatMessage>>(_a[1]))); break;
        case 2: _t->addMessages((*reinterpret_cast<std::add_pointer_t<QList<ChatMessage>>>(_a[1]))); break;
        case 3: _t->prependMessages((*reinterpret_cast<std::add_pointer_t<QList<ChatMessage>>>(_a[1]))); break;
        case 4: _t->clearMessages(); break;
        case 5: _t->removeMessage((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 6: _t->confirmMessage((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<ChatMessage>>(_a[2]))); break;
        case 7: _t->updateMessageStatus((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<ChatMessage::MessageStatus>>(_a[2]))); break;
        case 8: _t->editMessage((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 9: { bool _r = _t->getMessageById((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<ChatMessage&>>(_a[2])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< ChatMessage >(); break;
            }
            break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<ChatMessage> >(); break;
            }
            break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<ChatMessage> >(); break;
            }
            break;
        case 6:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< ChatMessage >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ChatMessageModel::*)(qint64 )>(_a, &ChatMessageModel::messageNeedsReadReceipt, 0))
            return;
    }
}

const QMetaObject *ChatMessageModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ChatMessageModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16ChatMessageModelE_t>.strings))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int ChatMessageModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void ChatMessageModel::messageNeedsReadReceipt(qint64 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}
QT_WARNING_POP
