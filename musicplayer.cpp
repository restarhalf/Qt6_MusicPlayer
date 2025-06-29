//
// Created by restarhalf on 25-6-28.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MusicPlayer.h" resolved

#include "musicplayer.h"
#include "ui_MusicPlayer.h"
#include <QFileDialog>
#include <QDirIterator>
#include <QFileInfo>
#include <QTimer>
#include <QMediaMetaData>
#include <QRegularExpression>
#include <QTextStream>
#include <QFile>
#include <QAbstractItemView>
#include <QItemSelectionModel>
#include <QEventLoop>
#include <QDebug>
#include <algorithm>
#include <tag.h>
#include <fileref.h>
#include <tpropertymap.h>
#include <flacfile.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <id3v2frame.h>
#include <unsynchronizedlyricsframe.h>

namespace rsh
{
    MusicPlayer::MusicPlayer(QWidget *parent) :
        QWidget(parent), ui(new Ui::MusicPlayer) ,lrcmodel(new QStandardItemModel(this)),musicmodel(new QStandardItemModel(this)),player(new QMediaPlayer(this)),audioOutput(new QAudioOutput(this)) {
        ui->setupUi(this);

        // 设置窗口图标
        this->setWindowIcon(QIcon(":/images/icon.png"));

        // 设置窗口最小尺寸，确保界面元素不会过小
        this->setMinimumSize(640, 480);

        // 使窗口可以调整大小
        this->setWindowFlags(Qt::Window);

        ui->voice->hide();
        ui->bofangtiao->setMaximum(0);
        ui->lrclist->setModel(lrcmodel);
        ui->musiclist->setModel(musicmodel);

        // 初始化歌词模型
        lrcmodel->setHorizontalHeaderLabels({"歌词"});
        currentLyricIndex = -1;
        lyricTimer = new QTimer(this);
        // 添加默认的歌词提示
        auto defaultItem = new QStandardItem("请选择歌曲播放");
        lrcmodel->appendRow(defaultItem);

        player->setAudioOutput(audioOutput);
        ui->voice->setMaximum(100);
        connect(player,&QMediaPlayer::durationChanged, [this](qint64 duration) {
            if (duration % 60000 / 1000 < 10) ui->full->setText(QString::number(duration / 60000) + ":0" + QString::number((duration % 60000) / 1000));
            else ui->full->setText(QString::number(duration / 60000) + ":" + QString::number((duration % 60000) / 1000));
            ui->bofangtiao->setMaximum(static_cast<int>(duration));
        });
        connect(player,&QMediaPlayer::positionChanged, [this](qint64 position) {
            if (position % 60000 / 1000 < 10) ui->now->setText(QString::number(position / 60000) + ":0" + QString::number((position % 60000) / 1000));
            else ui->now->setText(QString::number(position / 60000) + ":" + QString::number((position % 60000) / 1000));
            if (!isSliderBeingDragged)
            {
                ui->bofangtiao->setValue(static_cast<int>(position));
            }
        });// 在构造函数的最后添加
        connect(lyricTimer, &QTimer::timeout, this, &MusicPlayer::updateCurrentLyric);
        lyricTimer->start(100); // 每100毫秒更新一次歌词
    }

    MusicPlayer::~MusicPlayer() {
        delete ui;
    }

    void MusicPlayer::DirBtn_clicked() {
        isSliderBeingDragged = false;
        auto dir = QFileDialog::getExistingDirectory(this, "选择文件夹", "D:\\Local Music");
        if (dir.isEmpty()) {
            return;
        }

        QDirIterator it(dir, {"*.flac", "*.mp3"}, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            auto info = it.nextFileInfo();
            QString filePath = info.absoluteFilePath();
            auto item = new QStandardItem(info.completeBaseName());
            item->setData(filePath, Qt::UserRole + 1);  // 存储完整文件路径
            musicmodel->appendRow(item);
        }
    }

    void MusicPlayer::musicName_clicked(const QModelIndex &index) {
        isSliderBeingDragged = false; // 确保在点击歌曲时不再拖动进度条
        auto filePath = index.data(Qt::UserRole + 1).toString();

        // 加载歌词
        loadLyricsFromMetadata(filePath);

        player->setSource(QUrl::fromLocalFile(filePath));
        ui->songName->setText(index.data(Qt::DisplayRole).toString());
        player->play();
    }

    void MusicPlayer::PauseBtn_clicked() {
        isSliderBeingDragged=false;
        player->pause();
    }

    void MusicPlayer::StartBtn_clicked() {
        isSliderBeingDragged= false; // 确保在点击开始按钮时不再拖动进度条
        player->play();
    }

    void MusicPlayer::PrewBtn_clicked() {
        isSliderBeingDragged= false;
        auto currentIndex = ui->musiclist->currentIndex();
        if (!currentIndex.isValid() || currentIndex.row() == 0) {
            return; // 没有上一首歌
        }
        auto previousIndex = ui->musiclist->model()->index(currentIndex.row() - 1, 0);
        ui->musiclist->setCurrentIndex(previousIndex);
        musicName_clicked(previousIndex);
    }

    void MusicPlayer::NextBtn_clicked() {
        isSliderBeingDragged= false;
        auto currentIndex = ui->musiclist->currentIndex();
        if (!currentIndex.isValid()) {
            return;
        }
        auto nextIndex = ui->musiclist->model()->index(currentIndex.row() + 1, 0);
        if (nextIndex.isValid()) {
            ui->musiclist->setCurrentIndex(nextIndex);
            musicName_clicked(nextIndex);
        } else {
            // 如果没有下一首歌，可以选择循环播放或其他逻辑
            ui->musiclist->setCurrentIndex(ui->musiclist->model()->index(0, 0));
            musicName_clicked(ui->musiclist->model()->index(0, 0));
        }
    }

    void MusicPlayer::VoiceBtn_clicked() {
        ui->voice->setVisible(!ui->voice->isVisible());
    }

    void MusicPlayer::Audio_release() {
        player->play(); // 确保在拖动后继续播放
        isSliderBeingDragged = false; // 用户停止拖动
    }

    void MusicPlayer::Audio_pressed() {
        player->pause();
        isSliderBeingDragged = true; // 用户开始拖动
    }

    void MusicPlayer::Audio_valueChanged(int value) {
        // 当用户拖动进度条时，实时更新时间显示
        if (isSliderBeingDragged) {
            auto position = static_cast<qint64>(value);
            if (position % 60000 / 1000 < 10) {
                ui->now->setText(QString::number(position / 60000) + ":0" + QString::number((position % 60000) / 1000));
            } else {
                ui->now->setText(QString::number(position / 60000) + ":" + QString::number((position % 60000) / 1000));
            }
            player->setPosition(position); // 设置播放器位置
        }
    }

    void MusicPlayer::Voice_valueChanged(int value) {
        // 音量调节
        if (audioOutput) {
            float volume = static_cast<float>(value) / 100.0f; // 修复类型转换
            audioOutput->setVolume(volume);
            ui->voice->setValue(value); // 更新音量滑块的值
        }
    }

    // 从音频文件元数据中提取歌词
    QString MusicPlayer::extractLyricsFromFile(const QString& filePath) {
        TagLib::FileRef file(filePath.toLocal8Bit().constData());

        if (file.isNull()) {
            return QString();
        }

        // 尝试从 MP3 文件中提取歌词
        if (auto mp3File = dynamic_cast<TagLib::MPEG::File*>(file.file())) {
            if (auto id3v2Tag = mp3File->ID3v2Tag()) {
                // 查找 USLT 帧（未同步歌词）
                auto frameList = id3v2Tag->frameList("USLT");
                if (!frameList.isEmpty()) {
                    if (auto lyricsFrame = dynamic_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame*>(frameList.front())) {
                        return QString::fromStdString(lyricsFrame->text().to8Bit(true));
                    }
                }
            }
        }

        // 对于 FLAC 文件，从 Vorbis 注释中获取歌词
        if (auto flacFile = dynamic_cast<TagLib::FLAC::File*>(file.file())) {
            auto properties = flacFile->properties();
            if (properties.contains("LYRICS")) {
                return QString::fromStdString(properties["LYRICS"].front().to8Bit(true));
            }
            if (properties.contains("UNSYNCEDLYRICS")) {
                return QString::fromStdString(properties["UNSYNCEDLYRICS"].front().to8Bit(true));
            }
        }

        return QString();
    }

    // 加载歌词（优先从元数据，其次从LRC文件）
    void MusicPlayer::loadLyricsFromMetadata(const QString& filePath) {
        lyrics.clear();
        lrcmodel->clear();
        currentLyricIndex = -1;

        // 首先尝试从音频文件元数据中读取歌词
        QString embeddedLyrics = extractLyricsFromFile(filePath);

        if (!embeddedLyrics.isEmpty()) {
            // 如果是 LRC 格式的嵌入歌词
            if (embeddedLyrics.contains(QRegularExpression(R"(\[\d{2}:\d{2}\.\d{2}\])"))) {
                parseLrcContent(embeddedLyrics);
            } else {
                // 普通文本歌词
                auto item = new QStandardItem(embeddedLyrics);
                lrcmodel->appendRow(item);
            }
            return;
        }

        // 如果元数据中没有歌词，尝试加载 LRC 文件
        QString lrcPath = findLrcFile(filePath);
        if (!lrcPath.isEmpty()) {
            parseLrcFile(lrcPath);
        } else {
            auto defaultItem = new QStandardItem("未找到歌词");
            lrcmodel->appendRow(defaultItem);
        }
    }
    void MusicPlayer::parseLrcFile(const QString& lrcFilePath) {
        lyrics.clear();
        lrcmodel->clear();

        QFile file(lrcFilePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            auto defaultItem = new QStandardItem("无法打开歌词文件");
            lrcmodel->appendRow(defaultItem);
            return;
        }

        QTextStream in(&file);
        in.setEncoding(QStringConverter::Utf8);

        QRegularExpression timeRegex(R"(\[(\d{2}):(\d{2})\.(\d{2})\])");

        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty()) continue;

            // 提取歌词文本（移除时间标签）
            QString text = line;
            QRegularExpressionMatchIterator matches = timeRegex.globalMatch(line);
            while (matches.hasNext()) {
                QRegularExpressionMatch match = matches.next();
                text = text.remove(match.captured(0));
            }
            text = text.trimmed();

            // 提取所有时间戳
            matches = timeRegex.globalMatch(line);
            while (matches.hasNext()) {
                QRegularExpressionMatch match = matches.next();
                int minutes = match.captured(1).toInt();
                int seconds = match.captured(2).toInt();
                int centiseconds = match.captured(3).toInt();

                qint64 timestamp = (minutes * 60 + seconds) * 1000 + centiseconds * 10;

                LyricLine lyric{timestamp, text};
                lyrics.append(lyric);
            }
        }

        // 按时间戳排序
        std::sort(lyrics.begin(), lyrics.end(), [](const LyricLine& a, const LyricLine& b) {
            return a.timestamp < b.timestamp;
        });

        // 添加到模型中显示
        for (const auto& lyric : lyrics) {
            auto item = new QStandardItem(lyric.text);
            lrcmodel->appendRow(item);
        }

        file.close();
    }
    void MusicPlayer::parseLrcContent(const QString& lrcContent) {
        lyrics.clear();
        lrcmodel->clear();

        QTextStream stream(const_cast<QString*>(&lrcContent), QIODevice::ReadOnly);
        QRegularExpression timeRegex(R"(\[(\d{2}):(\d{2})\.(\d{2})\])");

        while (!stream.atEnd()) {
            QString line = stream.readLine().trimmed();
            if (line.isEmpty()) continue;

            // 提取歌词文本（移除时间标签）
            QString text = line;
            QRegularExpressionMatchIterator matches = timeRegex.globalMatch(line);
            while (matches.hasNext()) {
                QRegularExpressionMatch match = matches.next();
                text = text.remove(match.captured(0));
            }
            text = text.trimmed();

            // 提取所有时间戳
            matches = timeRegex.globalMatch(line);
            while (matches.hasNext()) {
                QRegularExpressionMatch match = matches.next();
                int minutes = match.captured(1).toInt();
                int seconds = match.captured(2).toInt();
                int centiseconds = match.captured(3).toInt();

                qint64 timestamp = (minutes * 60 + seconds) * 1000 + centiseconds * 10;

                LyricLine lyric{timestamp, text};
                lyrics.append(lyric);
            }
        }

        // 按时间戳排序
        std::sort(lyrics.begin(), lyrics.end(), [](const LyricLine& a, const LyricLine& b) {
            return a.timestamp < b.timestamp;
        });

        // 添加到模型中显示
        for (const auto& lyric : lyrics) {
            auto item = new QStandardItem(lyric.text);
            lrcmodel->appendRow(item);
        }
    }
    QString MusicPlayer::findLrcFile(const QString& musicFilePath) {
        QFileInfo musicInfo(musicFilePath);
        QString baseName = musicInfo.completeBaseName();
        QString dirPath = musicInfo.absolutePath();

        // 可能的 LRC 文件路径
        QStringList possiblePaths = {
            dirPath + "/" + baseName + ".lrc",
            dirPath + "/" + baseName + ".LRC",
            dirPath + "/lyrics/" + baseName + ".lrc",
            dirPath + "/Lyrics/" + baseName + ".lrc"
        };

        // 检查每个可能的路径
        for (const QString& path : possiblePaths) {
            if (QFile::exists(path)) {
                return path;
            }
        }

        return QString(); // 未找到
    }
    void MusicPlayer::updateCurrentLyric() {
        if (lyrics.isEmpty()) return;

        qint64 currentPosition = player->position();
        int newIndex1 = -1;
        int newIndex = -1;

        // 找到当前播放位置对应的歌词
        for (int i = 0; i < lyrics.size(); ++i) {
            if (currentPosition >= lyrics[i].timestamp) {

                if (lyrics[i].timestamp == lyrics[newIndex].timestamp) newIndex1 = i;
                else {newIndex = i;newIndex1=-1;}

            } else {
                break;
            }
        }

        // 如果歌词索引发生变化，更新高亮
        if ((newIndex != currentLyricIndex && newIndex >= 0) || (newIndex1 != currentIndex && newIndex1 >= 0)) {
            currentLyricIndex = newIndex;
            currentIndex = newIndex1;
            // 清除之前的高亮
            for (int i = 0; i < lrcmodel->rowCount(); ++i) {
                auto item = lrcmodel->item(i);
                if (item) {
                    QFont font = item->font();
                    font.setBold(false);
                    item->setFont(font);
                    item->setForeground(QBrush(Qt::black));
                }
            }

            // 设置当前歌词高亮
            if (currentLyricIndex < lrcmodel->rowCount()) {
                auto item = lrcmodel->item(currentLyricIndex);
                if (item) {
                    QFont font = item->font();
                    font.setBold(true);
                    item->setFont(font);
                    item->setForeground(QBrush(Qt::red));
                }
                if (currentIndex < lrcmodel->rowCount()) {
                    auto item = lrcmodel->item(currentIndex);
                    if (item) {
                        QFont font = item->font();
                        font.setBold(true);
                        item->setFont(font);
                        item->setForeground(QBrush(Qt::red));
                    }

                    // 滚动到当前歌词（确保歌词在视图中央）
                    QModelIndex index = lrcmodel->index(currentLyricIndex, 0);
                    ui->lrclist->scrollTo(index, QAbstractItemView::PositionAtCenter);


                }
            }
        }
    }
}

