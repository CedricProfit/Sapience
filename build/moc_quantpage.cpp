/****************************************************************************
** Meta object code from reading C++ file 'quantpage.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.2.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/qt/quantpage.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'quantpage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.2.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_QuantPage_t {
    QByteArrayData data[9];
    char stringdata[182];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_QuantPage_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_QuantPage_t qt_meta_stringdata_QuantPage = {
    {
QT_MOC_LITERAL(0, 0, 9),
QT_MOC_LITERAL(1, 10, 19),
QT_MOC_LITERAL(2, 30, 0),
QT_MOC_LITERAL(3, 31, 22),
QT_MOC_LITERAL(4, 54, 22),
QT_MOC_LITERAL(5, 77, 24),
QT_MOC_LITERAL(6, 102, 24),
QT_MOC_LITERAL(7, 127, 27),
QT_MOC_LITERAL(8, 155, 25)
    },
    "QuantPage\0updateTimer_timeout\0\0"
    "eeHistoryReplyFinished\0eeMktDataReplyFinished\0"
    "trexHistoryReplyFinished\0"
    "trexMktDataReplyFinished\0"
    "trexMktHistoryReplyFinished\0"
    "eeMktHistoryReplyFinished\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QuantPage[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x08,
       3,    0,   50,    2, 0x08,
       4,    0,   51,    2, 0x08,
       5,    0,   52,    2, 0x08,
       6,    0,   53,    2, 0x08,
       7,    0,   54,    2, 0x08,
       8,    0,   55,    2, 0x08,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void QuantPage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QuantPage *_t = static_cast<QuantPage *>(_o);
        switch (_id) {
        case 0: _t->updateTimer_timeout(); break;
        case 1: _t->eeHistoryReplyFinished(); break;
        case 2: _t->eeMktDataReplyFinished(); break;
        case 3: _t->trexHistoryReplyFinished(); break;
        case 4: _t->trexMktDataReplyFinished(); break;
        case 5: _t->trexMktHistoryReplyFinished(); break;
        case 6: _t->eeMktHistoryReplyFinished(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject QuantPage::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_QuantPage.data,
      qt_meta_data_QuantPage,  qt_static_metacall, 0, 0}
};


const QMetaObject *QuantPage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QuantPage::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QuantPage.stringdata))
        return static_cast<void*>(const_cast< QuantPage*>(this));
    return QWidget::qt_metacast(_clname);
}

int QuantPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
