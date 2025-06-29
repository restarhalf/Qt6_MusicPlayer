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
#include <QRegularExpression>
#include <QTextStream>
#include <QFile>
#include <QAbstractItemView>
#include <QDebug>
#include <QSettings>
#include <algorithm>
#include <random>
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
        currentIndex = -1;
        lyricTimer = new QTimer(this);
        uiUpdateTimer = new QTimer(this);
        playbackTimer = new QTimer(this);

        // 添加默认的歌词提示
        auto defaultItem = new QStandardItem("请选择歌曲播放");
        lrcmodel->appendRow(defaultItem);

        player->setAudioOutput(audioOutput);
        ui->voice->setMaximum(100);

        // 连接播放器信号
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
        });

        // 连接播放完成信号
        connect(player, &QMediaPlayer::mediaStatusChanged, [this](QMediaPlayer::MediaStatus status) {
            if (status == QMediaPlayer::EndOfMedia) {
                handlePlaybackFinished();
            }
        });

        // 连接播放状态变化信号
        connect(player, &QMediaPlayer::playbackStateChanged, [this](QMediaPlayer::PlaybackState state) {
            updatePlaybackState();
        });

        // 启动定时器
        connect(lyricTimer, &QTimer::timeout, this, &MusicPlayer::updateCurrentLyric);
        connect(uiUpdateTimer, &QTimer::timeout, this, &MusicPlayer::updatePlaybackState);
        connect(playbackTimer, &QTimer::timeout, [this]() {
            // 定期检查播放状态并更新UI
            updateVolumeDisplay();
        });

        lyricTimer->start(100); // 每100毫秒更新一次歌词
        uiUpdateTimer->start(500); // 每500毫秒更新一次UI状态
        playbackTimer->start(1000); // 每秒检查一次播放状态

        // 加���播放器状态
        loadPlayerState();
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

        // 更新播放器信息
        updatePlayerInfo(filePath);

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

    // 加载歌词（优先从元数据，其次从LRC文件���
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

            // 提取歌词文本（移除时间标签��
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
        // 如果没有播放器或歌词为空，直接返回
        if (!player || lyrics.isEmpty()) {
            return;
        }

        // 获取当前播放位置
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
                    item->setForeground(QBrush(Qt::white));
                    item->setBackground(QBrush(Qt::blue));
                }
                if (currentIndex < lrcmodel->rowCount()) {
                    auto item = lrcmodel->item(currentIndex);
                    if (item) {
                        QFont font = item->font();
                        font.setBold(true);
                        item->setFont(font);
                        item->setForeground(QBrush(Qt::white));
                        item->setBackground(QBrush(Qt::blue));
                    }

                    // 滚动到当前歌词（确保歌词在视图中央）
                    QModelIndex index = lrcmodel->index(currentLyricIndex, 0);
                    ui->lrclist->scrollTo(index, QAbstractItemView::PositionAtCenter);

                }
            }
        }
    }

    // 更新播放状态
    void MusicPlayer::updatePlaybackState() {
        if (!player) return;

        QMediaPlayer::PlaybackState state = player->playbackState();

        // 根据播放状态更新按钮文本和状态
        switch (state) {
            case QMediaPlayer::PlayingState:
                // 可以在这里更新播放按钮图标或文本
                break;
            case QMediaPlayer::PausedState:
                // 可以在这里更新暂停按钮图标或文本
                break;
            case QMediaPlayer::StoppedState:
                // 重置UI状态
                ui->bofangtiao->setValue(0);
                ui->now->setText("0:00");
                currentLyricIndex = -1;
                break;
        }
    }

    // 更新播放器信息
    void MusicPlayer::updatePlayerInfo(const QString& filePath) {
        if (filePath.isEmpty()) return;

        QFileInfo fileInfo(filePath);
        QString fileName = fileInfo.completeBaseName();

        // 更新当前播放歌曲索引
        for (int i = 0; i < musicmodel->rowCount(); ++i) {
            auto item = musicmodel->item(i);
            if (item && item->data(Qt::UserRole + 1).toString() == filePath) {
                currentIndex = i;
                ui->musiclist->setCurrentIndex(musicmodel->index(i, 0));
                break;
            }
        }

        // 添加到播放历史
        if (currentIndex >= 0 && (playHistory.isEmpty() || playHistory.last() != currentIndex)) {
            playHistory.append(currentIndex);
            historyIndex = playHistory.size() - 1;

            // 限制历史记录长度
            if (playHistory.size() > 50) {
                playHistory.removeFirst();
                historyIndex--;
            }
        }

        // 保存播放状态
        savePlayerState();
    }

    // 处理播放完成
    void MusicPlayer::handlePlaybackFinished() {
        switch (currentPlaybackMode) {
            case Sequential:
                NextBtn_clicked(); // 播放下一首
                break;
            case Loop:
                player->setPosition(0); // 重新开始播放当前歌曲
                player->play();
                break;
            case LoopAll:
                NextBtn_clicked(); // 播放下一首，NextBtn_clicked已经处理了循环逻辑
                break;
            case Random:
                {
                    if (musicmodel->rowCount() > 0) {
                        int randomIndex = rand() % musicmodel->rowCount();
                        auto randomModelIndex = musicmodel->index(randomIndex, 0);
                        ui->musiclist->setCurrentIndex(randomModelIndex);
                        musicName_clicked(randomModelIndex);
                    }
                }
                break;
        }
    }

    // 更新播放模式
    void MusicPlayer::updatePlaybackMode(PlaybackMode mode) {
        currentPlaybackMode = mode;

        // 可以在这里更���UI显示当前播放模式
        QString modeText;
        switch (mode) {
            case Sequential:
                modeText = "顺序播放";
                break;
            case Loop:
                modeText = "单曲循环";
                break;
            case LoopAll:
                modeText = "列表循环";
                break;
            case Random:
                modeText = "随机播放";
                break;
        }

        // 如果UI中有显示播放模式的标签，可以在这里更新
        // ui->playModeLabel->setText(modeText);

        savePlayerState();
    }

    // 刷新音乐列表
    void MusicPlayer::refreshMusicList() {
        // 保存当前选中的项目
        QString currentFilePath;
        auto currentIdx = ui->musiclist->currentIndex();
        if (currentIdx.isValid()) {
            currentFilePath = currentIdx.data(Qt::UserRole + 1).toString();
        }

        // 重新扫描音乐文件夹（如果需要的话）
        // 这里可以添加自动刷新逻辑

        // 恢复选中状态
        if (!currentFilePath.isEmpty()) {
            for (int i = 0; i < musicmodel->rowCount(); ++i) {
                auto item = musicmodel->item(i);
                if (item && item->data(Qt::UserRole + 1).toString() == currentFilePath) {
                    ui->musiclist->setCurrentIndex(musicmodel->index(i, 0));
                    break;
                }
            }
        }
    }

    // 更新音量显示
    void MusicPlayer::updateVolumeDisplay() {
        if (audioOutput) {
            int volume = static_cast<int>(audioOutput->volume() * 100);
            if (ui->voice->value() != volume) {
                ui->voice->setValue(volume);
            }
        }
    }

    // 保存播放器��态
    void MusicPlayer::savePlayerState() {
        // 这里可以保存到配置文件或注册表
        // 包括：当前播放歌曲、播放位置、音量、播放模式等

        // 示例：保存到QSettings
        /*
        QSettings settings;
        settings.setValue("player/currentIndex", currentIndex);
        settings.setValue("player/volume", audioOutput ? audioOutput->volume() : 0.5);
        settings.setValue("player/playbackMode", static_cast<int>(currentPlaybackMode));
        if (player) {
            settings.setValue("player/position", player->position());
        }
        */
    }

    // 加载播放器状态
    void MusicPlayer::loadPlayerState() {
        // 从配置文件或注册表加载播放器状态

        // 示例：从QSettings加载
        /*
        QSettings settings;
        currentIndex = settings.value("player/currentIndex", -1).toInt();
        float volume = settings.value("player/volume", 0.5).toFloat();
        currentPlaybackMode = static_cast<PlaybackMode>(
            settings.value("player/playbackMode", Sequential).toInt());

        if (audioOutput) {
            audioOutput->setVolume(volume);
            ui->voice->setValue(static_cast<int>(volume * 100));
        }
        */
    }
}
