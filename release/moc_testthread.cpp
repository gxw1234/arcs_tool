/****************************************************************************
** Meta object code from reading C++ file 'testthread.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../testthread.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'testthread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TestThread_t {
    QByteArrayData data[21];
    char stringdata0[215];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TestThread_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TestThread_t qt_meta_stringdata_TestThread = {
    {
QT_MOC_LITERAL(0, 0, 10), // "TestThread"
QT_MOC_LITERAL(1, 11, 9), // "updateLog"
QT_MOC_LITERAL(2, 21, 0), // ""
QT_MOC_LITERAL(3, 22, 7), // "message"
QT_MOC_LITERAL(4, 30, 14), // "updateProgress"
QT_MOC_LITERAL(5, 45, 3), // "row"
QT_MOC_LITERAL(6, 49, 5), // "value"
QT_MOC_LITERAL(7, 55, 12), // "updateResult"
QT_MOC_LITERAL(8, 68, 6), // "result"
QT_MOC_LITERAL(9, 75, 14), // "updateBootTime"
QT_MOC_LITERAL(10, 90, 2), // "on"
QT_MOC_LITERAL(11, 93, 7), // "voltage"
QT_MOC_LITERAL(12, 101, 12), // "highlightRow"
QT_MOC_LITERAL(13, 114, 15), // "updateSoftReset"
QT_MOC_LITERAL(14, 130, 14), // "updatedeviceId"
QT_MOC_LITERAL(15, 145, 17), // "updateTestContent"
QT_MOC_LITERAL(16, 163, 7), // "content"
QT_MOC_LITERAL(17, 171, 13), // "updateipValue"
QT_MOC_LITERAL(18, 185, 8), // "ipresult"
QT_MOC_LITERAL(19, 194, 8), // "testtime"
QT_MOC_LITERAL(20, 203, 11) // "requestStop"

    },
    "TestThread\0updateLog\0\0message\0"
    "updateProgress\0row\0value\0updateResult\0"
    "result\0updateBootTime\0on\0voltage\0"
    "highlightRow\0updateSoftReset\0"
    "updatedeviceId\0updateTestContent\0"
    "content\0updateipValue\0ipresult\0testtime\0"
    "requestStop"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TestThread[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      10,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   69,    2, 0x06 /* Public */,
       4,    2,   72,    2, 0x06 /* Public */,
       7,    2,   77,    2, 0x06 /* Public */,
       9,    3,   82,    2, 0x06 /* Public */,
       9,    2,   89,    2, 0x26 /* Public | MethodCloned */,
      12,    1,   94,    2, 0x06 /* Public */,
      13,    2,   97,    2, 0x06 /* Public */,
      14,    1,  102,    2, 0x06 /* Public */,
      15,    2,  105,    2, 0x06 /* Public */,
      17,    3,  110,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      20,    0,  117,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    5,    6,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,    5,    8,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool, QMetaType::Int,    5,   10,   11,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    5,   10,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    5,   10,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,    5,   16,
    QMetaType::Void, QMetaType::QString, QMetaType::Int, QMetaType::Int,   18,    5,   19,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void TestThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TestThread *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->updateLog((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->updateProgress((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->updateResult((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 3: _t->updateBootTime((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 4: _t->updateBootTime((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 5: _t->highlightRow((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->updateSoftReset((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 7: _t->updatedeviceId((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->updateTestContent((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 9: _t->updateipValue((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 10: _t->requestStop(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TestThread::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TestThread::updateLog)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (TestThread::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TestThread::updateProgress)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (TestThread::*)(int , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TestThread::updateResult)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (TestThread::*)(int , bool , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TestThread::updateBootTime)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (TestThread::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TestThread::highlightRow)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (TestThread::*)(int , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TestThread::updateSoftReset)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (TestThread::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TestThread::updatedeviceId)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (TestThread::*)(int , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TestThread::updateTestContent)) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (TestThread::*)(const QString & , int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TestThread::updateipValue)) {
                *result = 9;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject TestThread::staticMetaObject = { {
    &QThread::staticMetaObject,
    qt_meta_stringdata_TestThread.data,
    qt_meta_data_TestThread,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *TestThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TestThread::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TestThread.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int TestThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void TestThread::updateLog(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void TestThread::updateProgress(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void TestThread::updateResult(int _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void TestThread::updateBootTime(int _t1, bool _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 5
void TestThread::highlightRow(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void TestThread::updateSoftReset(int _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void TestThread::updatedeviceId(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void TestThread::updateTestContent(int _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void TestThread::updateipValue(const QString & _t1, int _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
