/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[22];
    char stringdata0[314];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 10), // "updateTime"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 22), // "onConnectButtonClicked"
QT_MOC_LITERAL(4, 46, 23), // "onOutputOnButtonClicked"
QT_MOC_LITERAL(5, 70, 24), // "onOutputOffButtonClicked"
QT_MOC_LITERAL(6, 95, 18), // "updateDeviceStatus"
QT_MOC_LITERAL(7, 114, 15), // "combo_changed_5"
QT_MOC_LITERAL(8, 130, 5), // "index"
QT_MOC_LITERAL(9, 136, 15), // "combo_changed_8"
QT_MOC_LITERAL(10, 152, 21), // "start_test_content_11"
QT_MOC_LITERAL(11, 174, 21), // "start_test_content_12"
QT_MOC_LITERAL(12, 196, 18), // "start_test_content"
QT_MOC_LITERAL(13, 215, 10), // "resetTable"
QT_MOC_LITERAL(14, 226, 14), // "onTestBootTime"
QT_MOC_LITERAL(15, 241, 3), // "row"
QT_MOC_LITERAL(16, 245, 2), // "on"
QT_MOC_LITERAL(17, 248, 7), // "voltage"
QT_MOC_LITERAL(18, 256, 15), // "onTestSoftReset"
QT_MOC_LITERAL(19, 272, 14), // "updatedeviceId"
QT_MOC_LITERAL(20, 287, 8), // "deviceId"
QT_MOC_LITERAL(21, 296, 17) // "showSNInputDialog"

    },
    "MainWindow\0updateTime\0\0onConnectButtonClicked\0"
    "onOutputOnButtonClicked\0"
    "onOutputOffButtonClicked\0updateDeviceStatus\0"
    "combo_changed_5\0index\0combo_changed_8\0"
    "start_test_content_11\0start_test_content_12\0"
    "start_test_content\0resetTable\0"
    "onTestBootTime\0row\0on\0voltage\0"
    "onTestSoftReset\0updatedeviceId\0deviceId\0"
    "showSNInputDialog"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   94,    2, 0x08 /* Private */,
       3,    0,   95,    2, 0x08 /* Private */,
       4,    0,   96,    2, 0x08 /* Private */,
       5,    0,   97,    2, 0x08 /* Private */,
       6,    0,   98,    2, 0x08 /* Private */,
       7,    1,   99,    2, 0x08 /* Private */,
       9,    1,  102,    2, 0x08 /* Private */,
      10,    0,  105,    2, 0x08 /* Private */,
      11,    0,  106,    2, 0x08 /* Private */,
      12,    0,  107,    2, 0x08 /* Private */,
      13,    0,  108,    2, 0x08 /* Private */,
      14,    3,  109,    2, 0x08 /* Private */,
      14,    2,  116,    2, 0x28 /* Private | MethodCloned */,
      18,    2,  121,    2, 0x08 /* Private */,
      19,    1,  126,    2, 0x08 /* Private */,
      21,    0,  129,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool, QMetaType::Int,   15,   16,   17,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,   15,   16,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,   15,   16,
    QMetaType::Void, QMetaType::QString,   20,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->updateTime(); break;
        case 1: _t->onConnectButtonClicked(); break;
        case 2: _t->onOutputOnButtonClicked(); break;
        case 3: _t->onOutputOffButtonClicked(); break;
        case 4: _t->updateDeviceStatus(); break;
        case 5: _t->combo_changed_5((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->combo_changed_8((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->start_test_content_11(); break;
        case 8: _t->start_test_content_12(); break;
        case 9: _t->start_test_content(); break;
        case 10: _t->resetTable(); break;
        case 11: _t->onTestBootTime((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 12: _t->onTestBootTime((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 13: _t->onTestSoftReset((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 14: _t->updatedeviceId((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 15: _t->showSNInputDialog(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    &QMainWindow::staticMetaObject,
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 16;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
