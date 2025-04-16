/****************************************************************************
** Meta object code from reading C++ file 'smartpowercontroller.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../smartpowercontroller.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'smartpowercontroller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SmartPowerController_t {
    QByteArrayData data[14];
    char stringdata0[160];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SmartPowerController_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SmartPowerController_t qt_meta_stringdata_SmartPowerController = {
    {
QT_MOC_LITERAL(0, 0, 20), // "SmartPowerController"
QT_MOC_LITERAL(1, 21, 18), // "deviceCountChanged"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 5), // "count"
QT_MOC_LITERAL(4, 47, 23), // "connectionStatusChanged"
QT_MOC_LITERAL(5, 71, 9), // "connected"
QT_MOC_LITERAL(6, 81, 19), // "outputStatusChanged"
QT_MOC_LITERAL(7, 101, 2), // "on"
QT_MOC_LITERAL(8, 104, 13), // "statusUpdated"
QT_MOC_LITERAL(9, 118, 7), // "voltage"
QT_MOC_LITERAL(10, 126, 7), // "current"
QT_MOC_LITERAL(11, 134, 3), // "bnc"
QT_MOC_LITERAL(12, 138, 13), // "errorOccurred"
QT_MOC_LITERAL(13, 152, 7) // "message"

    },
    "SmartPowerController\0deviceCountChanged\0"
    "\0count\0connectionStatusChanged\0connected\0"
    "outputStatusChanged\0on\0statusUpdated\0"
    "voltage\0current\0bnc\0errorOccurred\0"
    "message"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SmartPowerController[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       4,    1,   42,    2, 0x06 /* Public */,
       6,    1,   45,    2, 0x06 /* Public */,
       8,    3,   48,    2, 0x06 /* Public */,
      12,    1,   55,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::Bool,    7,
    QMetaType::Void, QMetaType::Double, QMetaType::Double, QMetaType::Double,    9,   10,   11,
    QMetaType::Void, QMetaType::QString,   13,

       0        // eod
};

void SmartPowerController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SmartPowerController *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->deviceCountChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->connectionStatusChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->outputStatusChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->statusUpdated((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 4: _t->errorOccurred((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SmartPowerController::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SmartPowerController::deviceCountChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SmartPowerController::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SmartPowerController::connectionStatusChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (SmartPowerController::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SmartPowerController::outputStatusChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (SmartPowerController::*)(double , double , double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SmartPowerController::statusUpdated)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (SmartPowerController::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SmartPowerController::errorOccurred)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SmartPowerController::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_SmartPowerController.data,
    qt_meta_data_SmartPowerController,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SmartPowerController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SmartPowerController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SmartPowerController.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SmartPowerController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void SmartPowerController::deviceCountChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SmartPowerController::connectionStatusChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void SmartPowerController::outputStatusChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void SmartPowerController::statusUpdated(double _t1, double _t2, double _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void SmartPowerController::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
