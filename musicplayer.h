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
#include <QEvent>
#include <QCursor>
#include <QSet>

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
    void StartBtn_clicked();
    void PrewBtn_clicked();
    void NextBtn_clicked();
    void VoiceBtn_clicked();
    void Audio_release();
    void Audio_pressed(); // 当用户开始拖动进度条时
    void Audio_valueChanged(int value); // 当进度条值改变时
    void Voice_valueChanged(int value); // 音量调节
    void Lrc_clicked(const QModelIndex &index); // 歌词点击槽函数

    // Qt6兼容的槽函数
    void onDurationChanged(qint64 duration);
    void onPositionChanged(qint64 position);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);

    // 定时器相关的槽函数
    void updateCurrentLyric();
    void updatePlaybackState();
    void updateVolumeDisplay();

    // 音量控制事件处理
    void onVolumeButtonEntered(); // 鼠标进入音量按钮
    void onVolumeButtonLeft(); // 鼠标离开音量按钮
    void onVolumeSliderEntered(); // 鼠标进入音量滑块
    void onVolumeSliderLeft(); // 鼠标离开音量滑块

    QString getMusicTitle(const QString& filePath);
    QString getArtistName(const QString& filePath);
    QString getAlbumName(const QString& filePath);
    void updateMusicInfo(const QString& filePath);

private:
    Ui::MusicPlayer *ui;
    QStandardItemModel *lrcmodel = nullptr;
    QStandardItemModel *musicmodel = nullptr;
    QMediaPlayer *player = nullptr;
    QAudioOutput *audioOutput = nullptr;
    bool isSliderBeingDragged = false; // 跟踪是否正在拖动进度条
    bool m_isValid = false; // 对象有效性标志

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
    void addLyricPaddingItems(); // 添加歌词填充项，使歌词能够居中显示
    int calculatePaddingItemsCount(); // 计算需要的填充项数量

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

    // 新增：歌词平滑滚动相关
    QPropertyAnimation *lyricScrollAnimation = nullptr;
    QTimer *lyricScrollTimer = nullptr;
    int targetScrollPosition = 0;
    bool isUserScrolling = false;

    // 音量控制相关
    int lastVolume = 50;
    bool isMuted = false; // 静音状态
    QTimer* volumeHideTimer = nullptr; // 音量滑块自动隐藏定时器
    bool isVolumeSliderBeingDragged = false; // 跟踪音量滑块是否正在被拖拽

    void setupAnimations();
    void fadeInWidget(QWidget *widget, int duration);
    void fadeOutWidget(QWidget *widget, int duration);
    void updateVolumeIcon(int volume);
    void showVolumeSlider(); // 显示音量滑块
    void hideVolumeSlider(); // 隐藏音量滑块

protected:
    // 事件过滤器，处理鼠标悬停事件
    bool eventFilter(QObject *obj, QEvent *event) override;
};
} // rsh

#endif //MUSICPLAYER_H
