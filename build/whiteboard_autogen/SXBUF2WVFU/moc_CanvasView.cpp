/****************************************************************************
** Meta object code from reading C++ file 'CanvasView.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/canvas/CanvasView.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CanvasView.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CanvasView_t {
    uint offsetsAndSizes[26];
    char stringdata0[11];
    char stringdata1[12];
    char stringdata2[1];
    char stringdata3[8];
    char stringdata4[12];
    char stringdata5[5];
    char stringdata6[14];
    char stringdata7[28];
    char stringdata8[4];
    char stringdata9[14];
    char stringdata10[16];
    char stringdata11[5];
    char stringdata12[15];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CanvasView_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CanvasView_t qt_meta_stringdata_CanvasView = {
    {
        QT_MOC_LITERAL(0, 10),  // "CanvasView"
        QT_MOC_LITERAL(11, 11),  // "zoomChanged"
        QT_MOC_LITERAL(23, 0),  // ""
        QT_MOC_LITERAL(24, 7),  // "percent"
        QT_MOC_LITERAL(32, 11),  // "toolChanged"
        QT_MOC_LITERAL(44, 4),  // "name"
        QT_MOC_LITERAL(49, 13),  // "objectCreated"
        QT_MOC_LITERAL(63, 27),  // "std::shared_ptr<DrawObject>"
        QT_MOC_LITERAL(91, 3),  // "obj"
        QT_MOC_LITERAL(95, 13),  // "onObjectAdded"
        QT_MOC_LITERAL(109, 15),  // "onObjectRemoved"
        QT_MOC_LITERAL(125, 4),  // "uuid"
        QT_MOC_LITERAL(130, 14)   // "onBoardCleared"
    },
    "CanvasView",
    "zoomChanged",
    "",
    "percent",
    "toolChanged",
    "name",
    "objectCreated",
    "std::shared_ptr<DrawObject>",
    "obj",
    "onObjectAdded",
    "onObjectRemoved",
    "uuid",
    "onBoardCleared"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CanvasView[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   50,    2, 0x06,    1 /* Public */,
       4,    1,   53,    2, 0x06,    3 /* Public */,
       6,    1,   56,    2, 0x06,    5 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       9,    1,   59,    2, 0x0a,    7 /* Public */,
      10,    1,   62,    2, 0x0a,    9 /* Public */,
      12,    0,   65,    2, 0x0a,   11 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, 0x80000000 | 7,    8,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, QMetaType::QUuid,   11,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject CanvasView::staticMetaObject = { {
    QMetaObject::SuperData::link<QGraphicsView::staticMetaObject>(),
    qt_meta_stringdata_CanvasView.offsetsAndSizes,
    qt_meta_data_CanvasView,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CanvasView_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<CanvasView, std::true_type>,
        // method 'zoomChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'toolChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'objectCreated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<std::shared_ptr<DrawObject>, std::false_type>,
        // method 'onObjectAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<std::shared_ptr<DrawObject>, std::false_type>,
        // method 'onObjectRemoved'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QUuid, std::false_type>,
        // method 'onBoardCleared'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void CanvasView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CanvasView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->zoomChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->toolChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->objectCreated((*reinterpret_cast< std::add_pointer_t<std::shared_ptr<DrawObject>>>(_a[1]))); break;
        case 3: _t->onObjectAdded((*reinterpret_cast< std::add_pointer_t<std::shared_ptr<DrawObject>>>(_a[1]))); break;
        case 4: _t->onObjectRemoved((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1]))); break;
        case 5: _t->onBoardCleared(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CanvasView::*)(int );
            if (_t _q_method = &CanvasView::zoomChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CanvasView::*)(const QString & );
            if (_t _q_method = &CanvasView::toolChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CanvasView::*)(std::shared_ptr<DrawObject> );
            if (_t _q_method = &CanvasView::objectCreated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *CanvasView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CanvasView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CanvasView.stringdata0))
        return static_cast<void*>(this);
    return QGraphicsView::qt_metacast(_clname);
}

int CanvasView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void CanvasView::zoomChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CanvasView::toolChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void CanvasView::objectCreated(std::shared_ptr<DrawObject> _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
