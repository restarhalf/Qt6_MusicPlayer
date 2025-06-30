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
        QWidget(parent), ui(nullptr) {

        // 添加对象有效性标志
        m_isValid = false;

        // 使用try-catch包围UI初始化，防止初始化失败
        try {
            ui = new Ui::MusicPlayer;
            ui->setupUi(this);
        } catch (const std::exception& e) {
            qCritical() << "UI initialization failed:" << e.what();
            return;
        }

        // 初始化所有指针成员为nullptr - 防止空指针访问
        lrcmodel = nullptr;
        musicmodel = nullptr;
        player = nullptr;
        audioOutput = nullptr;
        lyricTimer = nullptr;
        uiUpdateTimer = nullptr;
        playbackTimer = nullptr;
        fadeAnimation = nullptr;
        opacityEffect = nullptr;
        scrollBar = nullptr;

        // 严格检查UI组件是否存在
        if (!ui) {
            qCritical() << "UI initialization failed!";
            return;
        }

        // 逐步初始化所有组件
        try {
            lrcmodel = new QStandardItemModel(this);
            musicmodel = new QStandardItemModel(this);
            player = new QMediaPlayer(this);
            audioOutput = new QAudioOutput(this);

            lyricTimer = new QTimer(this);
            uiUpdateTimer = new QTimer(this);
            playbackTimer = new QTimer(this);
        } catch (const std::exception& e) {
            qCritical() << "Component initialization failed:" << e.what();
            return;
        }

        // 设置窗口属性
        this->setWindowIcon(QIcon(":/images/icon.png"));
        this->setMinimumSize(640, 480);
        this->setWindowFlags(Qt::Window);

        // 逐个检查并设置UI组件
        if (ui->voice) {
            ui->voice->hide();
            ui->voice->setMaximum(100);
            ui->voice->setMinimum(0);
            ui->voice->setValue(50); // 设置默认音量
        }

        if (ui->bofangtiao) {
            ui->bofangtiao->setMaximum(0);
            ui->bofangtiao->setMinimum(0);
            ui->bofangtiao->setValue(0);
        }

        if (ui->lrclist && lrcmodel) {
            ui->lrclist->setModel(lrcmodel);
            // 设置选择模式，防止多选导致的问题
            ui->lrclist->setSelectionMode(QAbstractItemView::SingleSelection);
            // 禁用拖拽功能
            ui->lrclist->setDragEnabled(false);
            ui->lrclist->setAcceptDrops(false);
            ui->lrclist->setDragDropMode(QAbstractItemView::NoDragDrop);
            // 禁用拖拽复制
            ui->lrclist->setDefaultDropAction(Qt::IgnoreAction);
        }

        if (ui->musiclist && musicmodel) {
            ui->musiclist->setModel(musicmodel);
            ui->musiclist->setSelectionMode(QAbstractItemView::SingleSelection);
        }

        // 初始化歌词模型
        if (lrcmodel) {
            lrcmodel->setHorizontalHeaderLabels({"歌词"});
            currentLyricIndex = -1;
            currentIndex = -1;

            // 添加默认的歌词提示
            auto defaultItem = new QStandardItem("请选择歌曲播放");
            if (defaultItem) {
                lrcmodel->appendRow(defaultItem);
            }
        }

        // 确保播放器和音频输出正确连接
        if (player && audioOutput) {
            player->setAudioOutput(audioOutput);
            // 设置默认音量
            audioOutput->setVolume(0.5f);
        }

        // 使用Qt5兼容的信号连接方式 - 更稳定
        if (player) {
            connect(player, SIGNAL(durationChanged(qint64)), this, SLOT(onDurationChanged(qint64)));
            connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(onPositionChanged(qint64)));
            connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(onMediaStatusChanged(QMediaPlayer::MediaStatus)));
            connect(player, SIGNAL(playbackStateChanged(QMediaPlayer::PlaybackState)), this, SLOT(onPlaybackStateChanged(QMediaPlayer::PlaybackState)));
        }

        // 延迟启动定时器，确保所有组件都已初始化
        QTimer::singleShot(100, [this]() {
            if (!m_isValid) return;

            if (lyricTimer) {
                connect(lyricTimer, SIGNAL(timeout()), this, SLOT(updateCurrentLyric()));
                lyricTimer->start(100);
            }

            if (uiUpdateTimer) {
                connect(uiUpdateTimer, SIGNAL(timeout()), this, SLOT(updatePlaybackState()));
                uiUpdateTimer->start(500);
            }

            if (playbackTimer) {
                connect(playbackTimer, SIGNAL(timeout()), this, SLOT(updateVolumeDisplay()));
                playbackTimer->start(1000);
            }
        });

        // 设置动画
        setupAnimations();

        // 加载播放器状态
        loadPlayerState();

        // 最后标记对象为有效
        m_isValid = true;
    }

    MusicPlayer::~MusicPlayer() {
        // 立即标记对象为无效，防止任何回调访问
        m_isValid = false;

        // 停���所有定时器，防止在对象销毁时继续访问UI
        if (lyricTimer) {
            lyricTimer->stop();
            lyricTimer->deleteLater();
            lyricTimer = nullptr;
        }

        if (uiUpdateTimer) {
            uiUpdateTimer->stop();
            uiUpdateTimer->deleteLater();
            uiUpdateTimer = nullptr;
        }

        if (playbackTimer) {
            playbackTimer->stop();
            playbackTimer->deleteLater();
            playbackTimer = nullptr;
        }

        // 断开所有信号连接，防止在销���过程中触发回调
        if (player) {
            disconnect(player, nullptr, this, nullptr);
            player->stop();
            player->setSource(QUrl());
        }

        if (lyricTimer) {
            disconnect(lyricTimer, nullptr, this, nullptr);
        }

        if (uiUpdateTimer) {
            disconnect(uiUpdateTimer, nullptr, this, nullptr);
        }

        if (playbackTimer) {
            disconnect(playbackTimer, nullptr, this, nullptr);
        }

        // 停止动画
        if (fadeAnimation) {
            fadeAnimation->stop();
            fadeAnimation->deleteLater();
            fadeAnimation = nullptr;
        }

        // 保存播放器状态
        savePlayerState();

        // 最后删除UI
        delete ui;
        ui = nullptr;
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
            auto item = new QStandardItem(getMusicTitle(filePath));
            item->setData(filePath, Qt::UserRole + 1);  // 存储��整文件路径
            musicmodel->appendRow(item);
        }
    }

    void MusicPlayer::musicName_clicked(const QModelIndex &index) {
        if (!player || !audioOutput || !index.isValid()) {
            return; // 安全检查：确保播放器和索引有效
        }

        isSliderBeingDragged = false; // 确保在点击歌��时不再拖动进度条
        auto filePath = index.data(Qt::UserRole + 1).toString();

        if (filePath.isEmpty()) {
            return; // 安全检查：确保文件路����有效
        }

        // 加载歌词
        loadLyricsFromMetadata(filePath);

        // 更新播放器信息
        updatePlayerInfo(filePath);

        player->setSource(QUrl::fromLocalFile(filePath));
        if (ui && ui->songName) {
            ui->songName->setText(index.data(Qt::DisplayRole).toString());
        }
        player->play();
    }

    void MusicPlayer::PauseBtn_clicked() {
        if (!player) return; // 安全检查
        isSliderBeingDragged = false;
        player->pause();
    }

    void MusicPlayer::StartBtn_clicked() {
        if (!player) return; // 安全检查
        isSliderBeingDragged = false; // 确保在点击开始按钮时不再拖动进度条
        player->play();
    }

    void MusicPlayer::PrewBtn_clicked() {
        // 强化安全检查
        if (!m_isValid || !ui || !ui->musiclist || !musicmodel) return;

        isSliderBeingDragged = false;
        auto currentIndex = ui->musiclist->currentIndex();
        if (!currentIndex.isValid() || currentIndex.row() == 0) {
            return; // 没有上一首歌
        }
        auto previousIndex = ui->musiclist->model()->index(currentIndex.row() - 1, 0);
        if (previousIndex.isValid()) {
            ui->musiclist->setCurrentIndex(previousIndex);
            musicName_clicked(previousIndex);
        }
    }

    void MusicPlayer::NextBtn_clicked() {
        // 强化安全检查
        if (!m_isValid || !ui || !ui->musiclist || !musicmodel) return;

        isSliderBeingDragged = false;
        auto currentIndex = ui->musiclist->currentIndex();
        if (!currentIndex.isValid()) {
            return;
        }
        auto nextIndex = ui->musiclist->model()->index(currentIndex.row() + 1, 0);
        if (nextIndex.isValid()) {
            ui->musiclist->setCurrentIndex(nextIndex);
            musicName_clicked(nextIndex);
        } else {
            // 如果没有下一首歌，检查是否有歌曲可以循环
            if (musicmodel->rowCount() > 0) {
                ui->musiclist->setCurrentIndex(ui->musiclist->model()->index(0, 0));
                musicName_clicked(ui->musiclist->model()->index(0, 0));
            }
        }
    }

    void MusicPlayer::VoiceBtn_clicked() {
        // 强化安全检查
        if (!m_isValid || !ui || !ui->voice) return;

        ui->voice->setVisible(!ui->voice->isVisible());
    }

    void MusicPlayer::Audio_release() {
        // 强化安全检查
        if (!m_isValid || !player) return;

        player->play(); // 确保在拖动后继续播放
        isSliderBeingDragged = false; // 用户停止拖动
    }

    void MusicPlayer::Audio_pressed() {
        // 强化安全检查
        if (!m_isValid || !player) return;

        player->pause();
        isSliderBeingDragged = true; // 用户开始拖动
    }

    void MusicPlayer::Audio_valueChanged(int value) {
        // 当用户���动进度条时，实时更新时间显示
        if (isSliderBeingDragged && ui && ui->now && player) {
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
        if (audioOutput && ui && ui->voice) {
            float volume = static_cast<float>(value) / 100.0f; // 修复类型转换
            audioOutput->setVolume(volume);
            ui->voice->setValue(value); // 更新音量滑块的值
        }
    }
    QString MusicPlayer::getMusicTitle(const QString& filePath) {
        TagLib::FileRef file(filePath.toLocal8Bit().constData());
        if (file.isNull()) {
            return QString();
        }

        // 尝试从 MP3 文件中获取标题
        if (auto mp3File = dynamic_cast<TagLib::MPEG::File*>(file.file())) {
            if (auto id3v2Tag = mp3File->ID3v2Tag()) {
                return QString::fromStdString(id3v2Tag->title().to8Bit(true));
            }
        }

        // 对于 FLAC 文件，从 Vorbis 注释中获取标题
        if (auto flacFile = dynamic_cast<TagLib::FLAC::File*>(file.file())) {
            auto properties = flacFile->properties();
            if (properties.contains("TITLE")) {
                return QString::fromStdString(properties["TITLE"].front().to8Bit(true));
            }
        }

        return QString();
    }

    // 静态正则表达式，避免重复创建临时对象
    static QRegularExpression timeRegex(R"(\[\d{2}:\d{2}\.\d{2}\])");
    static QRegularExpression lrcFormatRegex(R"(\[\d{2}:\d{2}\.\d{2}\])");

    // 从音频文件元数据中提取歌词
    QString MusicPlayer::extractLyricsFromFile(const QString& filePath) {
        TagLib::FileRef file(filePath.toLocal8Bit().constData());

        if (file.isNull()) {
            return QString();
        }

        // 尝试从 MP3 文件��提取歌词
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
            // 如果是 LRC 格式的嵌入歌�� - 使用���态正则表达式
            if (embeddedLyrics.contains(lrcFormatRegex)) {
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
            auto defaultItem = new QStandardItem("无法��开歌词文件");
            defaultItem->setTextAlignment(Qt::AlignCenter);  // 居��对齐
            lrcmodel->appendRow(defaultItem);
            return;
        }

        QTextStream in(&file);
        in.setEncoding(QStringConverter::Utf8);

        QRegularExpression timeRegex(R"(\[(\d{2}):(\d{2})\.(\d{2})\])");

        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
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

        // 添加到模型中显示，设置居中对齐
        for (const auto& lyric : lyrics) {
            auto item = new QStandardItem(lyric.text);
            item->setTextAlignment(Qt::AlignCenter);  // 设置文本居中对齐
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

        // ��加到模型中显示，设置居中对齐
        for (const auto& lyric : lyrics) {
            auto item = new QStandardItem(lyric.text);
            item->setTextAlignment(Qt::AlignCenter);  // 设置文本居中对齐
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
        // 强化安全检查 - 检查对象有效性
        if (!m_isValid || !player || lyrics.isEmpty() || !lrcmodel || !ui) {
            return;
        }

        // 获取当前播放位置
        qint64 currentPosition = player->position();
        int newIndex1 = -1;
        int newIndex = -1;

        // 找到当前播放位置对应的歌词 - 修复数组越界问题
        for (int i = 0; i < lyrics.size(); ++i) {
            if (currentPosition >= lyrics[i].timestamp) {
                // 安全检查：确保newIndex有效才进行比较
                if (newIndex >= 0 && newIndex < lyrics.size() &&
                    lyrics[i].timestamp == lyrics[newIndex].timestamp) {
                    newIndex1 = i;
                } else {
                    newIndex = i;
                    newIndex1 = -1;
                }
            } else {
                break;
            }
        }

        // 如果歌词索引发生变化，更新高亮
        if ((newIndex != currentLyricIndex && newIndex >= 0) ||
            (newIndex1 != currentIndex && newIndex1 >= 0)) {
            currentLyricIndex = newIndex;
            currentIndex = newIndex1;

            // 应用新的歌词高亮样式
            updateLyricHighlight();

            // 平滑滚动到当前歌词
            smoothScrollToLyric(currentLyricIndex);
        }
    }

    // ��增：更新歌词高亮的方法
    void MusicPlayer::updateLyricHighlight() {
        // 强化安全检查 - 检查对象有效性
        if (!m_isValid || !lrcmodel || !ui || !ui->lrclist) {
            return;
        }

        // 清除所有歌词的样式
        for (int i = 0; i < lrcmodel->rowCount(); ++i) {
            auto item = lrcmodel->item(i);
            if (!item) continue; // 跳过空项

            // 计算距离当前歌词的��离，用于设置透明度
            int distance = abs(i - currentLyricIndex);

            QFont font = item->font();
            font.setBold(false);
            font.setPointSize(12);

            // 根据距离设置透明度和颜色
            QColor textColor;
            if (i == currentLyricIndex) {
                // 当前歌词：亮白色，粗体，较大字体
                font.setBold(true);
                font.setPointSize(14);
                textColor = QColor(255, 255, 255, 255);

                // 使用try-catch保护背景设置
                try {
                    item->setBackground(QBrush(QColor(74, 144, 226, 100))); // 半透明蓝色背景
                } catch (...) {
                    qWarning() << "Failed to set background for current lyric item";
                }
            } else if (distance == 1) {
                // 相邻歌词：中等透明度
                textColor = QColor(255, 255, 255, 180);
                try {
                    item->setBackground(QBrush(Qt::transparent));
                } catch (...) {
                    // 忽略背景设置错误
                }
            } else if (distance <= 3) {
                // 较近歌词：低透明度
                textColor = QColor(255, 255, 255, 120);
                try {
                    item->setBackground(QBrush(Qt::transparent));
                } catch (...) {
                    // 忽略背景设置错误
                }
            } else {
                // 远处歌词：很低透明度
                textColor = QColor(255, 255, 255, 80);
                try {
                    item->setBackground(QBrush(Qt::transparent));
                } catch (...) {
                    // 忽略背景设置错误
                }
            }

            // 安全设置字体和前景色
            try {
                item->setFont(font);
                item->setForeground(QBrush(textColor));
            } catch (...) {
                qWarning() << "Failed to set font or foreground for lyric item";
            }
        }

        // 处理双歌词情况（添加更严格的边界检查）
        if (currentIndex >= 0 && currentIndex < lrcmodel->rowCount()) {
            auto item = lrcmodel->item(currentIndex);
            if (item) {
                try {
                    QFont font = item->font();
                    font.setBold(true);
                    font.setPointSize(14);
                    item->setFont(font);
                    item->setForeground(QBrush(QColor(255, 255, 255, 255)));
                    item->setBackground(QBrush(QColor(74, 144, 226, 100)));
                } catch (...) {
                    qWarning() << "Failed to set properties for secondary lyric item";
                }
            }
        }
    }

    // 新增：平滑滚动到指定歌词的方法
    void MusicPlayer::smoothScrollToLyric(int lyricIndex) {
        // 强化安全检查 - 检查对象有效性和边界
        if (!m_isValid || !ui || !ui->lrclist || !lrcmodel ||
            lyricIndex < 0 || lyricIndex >= lrcmodel->rowCount()) {
            return;
        }

        // 获取��动条
        QScrollBar *scrollBar = ui->lrclist->verticalScrollBar();
        if (!scrollBar) return;

        // 停止之前的动画避免冲突
        if (fadeAnimation) {
            fadeAnimation->stop();
            fadeAnimation->deleteLater();
            fadeAnimation = nullptr;
        }

        // 计算目标位置（让当前歌词显示在视图的中央）
        QModelIndex index = lrcmodel->index(lyricIndex, 0);
        QRect itemRect = ui->lrclist->visualRect(index);

        int viewHeight = ui->lrclist->viewport()->height();
        int currentScrollValue = scrollBar->value();

        // 计算目标滚动位置，让歌词显示在中央偏上位置
        int targetScrollValue = itemRect.top() + scrollBar->value() - (viewHeight * 0.4);

        // 限制滚动范围
        targetScrollValue = qBound(scrollBar->minimum(), targetScrollValue, scrollBar->maximum());

        // ����目标位置和当前位置差距很小，就不需要动画
        if (abs(targetScrollValue - currentScrollValue) < 10) {
            ui->lrclist->scrollTo(index, QAbstractItemView::PositionAtCenter);
            QTimer::singleShot(50, [this]() {
                pulseCurrentLyric();
            });
            return;
        }

        // 创建平滑滚动动画
        fadeAnimation = new QPropertyAnimation(scrollBar, "value", this);

        // 根据距离调整���画时间，距��越远动画时间越长，但有上下限
        int distance = abs(targetScrollValue - currentScrollValue);
        int animationDuration = qBound(300, distance * 2, 1000); // 300ms到1000ms之间

        fadeAnimation->setDuration(animationDuration);
        fadeAnimation->setEasingCurve(QEasingCurve::OutCubic); // 使用更平滑的缓动曲线
        fadeAnimation->setStartValue(currentScrollValue);
        fadeAnimation->setEndValue(targetScrollValue);

        // ���接动画完成信号
        connect(fadeAnimation, &QPropertyAnimation::finished, [this]() {
            // 滚动完成后添加脉冲效果
            QTimer::singleShot(100, [this]() {
                pulseCurrentLyric();
            });
        });

        // 添加动画值变化时的回调，可以实现更复杂的效果
        connect(fadeAnimation, &QPropertyAnimation::valueChanged, [this](const QVariant &value) {
            // 可以在这里添加滚动过程中的额外效果
            Q_UNUSED(value)
        });

        fadeAnimation->start();
    }

    // 新增：当前歌词脉冲效果
    void MusicPlayer::pulseCurrentLyric() {
        // 强化安全检查 - 检查对象有效性和边界
        if (!m_isValid || !lrcmodel ||
            currentLyricIndex < 0 || currentLyricIndex >= lrcmodel->rowCount()) {
            return;
        }

        auto item = lrcmodel->item(currentLyricIndex);
        if (!item) return;

        // 避���在动画过程中直接修改item，使用更安全的方式
        // 检查item是否仍然有效
        if (currentLyricIndex >= lrcmodel->rowCount()) {
            return; // item可能已经被删除
        }

        // 使用信号槽机制来安全地更新背景色，避免直接在lambda中捕获item指针
        QTimer *pulseTimer = new QTimer(this);
        pulseTimer->setSingleShot(true);

        // 创建一个安全的动画更新机制
        QPropertyAnimation *pulseAnimation = new QPropertyAnimation(this, "opacity", this);
        pulseAnimation->setDuration(300);
        pulseAnimation->setEasingCurve(QEasingCurve::InOutSine);
        pulseAnimation->setStartValue(0.3);
        pulseAnimation->setEndValue(1.0);

        // 使用更安全的方式更新背景色
        connect(pulseAnimation, &QPropertyAnimation::valueChanged, [this](const QVariant &value) {
            // 在每次值变化时重新验证item的有效性
            if (!m_isValid || !lrcmodel ||
                currentLyricIndex < 0 || currentLyricIndex >= lrcmodel->rowCount()) {
                return;
            }

            auto currentItem = lrcmodel->item(currentLyricIndex);
            if (!currentItem) return;

            double alpha = value.toDouble();
            QColor highlightColor = QColor(74, 144, 226, static_cast<int>(alpha * 150 + 100));

            // 使用try-catch保护关键操作
            try {
                currentItem->setBackground(QBrush(highlightColor));
            } catch (...) {
                // 如果设置背景失败，忽��错误
                qWarning() << "Failed to set background color for lyric item";
            }
        });

        // 动画完成后清理资源
        connect(pulseAnimation, &QPropertyAnimation::finished, [pulseTimer, pulseAnimation]() {
            pulseTimer->deleteLater();
            pulseAnimation->deleteLater();
        });

        pulseAnimation->start();
    }
    // 更新播放状态
    void MusicPlayer::updatePlaybackState() {
        // 强化安全检查 - 检查对象有效性
        if (!m_isValid || !player || !ui) return;

        QMediaPlayer::PlaybackState state = player->playbackState();

        // 根据播放状态更新按钮文本和状态
        switch (state) {
            case QMediaPlayer::PlayingState:
                // 可以在这里更新播放���钮图标或文本
                break;
            case QMediaPlayer::PausedState:
                // 可以在这里更��暂停按钮图标或文本
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

            // 限制历史记��长度
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
                NextBtn_clicked(); // 播放下一��，NextBtn_clicked已经处理了循环逻辑
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

    // ��新音乐列表
    void MusicPlayer::refreshMusicList() {
        // 保存当前������的项目
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
        // 强化安全检查 - 检查对象有效性
        if (!m_isValid || !audioOutput || !ui || !ui->voice) {
            return;
        }

        if (audioOutput) {
            int volume = static_cast<int>(audioOutput->volume() * 100);
            if (ui->voice->value() != volume) {
                ui->voice->setValue(volume);
            }
        }
    }

    void MusicPlayer::Lrc_clicked(const QModelIndex& index)
    {
        // 强化安全检查 - 检查对象有效性
        if (!m_isValid || !index.isValid() || !player || lyrics.isEmpty() ||
            !ui || !ui->lrclist || !lrcmodel) {
            return;
        }

        int clickedRow = index.row();

        // 边界检查
        if (clickedRow < 0 || clickedRow >= lyrics.size() || clickedRow >= lrcmodel->rowCount()) {
            return;
        }

        // 防止重复快速点击导致的问题
        static QTime lastClickTime;
        QTime currentTime = QTime::currentTime();
        if (lastClickTime.isValid() && lastClickTime.msecsTo(currentTime) < 100) {
            return; // 100毫秒内的重复点击将被忽略
        }
        lastClickTime = currentTime;

        // 停止当前动画避免冲突
        if (fadeAnimation) {
            fadeAnimation->stop();
            fadeAnimation->deleteLater();
            fadeAnimation = nullptr;
        }

        // 设置播放位置到点击的歌词时间戳
        qint64 targetTimestamp = lyrics[clickedRow].timestamp;
        if (player) {
            player->setPosition(targetTimestamp);
        }

        // 更新当前歌词索引
        currentLyricIndex = clickedRow;
        currentIndex = -1; // 重置双歌词索引

        // 处理相同时间戳的情况���添加边界检查）
        if (clickedRow > 0 && clickedRow < lyrics.size() &&
            lyrics[clickedRow].timestamp == lyrics[clickedRow - 1].timestamp) {
            currentLyricIndex = clickedRow - 1;
            currentIndex = clickedRow;
        } else if (clickedRow < lyrics.size() - 1 && clickedRow >= 0 &&
                   lyrics[clickedRow].timestamp == lyrics[clickedRow + 1].timestamp) {
            currentLyricIndex = clickedRow;
            currentIndex = clickedRow + 1;
        }

        // 安全清除选择
        if (ui && ui->lrclist) {
            ui->lrclist->clearSelection();
        }

        // 立即更新歌词高亮显示
        updateLyricHighlight();

        // 滚动到点击的歌词
        smoothScrollToLyric(currentLyricIndex);
    }

    // 保��播放器��态
    void MusicPlayer::savePlayerState() {
        // 这里可以保存到配置文件或注册表
        // 包括：当前播放歌曲、播放位置、音量、播放模式等

        // 示例：保��到QSettings
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

    // 加载播放器��态
    void MusicPlayer::loadPlayerState() {
        // 从配置文件或注册表加载播放器状态

        // ��例：从QSettings加载
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

    // 设置动画效果
    void MusicPlayer::setupAnimations() {
        // 初始化歌词列表的透明度效果
        opacityEffect = new QGraphicsOpacityEffect(this);
        opacityEffect->setOpacity(1.0);
        ui->lrclist->setGraphicsEffect(opacityEffect);

        // 获取歌词列表的滚动条（仅用于程序控制，不显示给用户）
        scrollBar = ui->lrclist->verticalScrollBar();

        // 设置歌词列表的平滑滚动属性
        ui->lrclist->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        // 隐藏水平和垂直滚动条
        ui->lrclist->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->lrclist->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    // 淡入效果
    void MusicPlayer::fadeInWidget(QWidget *widget, int duration) {
        if (!widget) return;

        QGraphicsOpacityEffect *effect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
        if (!effect) {
            effect = new QGraphicsOpacityEffect(widget);
            widget->setGraphicsEffect(effect);
        }

        QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity", this);
        animation->setDuration(duration);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->setEasingCurve(QEasingCurve::InOutSine);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

    // 淡出效果
    void MusicPlayer::fadeOutWidget(QWidget *widget, int duration) {
        if (!widget) return;

        QGraphicsOpacityEffect *effect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
        if (!effect) {
            effect = new QGraphicsOpacityEffect(widget);
            widget->setGraphicsEffect(effect);
        }

        QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity", this);
        animation->setDuration(duration);
        animation->setStartValue(1.0);
        animation->setEndValue(0.0);
        animation->setEasingCurve(QEasingCurve::InOutSine);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

    // Qt6兼容的槽函数实现
    void MusicPlayer::onDurationChanged(qint64 duration) {
        if (!ui || !ui->full || !ui->bofangtiao) return; // 安全���查
        if (duration % 60000 / 1000 < 10)
            ui->full->setText(QString::number(duration / 60000) + ":0" + QString::number((duration % 60000) / 1000));
        else
            ui->full->setText(QString::number(duration / 60000) + ":" + QString::number((duration % 60000) / 1000));
        ui->bofangtiao->setMaximum(static_cast<int>(duration));
    }

    void MusicPlayer::onPositionChanged(qint64 position) {
        if (!ui || !ui->now || !ui->bofangtiao) return; // 安全检查
        if (position % 60000 / 1000 < 10)
            ui->now->setText(QString::number(position / 60000) + ":0" + QString::number((position % 60000) / 1000));
        else
            ui->now->setText(QString::number(position / 60000) + ":" + QString::number((position % 60000) / 1000));
        if (!isSliderBeingDragged) {
            ui->bofangtiao->setValue(static_cast<int>(position));
        }
    }

    void MusicPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            handlePlaybackFinished();
        }
    }

    void MusicPlayer::onPlaybackStateChanged(QMediaPlayer::PlaybackState state) {
        updatePlaybackState();
    }
}
