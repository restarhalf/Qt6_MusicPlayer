//
// Created by restarhalf on 25-6-28.
//

#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QWidget>
#include <QStandardItemModel>
#include <QMediaPlayer>
#include <QAudioOutput>
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

private:
    Ui::MusicPlayer *ui;
    QStandardItemModel *lrcmodel;
    QStandardItemModel *musicmodel;
    QMediaPlayer *player=nullptr;
    QAudioOutput *audioOutput=nullptr;
    bool isSliderBeingDragged = false; // 跟踪是否正在拖动进度条

    // 歌词相关
    struct LyricLine {
        qint64 timestamp;  // 时间戳（毫秒）
        QString text;      // 歌词文本
    };

    QList<LyricLine> lyrics;
    int currentLyricIndex;
    int currentIndex;
    QTimer* lyricTimer;

    // TagLib 相关方法
    QString extractLyricsFromFile(const QString& filePath);
    void loadLyricsFromMetadata(const QString& filePath);
    void parseLrcFile(const QString& lrcFilePath);
    void parseLrcContent(const QString& lrcContent);
    QString findLrcFile(const QString& musicFilePath);
void updateCurrentLyric();
};
} // rsh

#endif //MUSICPLAYER_H
