/****************************************************************************
** Meta object code from reading C++ file 'EraserTool.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/tools/EraserTool.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'EraserTool.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_EraserTool_t {
    uint offsetsAndSizes[8];
    char stringdata0[11];
    char stringdata1[15];
    char stringdata2[1];
    char stringdata3[5];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_EraserTool_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_EraserTool_t qt_meta_stringdata_EraserTool = {
    {
        QT_MOC_LITERAL(0, 10),  // "EraserTool"
        QT_MOC_LITERAL(11, 14),  // "eraseRequested"
        QT_MOC_LITERAL(26, 0),  // ""
        QT_MOC_LITERAL(27, 4)   // "uuid"
    },
    "EraserTool",
    "eraseRequested",
    "",
    "uuid"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_EraserTool[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   20,    2, 0x06,    1 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QUuid,    3,

       0        // eod
};

Q_CONSTINIT const QMetaObject EraserTool::staticMetaObject = { {
    QMetaObject::SuperData::link<DrawTool::staticMetaObject>(),
    qt_meta_stringdata_EraserTool.offsetsAndSizes,
    qt_meta_data_EraserTool,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_EraserTool_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<EraserTool, std::true_type>,
        // method 'eraseRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QUuid, std::false_type>
    >,
    nullptr
} };

void EraserTool::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<EraserTool *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->eraseRequested((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (EraserTool::*)(QUuid );
            if (_t _q_method = &EraserTool::eraseRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *EraserTool::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *EraserTool::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_EraserTool.stringdata0))
        return static_cast<void*>(this);
    return DrawTool::qt_metacast(_clname);
}

int EraserTool::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DrawTool::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void EraserTool::eraseRequested(QUuid _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
