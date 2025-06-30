//
// Created by restarhalf on 25-6-28.
//

#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QWidget>
#include <QStandardItemModel>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QScrollBar>

namespace rsh {
QT_BEGIN_NAMESPACE
namespace Ui { class MusicPlayer; }

class MusicPlayer : public QWidget {
Q_OBJECT

public:
    explicit MusicPlayer(QWidget *parent = nullptr);
    ~MusicPlayer() override;
    public slots:
    void DirBtn_clicked();
    void musicName_clicked(const QModelIndex &index);
    void PauseBtn_clicked();
    void StartBtn_clicked();
    void PrewBtn_clicked();
    void NextBtn_clicked();
    void VoiceBtn_clicked();
    void Audio_release();
    void Audio_pressed(); // 当用户开始拖动进度条时
    void Audio_valueChanged(int value); // 当进度条值改变时
    void Voice_valueChanged(int value); // 音量调节

    // Qt6兼容的槽函数
    void onDurationChanged(qint64 duration);
    void onPositionChanged(qint64 position);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);

    // 定时器相关的槽函数
    void updateCurrentLyric();
    void updatePlaybackState();
    void updateVolumeDisplay();

QString getMusicTitle(const QString& filePath);

private:
    Ui::MusicPlayer *ui;
    QStandardItemModel *lrcmodel = nullptr;
    QStandardItemModel *musicmodel = nullptr;
    QMediaPlayer *player = nullptr;
    QAudioOutput *audioOutput = nullptr;
    bool isSliderBeingDragged = false; // 跟踪是否正在拖动进度条

    // 歌词相关
    struct LyricLine {
        qint64 timestamp;  // 时间戳（毫秒）
        QString text;      // 歌词文本
    };

    QList<LyricLine> lyrics;
    int currentLyricIndex = -1;
    int currentIndex = -1;

    QTimer* lyricTimer = nullptr;
    QTimer* uiUpdateTimer = nullptr;  // UI更新定时器
    QTimer* playbackTimer = nullptr;  // 播放状态检查定时器

    // 播放状态管理
    enum PlaybackMode {
        Sequential,   // 顺序播放
        Loop,        // 单曲循环
        LoopAll,     // 列表循环
        Random       // 随机播放
    };
    PlaybackMode currentPlaybackMode = Sequential;

    // 播放列表管理
    QList<int> playHistory;  // 播放历史
    int historyIndex = -1;   // 历史索引

    // TagLib 相关方法
    QString extractLyricsFromFile(const QString& filePath);
    void loadLyricsFromMetadata(const QString& filePath);
    void parseLrcFile(const QString& lrcFilePath);
    void parseLrcContent(const QString& lrcContent);
    QString findLrcFile(const QString& musicFilePath);

    // 新增的歌词美化方法
    void updateLyricHighlight();
    void smoothScrollToLyric(int lyricIndex);
    void pulseCurrentLyric();

    // 新增的更新流方法
    void updatePlayerInfo(const QString& filePath);
    void handlePlaybackFinished();
    void updatePlaybackMode(PlaybackMode mode);
    void refreshMusicList();
    void savePlayerState();
    void loadPlayerState();

    // 动画相关
    QPropertyAnimation *fadeAnimation = nullptr;
    QGraphicsOpacityEffect *opacityEffect = nullptr;
    QScrollBar *scrollBar = nullptr;

    void setupAnimations();
    void fadeInWidget(QWidget *widget, int duration);
    void fadeOutWidget(QWidget *widget, int duration);
};
} // rsh

#endif //MUSICPLAYER_H
