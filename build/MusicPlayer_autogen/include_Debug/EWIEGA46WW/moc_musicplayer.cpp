/****************************************************************************
** Meta object code from reading C++ file 'musicplayer.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../musicplayer.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'musicplayer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.1. It"
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
struct qt_meta_tag_ZN3rsh11MusicPlayerE_t {};
} // unnamed namespace

template <> constexpr inline auto rsh::MusicPlayer::qt_create_metaobjectdata<qt_meta_tag_ZN3rsh11MusicPlayerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "rsh::MusicPlayer",
        "DirBtn_clicked",
        "",
        "musicName_clicked",
        "QModelIndex",
        "index",
        "StartBtn_clicked",
        "PrewBtn_clicked",
        "NextBtn_clicked",
        "VoiceBtn_clicked",
        "Audio_release",
        "Audio_pressed",
        "Audio_valueChanged",
        "value",
        "Voice_valueChanged",
        "Lrc_clicked",
        "onDurationChanged",
        "duration",
        "onPositionChanged",
        "position",
        "onMediaStatusChanged",
        "QMediaPlayer::MediaStatus",
        "status",
        "onPlaybackStateChanged",
        "QMediaPlayer::PlaybackState",
        "state",
        "updateCurrentLyric",
        "updatePlaybackState",
        "updateVolumeDisplay",
        "onVolumeButtonEntered",
        "onVolumeButtonLeft",
        "onVolumeSliderEntered",
        "onVolumeSliderLeft",
        "hideVolumeSlider",
        "getMusicTitle",
        "filePath",
        "getArtistName",
        "getAlbumName",
        "updateMusicInfo"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'DirBtn_clicked'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'musicName_clicked'
        QtMocHelpers::SlotData<void(const QModelIndex &)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 4, 5 },
        }}),
        // Slot 'StartBtn_clicked'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'PrewBtn_clicked'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'NextBtn_clicked'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'VoiceBtn_clicked'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'Audio_release'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'Audio_pressed'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'Audio_valueChanged'
        QtMocHelpers::SlotData<void(int)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 13 },
        }}),
        // Slot 'Voice_valueChanged'
        QtMocHelpers::SlotData<void(int)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 13 },
        }}),
        // Slot 'Lrc_clicked'
        QtMocHelpers::SlotData<void(const QModelIndex &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 4, 5 },
        }}),
        // Slot 'onDurationChanged'
        QtMocHelpers::SlotData<void(qint64)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 17 },
        }}),
        // Slot 'onPositionChanged'
        QtMocHelpers::SlotData<void(qint64)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 19 },
        }}),
        // Slot 'onMediaStatusChanged'
        QtMocHelpers::SlotData<void(QMediaPlayer::MediaStatus)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 21, 22 },
        }}),
        // Slot 'onPlaybackStateChanged'
        QtMocHelpers::SlotData<void(QMediaPlayer::PlaybackState)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 24, 25 },
        }}),
        // Slot 'updateCurrentLyric'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'updatePlaybackState'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'updateVolumeDisplay'
        QtMocHelpers::SlotData<void()>(28, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onVolumeButtonEntered'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onVolumeButtonLeft'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onVolumeSliderEntered'
        QtMocHelpers::SlotData<void()>(31, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onVolumeSliderLeft'
        QtMocHelpers::SlotData<void()>(32, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'hideVolumeSlider'
        QtMocHelpers::SlotData<void()>(33, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'getMusicTitle'
        QtMocHelpers::SlotData<QString(const QString &)>(34, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::QString, 35 },
        }}),
        // Slot 'getArtistName'
        QtMocHelpers::SlotData<QString(const QString &)>(36, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::QString, 35 },
        }}),
        // Slot 'getAlbumName'
        QtMocHelpers::SlotData<QString(const QString &)>(37, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::QString, 35 },
        }}),
        // Slot 'updateMusicInfo'
        QtMocHelpers::SlotData<void(const QString &)>(38, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 35 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MusicPlayer, qt_meta_tag_ZN3rsh11MusicPlayerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject rsh::MusicPlayer::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3rsh11MusicPlayerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3rsh11MusicPlayerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN3rsh11MusicPlayerE_t>.metaTypes,
    nullptr
} };

void rsh::MusicPlayer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MusicPlayer *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->DirBtn_clicked(); break;
        case 1: _t->musicName_clicked((*reinterpret_cast< std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 2: _t->StartBtn_clicked(); break;
        case 3: _t->PrewBtn_clicked(); break;
        case 4: _t->NextBtn_clicked(); break;
        case 5: _t->VoiceBtn_clicked(); break;
        case 6: _t->Audio_release(); break;
        case 7: _t->Audio_pressed(); break;
        case 8: _t->Audio_valueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->Voice_valueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 10: _t->Lrc_clicked((*reinterpret_cast< std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 11: _t->onDurationChanged((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 12: _t->onPositionChanged((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 13: _t->onMediaStatusChanged((*reinterpret_cast< std::add_pointer_t<QMediaPlayer::MediaStatus>>(_a[1]))); break;
        case 14: _t->onPlaybackStateChanged((*reinterpret_cast< std::add_pointer_t<QMediaPlayer::PlaybackState>>(_a[1]))); break;
        case 15: _t->updateCurrentLyric(); break;
        case 16: _t->updatePlaybackState(); break;
        case 17: _t->updateVolumeDisplay(); break;
        case 18: _t->onVolumeButtonEntered(); break;
        case 19: _t->onVolumeButtonLeft(); break;
        case 20: _t->onVolumeSliderEntered(); break;
        case 21: _t->onVolumeSliderLeft(); break;
        case 22: _t->hideVolumeSlider(); break;
        case 23: { QString _r = _t->getMusicTitle((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 24: { QString _r = _t->getArtistName((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 25: { QString _r = _t->getAlbumName((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 26: _t->updateMusicInfo((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *rsh::MusicPlayer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *rsh::MusicPlayer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3rsh11MusicPlayerE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int rsh::MusicPlayer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 27)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 27;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 27)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 27;
    }
    return _id;
}
QT_WARNING_POP
