/****************************************************************************
** Meta object code from reading C++ file 'BoardModel.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/data/BoardModel.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'BoardModel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_BoardModel_t {
    uint offsetsAndSizes[16];
    char stringdata0[11];
    char stringdata1[12];
    char stringdata2[1];
    char stringdata3[28];
    char stringdata4[4];
    char stringdata5[14];
    char stringdata6[5];
    char stringdata7[13];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_BoardModel_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_BoardModel_t qt_meta_stringdata_BoardModel = {
    {
        QT_MOC_LITERAL(0, 10),  // "BoardModel"
        QT_MOC_LITERAL(11, 11),  // "objectAdded"
        QT_MOC_LITERAL(23, 0),  // ""
        QT_MOC_LITERAL(24, 27),  // "std::shared_ptr<DrawObject>"
        QT_MOC_LITERAL(52, 3),  // "obj"
        QT_MOC_LITERAL(56, 13),  // "objectRemoved"
        QT_MOC_LITERAL(70, 4),  // "uuid"
        QT_MOC_LITERAL(75, 12)   // "boardCleared"
    },
    "BoardModel",
    "objectAdded",
    "",
    "std::shared_ptr<DrawObject>",
    "obj",
    "objectRemoved",
    "uuid",
    "boardCleared"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_BoardModel[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   32,    2, 0x06,    1 /* Public */,
       5,    1,   35,    2, 0x06,    3 /* Public */,
       7,    0,   38,    2, 0x06,    5 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QUuid,    6,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject BoardModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_BoardModel.offsetsAndSizes,
    qt_meta_data_BoardModel,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_BoardModel_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<BoardModel, std::true_type>,
        // method 'objectAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<std::shared_ptr<DrawObject>, std::false_type>,
        // method 'objectRemoved'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QUuid, std::false_type>,
        // method 'boardCleared'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void BoardModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<BoardModel *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->objectAdded((*reinterpret_cast< std::add_pointer_t<std::shared_ptr<DrawObject>>>(_a[1]))); break;
        case 1: _t->objectRemoved((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1]))); break;
        case 2: _t->boardCleared(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (BoardModel::*)(std::shared_ptr<DrawObject> );
            if (_t _q_method = &BoardModel::objectAdded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (BoardModel::*)(QUuid );
            if (_t _q_method = &BoardModel::objectRemoved; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (BoardModel::*)();
            if (_t _q_method = &BoardModel::boardCleared; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *BoardModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BoardModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_BoardModel.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int BoardModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void BoardModel::objectAdded(std::shared_ptr<DrawObject> _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void BoardModel::objectRemoved(QUuid _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void BoardModel::boardCleared()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
