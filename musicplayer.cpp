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
    MusicPlayer::MusicPlayer(QWidget *parent) : QWidget(parent), ui(nullptr)
    {

        m_isValid = false;
        try
        {
            ui = new Ui::MusicPlayer;
            ui->setupUi(this);
            setWindowTitle("AnonSoyo Player");
        }
        catch (const std::exception &e)
        {
            qCritical() << "UI initialization failed:" << e.what();
            return;
        }

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

        if (!ui)
        {
            qCritical() << "UI initialization failed!";
            return;
        }

        // 逐步初始化所有组件
        try
        {
            lrcmodel = new QStandardItemModel(this);
            musicmodel = new QStandardItemModel(this);
            player = new QMediaPlayer(this);
            audioOutput = new QAudioOutput(this);

            lyricTimer = new QTimer(this);
            uiUpdateTimer = new QTimer(this);
            playbackTimer = new QTimer(this);

            lyricScrollAnimation = new QPropertyAnimation(this);
            lyricScrollTimer = new QTimer(this);
        }
        catch (const std::exception &e)
        {
            qCritical() << "Component initialization failed:" << e.what();
            return;
        }

        this->setWindowIcon(QIcon(":/images/app_icon.ico"));
        this->setMinimumSize(640, 480);
        this->setWindowFlags(Qt::Window);

        if (ui->voice)
        {
            ui->voice->hide();
            ui->voice->setMaximum(100);
            ui->voice->setMinimum(0);
            ui->voice->setValue(50);
        }

        if (ui->bofangtiao)
        {
            ui->bofangtiao->setMaximum(0);
            ui->bofangtiao->setMinimum(0);
            ui->bofangtiao->setValue(0);
        }

        if (ui->lrclist && lrcmodel)
        {
            ui->lrclist->setModel(lrcmodel);
            ui->lrclist->setSelectionMode(QAbstractItemView::SingleSelection);
            ui->lrclist->setDragEnabled(false);
            ui->lrclist->setAcceptDrops(false);
            ui->lrclist->setDragDropMode(QAbstractItemView::NoDragDrop);
            ui->lrclist->setDefaultDropAction(Qt::IgnoreAction);
        }

        if (ui->musiclist && musicmodel)
        {
            ui->musiclist->setModel(musicmodel);
            ui->musiclist->setSelectionMode(QAbstractItemView::SingleSelection);
        }

        if (lrcmodel)
        {
            lrcmodel->setHorizontalHeaderLabels({"歌词"});
            currentLyricIndex = -1;
            currentIndex = -1;

            auto defaultItem = new QStandardItem("请选择歌曲播放");
            if (defaultItem)
            {
                lrcmodel->appendRow(defaultItem);
            }
        }

        if (player && audioOutput)
        {
            player->setAudioOutput(audioOutput);
            audioOutput->setVolume(0.5f);
        }

        if (player)
        {
            connect(player, SIGNAL(durationChanged(qint64)), this, SLOT(onDurationChanged(qint64)));
            connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(onPositionChanged(qint64)));
            connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(onMediaStatusChanged(QMediaPlayer::MediaStatus)));
            connect(player, SIGNAL(playbackStateChanged(QMediaPlayer::PlaybackState)), this, SLOT(onPlaybackStateChanged(QMediaPlayer::PlaybackState)));
        }

        QTimer::singleShot(100, [this]()
                           {
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
            } });

        setupAnimations();

        volumeHideTimer = new QTimer(this);
        volumeHideTimer->setSingleShot(true);
        volumeHideTimer->setInterval(500);
        connect(volumeHideTimer, SIGNAL(timeout()), this, SLOT(hideVolumeSlider()));

        if (lyricScrollAnimation && ui && ui->lrclist)
        {
            ui->lrclist->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
            ui->lrclist->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            ui->lrclist->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

            lyricScrollAnimation->setTargetObject(ui->lrclist->verticalScrollBar());
            lyricScrollAnimation->setPropertyName("value");
        }

        if (ui && ui->voice)
        {
            ui->voice->setVisible(false);
            ui->voice->setValue(50);
            lastVolume = 50;
        }

        if (ui && ui->voiBtn)
        {
            ui->voiBtn->installEventFilter(this);
            ui->voiBtn->setAttribute(Qt::WA_Hover, true);
        }

        if (ui && ui->voice)
        {
            ui->voice->installEventFilter(this);
            ui->voice->setAttribute(Qt::WA_Hover, true);
        }

        updateVolumeIcon(50);

        // 设置按钮图标为Windows内置图标
        if (ui->dirBtn) {
            ui->dirBtn->setIcon(QIcon(":/images/dir.png"));
        }
        if (ui->preBtn) {
            ui->preBtn->setIcon(QIcon(":/images/prew.png"));
        }
        if (ui->startBtn) {
            ui->startBtn->setIcon(QIcon(":/images/pause.png"));
        }
        if (ui->nextBtn) {
            ui->nextBtn->setIcon(QIcon(":/images/next.png"));
        }
        if (ui->voiBtn) {
            ui->voiBtn->setIcon(QIcon(":/images/voice.png"));
        }
        if (ui->deleteBtn) {
            ui->deleteBtn->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
        }
        loadPlayerState();

        m_isValid = true;
    }

    MusicPlayer::~MusicPlayer()
    {
        // 立即标记对象为无效，防止任何回调访问
        m_isValid = false;

        // 停止所有定时器，防止在对象销毁时继续访问UI
        if (lyricTimer)
        {
            lyricTimer->stop();
            lyricTimer->deleteLater();
            lyricTimer = nullptr;
        }

        if (uiUpdateTimer)
        {
            uiUpdateTimer->stop();
            uiUpdateTimer->deleteLater();
            uiUpdateTimer = nullptr;
        }

        if (playbackTimer)
        {
            playbackTimer->stop();
            playbackTimer->deleteLater();
            playbackTimer = nullptr;
        }

        // 断开所有信号连接，防止在销毁���程中触发回调
        if (player)
        {
            disconnect(player, nullptr, this, nullptr);
            player->stop();
            player->setSource(QUrl());
        }

        if (lyricTimer)
        {
            disconnect(lyricTimer, nullptr, this, nullptr);
        }

        if (uiUpdateTimer)
        {
            disconnect(uiUpdateTimer, nullptr, this, nullptr);
        }

        if (playbackTimer)
        {
            disconnect(playbackTimer, nullptr, this, nullptr);
        }

        // 停止动画
        if (fadeAnimation)
        {
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

    void MusicPlayer::DirBtn_clicked()
    {
        isSliderBeingDragged = false;
        auto dir = QFileDialog::getExistingDirectory(this, "选择文件夹", "D:\\Local Music");
        if (dir.isEmpty())
        {
            return;
        }

        // 创建一个集合来存储已存在的文件路径，提高查找效率
        QSet<QString> existingFilePaths;
        for (int i = 0; i < musicmodel->rowCount(); ++i)
        {
            auto item = musicmodel->item(i);
            if (item)
            {
                QString existingPath = item->data(Qt::UserRole + 1).toString();
                if (!existingPath.isEmpty())
                {
                    existingFilePaths.insert(existingPath);
                }
            }
        }

        int addedCount = 0; // 记录新添加的歌曲数量
        QDirIterator it(dir, {"*.flac", "*.mp3"}, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            auto info = it.nextFileInfo();
            QString filePath = info.absoluteFilePath();

            // 检查文件路径是否已存在
            if (!existingFilePaths.contains(filePath))
            {
                auto item = new QStandardItem(getMusicTitle(filePath));
                item->setData(filePath, Qt::UserRole + 1); // 存储完整文件路径
                musicmodel->appendRow(item);
                existingFilePaths.insert(filePath); // 添加到已存在集合中
                addedCount++;
            }
        }

        // 可选：显示导入结果信息
        if (addedCount > 0)
        {
            // 如果需要，可以在这里添加状态栏消息或其他提示
            // statusBar()->showMessage(QString("成功导入 %1 首���歌曲").arg(addedCount), 3000);
        }
    }

    void MusicPlayer::musicName_clicked(const QModelIndex &index)
    {
        if (!player || !audioOutput || !index.isValid())
        {
            return; // 安全检查：确保播放器和索��有效
        }
        isSliderBeingDragged = false; // 确保在点击歌曲时不再拖动进度条
        auto filePath = index.data(Qt::UserRole + 1).toString();

        if (filePath.isEmpty())
        {
            return; // 安全检查：确保文件路径有效
        }

        // 重置歌词索引状态，避免自动切歌后歌词跳转失效
        currentLyricIndex = -1;
        currentIndex = -1;

        // 加载歌词
        loadLyricsFromMetadata(filePath);

        // 更新播放器信息和音乐显示信息
        updatePlayerInfo(filePath);
        updateMusicInfo(filePath); // 添加这行来更新音乐信息显示

        player->setSource(QUrl::fromLocalFile(filePath));
        // 移除或注释掉这行，因为它会覆盖updateMusicInfo设置的信息
        // if (ui && ui->songName) {
        //     ui->songName->setText(index.data(Qt::DisplayRole).toString());
        // }
        ui->startBtn->setIcon(QIcon(":/images/start.png")); // 设置开始按钮图标
        player->play();
    }
    void MusicPlayer::StartBtn_clicked()
    {
        if (!player)
            return;                   // 安全检查
        isSliderBeingDragged = false; // 确保在点击开始按钮时不再拖动进度条
        if (player->playbackState() == QMediaPlayer::PlayingState)
        {
            // 如果播放器正在播放，则暂停
            ui->startBtn->setIcon(QIcon(":/images/pause.png"));
            player->pause();
        }
        else if (player->playbackState() == QMediaPlayer::PausedState)
        {
            // 如果播放器处于暂停状态，则继续播放
            ui->startBtn->setIcon(QIcon(":/images/start.png"));
            player->play();
        }
    }

    void MusicPlayer::PrewBtn_clicked()
    {
        if (!m_isValid || !ui || !ui->musiclist || !musicmodel)
            return;

        isSliderBeingDragged = false;
        auto currentIndex = ui->musiclist->currentIndex();
        if (!currentIndex.isValid() || currentIndex.row() == 0)
        {
            return; // 没有上一首歌
        }
        auto previousIndex = ui->musiclist->model()->index(currentIndex.row() - 1, 0);
        if (previousIndex.isValid())
        {
            ui->musiclist->setCurrentIndex(previousIndex);
            musicName_clicked(previousIndex);
        }
    }

    void MusicPlayer::NextBtn_clicked()
    {
        if (!m_isValid || !ui || !ui->musiclist || !musicmodel)
            return;

        isSliderBeingDragged = false;
        auto currentIndex = ui->musiclist->currentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }
        auto nextIndex = ui->musiclist->model()->index(currentIndex.row() + 1, 0);
        if (nextIndex.isValid())
        {
            ui->musiclist->setCurrentIndex(nextIndex);
            musicName_clicked(nextIndex);
        }
        else
        {
            if (musicmodel->rowCount() > 0)
            {
                ui->musiclist->setCurrentIndex(ui->musiclist->model()->index(0, 0));
                musicName_clicked(ui->musiclist->model()->index(0, 0));
            }
        }
    }

    void MusicPlayer::VoiceBtn_clicked()
    {
        // 强化安全检查
        if (!m_isValid || !ui || !ui->voice || !audioOutput)
            return;

        if (isMuted)
        {
            // 恢复音量
            isMuted = false;
            audioOutput->setVolume(static_cast<float>(lastVolume) / 100.0f);
            ui->voice->setValue(lastVolume);
            updateVolumeIcon(lastVolume);
        }
        else
        {
            // 静音
            isMuted = true;
            lastVolume = ui->voice->value(); // 保存当前音量
            audioOutput->setVolume(0.0f);
            ui->voice->setValue(0);
            updateVolumeIcon(0);
        }
    }

    void MusicPlayer::Audio_release()
    {
        // 强化安全检查
        if (!m_isValid || !player)
            return;

        player->play();               // 确保在拖动后继续播放
        isSliderBeingDragged = false; // 用户停止拖动
    }

    void MusicPlayer::Audio_pressed()
    {
        // 强化安全检查
        if (!m_isValid || !player)
            return;

        player->pause();
        isSliderBeingDragged = true; // 用户开始拖动
    }

    void MusicPlayer::Audio_valueChanged(int value)
    {
        // 当用户拖动进度条时，实时更新时间显示
        if (isSliderBeingDragged && ui && ui->now && player)
        {
            auto position = static_cast<qint64>(value);
            if (position % 60000 / 1000 < 10)
            {
                ui->now->setText(QString::number(position / 60000) + ":0" + QString::number((position % 60000) / 1000));
            }
            else
            {
                ui->now->setText(QString::number(position / 60000) + ":" + QString::number((position % 60000) / 1000));
            }
            player->setPosition(position); // 设置播放器位置
        }
    }

    void MusicPlayer::Voice_valueChanged(int value)
    {
        // 音量调节 - 主流播放器风格
        if (!audioOutput || !ui || !ui->voice)
            return;

        // 如果音量从0变为非0，自动取消静音状态
        if (isMuted && value > 0)
        {
            isMuted = false;
        }

        // 如果手动将音量调为0，进入��音状态
        if (!isMuted && value == 0)
        {
            isMuted = true;
        }

        // 更新实际音量
        float volume = static_cast<float>(value) / 100.0f;
        audioOutput->setVolume(volume);

        // 更新音���图标
        updateVolumeIcon(value);

        // 如果不是静音状态，保存当前���量为lastVolume
        if (!isMuted && value > 0)
        {
            lastVolume = value;
        }

        // 显示音量滑块并重置隐藏定时器
        showVolumeSlider();
    }

    // 更新音量图标
    void MusicPlayer::updateVolumeIcon(int volume)
    {
        if (!ui || !ui->voiBtn)
            return;

        QString iconPath;
        if (volume == 0 || isMuted)
        {
            iconPath = ":/images/mute.png";
        }
        else
            iconPath = ":/images/voice.png";

        // 如果图标文件不存在，使用默认图标
        if (!QFile::exists(iconPath))
        {
            iconPath = ":/images/voice.png"; // 默认音量图标
        }

        ui->voiBtn->setIcon(QIcon(iconPath));
    }

    // 显示音量滑块
    void MusicPlayer::showVolumeSlider()
    {
        if (!ui || !ui->voice || !volumeHideTimer)
            return;

        // 停止隐藏定时器
        volumeHideTimer->stop();
        // 显示音量滑块
        ui->voice->setVisible(true);

        // 启动隐��定时器
        volumeHideTimer->start();
    }

    // 隐藏音量滑块
    void MusicPlayer::hideVolumeSlider()
    {
        if (!ui || !ui->voice)
            return;

        // 直接隐藏，不使用动画（避免透明度效果�������扰）
        ui->voice->setVisible(false);

        // 清除可能存在的透明度效果
        QGraphicsOpacityEffect *effect = qobject_cast<QGraphicsOpacityEffect *>(ui->voice->graphicsEffect());
        if (effect)
        {
            ui->voice->setGraphicsEffect(nullptr);
            effect->deleteLater();
        }
    }
    QString MusicPlayer::getMusicTitle(const QString &filePath)
    {
        TagLib::FileRef file(filePath.toLocal8Bit().constData());
        if (file.isNull())
        {
            return QString();
        }

        // 尝试从 MP3 文件中获取标题
        if (auto mp3File = dynamic_cast<TagLib::MPEG::File *>(file.file()))
        {
            if (auto id3v2Tag = mp3File->ID3v2Tag())
            {
                return QString::fromStdString(id3v2Tag->title().to8Bit(true));
            }
        }

        // 对于 FLAC 文件，从 Vorbis 注释中获取标题
        if (auto flacFile = dynamic_cast<TagLib::FLAC::File *>(file.file()))
        {
            auto properties = flacFile->properties();
            if (properties.contains("TITLE"))
            {
                return QString::fromStdString(properties["TITLE"].front().to8Bit(true));
            }
        }

        return QString();
    }

    // 静态正则表达式，避免重复创建临时对象
    static QRegularExpression timeRegex(R"(\[\d{2}:\d{2}\.\d{2}\])");
    static QRegularExpression lrcFormatRegex(R"(\[\d{2}:\d{2}\.\d{2}\])");

    // 从音频文���元数���中提取歌词
    QString MusicPlayer::extractLyricsFromFile(const QString &filePath)
    {
        TagLib::FileRef file(filePath.toLocal8Bit().constData());

        if (file.isNull())
        {
            return QString();
        }

        // 尝试从 MP3 文件中提取歌词
        if (auto mp3File = dynamic_cast<TagLib::MPEG::File *>(file.file()))
        {
            if (auto id3v2Tag = mp3File->ID3v2Tag())
            {
                // 查找 USLT 帧（未同步歌词）
                auto frameList = id3v2Tag->frameList("USLT");
                if (!frameList.isEmpty())
                {
                    if (auto lyricsFrame = dynamic_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame *>(frameList.front()))
                    {
                        return QString::fromStdString(lyricsFrame->text().to8Bit(true));
                    }
                }
            }
        }

        // 对于 FLAC 文件，从 Vorbis 注释中获取歌词
        if (auto flacFile = dynamic_cast<TagLib::FLAC::File *>(file.file()))
        {
            auto properties = flacFile->properties();
            if (properties.contains("LYRICS"))
            {
                return QString::fromStdString(properties["LYRICS"].front().to8Bit(true));
            }
            if (properties.contains("UNSYNCEDLYRICS"))
            {
                return QString::fromStdString(properties["UNSYNCEDLYRICS"].front().to8Bit(true));
            }
        }

        return QString();
    }

    // 加载歌词（优先从元数据���其次从LRC文件）
    void MusicPlayer::loadLyricsFromMetadata(const QString &filePath)
    {
        lyrics.clear();
        lrcmodel->clear();
        currentLyricIndex = -1;

        // 首先尝试从音频文件元数据中���取歌词
        QString embeddedLyrics = extractLyricsFromFile(filePath);

        if (!embeddedLyrics.isEmpty())
        {
            // 如果是 LRC 格式的嵌入歌词 - 使用静态正则表达式
            if (embeddedLyrics.contains(lrcFormatRegex))
            {
                parseLrcContent(embeddedLyrics);
            }
            else
            {
                // 普通文本歌词
                auto item = new QStandardItem(embeddedLyrics);
                lrcmodel->appendRow(item);
            }
            return;
        }

        // 如果元数据中没有歌词，尝试加载 LRC 文件
        QString lrcPath = findLrcFile(filePath);
        if (!lrcPath.isEmpty())
        {
            parseLrcFile(lrcPath);
        }
        else
        {
            auto defaultItem = new QStandardItem("未找到歌词");
            lrcmodel->appendRow(defaultItem);
        }
    }
    void MusicPlayer::parseLrcFile(const QString &lrcFilePath)
    {
        lyrics.clear();
        lrcmodel->clear();

        QFile file(lrcFilePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            auto defaultItem = new QStandardItem("无法打开歌词文件");
            defaultItem->setTextAlignment(Qt::AlignCenter); // 居中对齐
            lrcmodel->appendRow(defaultItem);
            addLyricPaddingItems(); // 添加填充项
            return;
        }

        QTextStream in(&file);
        in.setEncoding(QStringConverter::Utf8);

        QRegularExpression timeRegex(R"(\[(\d{2}):(\d{2})\.(\d{2})\])");

        while (!in.atEnd())
        {
            QString line = in.readLine().trimmed();
            if (line.isEmpty())
                continue;

            // 提取歌词文本（移除时间标签）
            QString text = line;
            QRegularExpressionMatchIterator matches = timeRegex.globalMatch(line);
            while (matches.hasNext())
            {
                QRegularExpressionMatch match = matches.next();
                text = text.remove(match.captured(0));
            }
            text = text.trimmed();

            // 提取所有时间戳
            matches = timeRegex.globalMatch(line);
            while (matches.hasNext())
            {
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
        std::sort(lyrics.begin(), lyrics.end(), [](const LyricLine &a, const LyricLine &b)
                  { return a.timestamp < b.timestamp; });

        // 先添加顶部填充项
        int paddingCount = calculatePaddingItemsCount();
        for (int i = 0; i < paddingCount; i++)
        {
            auto paddingItem = new QStandardItem("");
            paddingItem->setTextAlignment(Qt::AlignCenter);
            paddingItem->setFlags(Qt::ItemIsEnabled); // 设置为不可选择
            lrcmodel->appendRow(paddingItem);
        }

        // 添加歌词到模型中显示，设置居中对齐
        for (const auto &lyric : lyrics)
        {
            auto item = new QStandardItem(lyric.text);
            item->setTextAlignment(Qt::AlignCenter); // 设置文本居中对齐
            lrcmodel->appendRow(item);
        }

        // 添加底部填充项
        for (int i = 0; i < paddingCount; i++)
        {
            auto paddingItem = new QStandardItem("");
            paddingItem->setTextAlignment(Qt::AlignCenter);
            paddingItem->setFlags(Qt::ItemIsEnabled); // 设置为不可选择
            lrcmodel->appendRow(paddingItem);
        }

        file.close();
    }
    void MusicPlayer::parseLrcContent(const QString &lrcContent)
    {
        lyrics.clear();
        lrcmodel->clear();

        QTextStream stream(const_cast<QString *>(&lrcContent), QIODevice::ReadOnly);
        QRegularExpression timeRegex(R"(\[(\d{2}):(\d{2})\.(\d{2})\])");

        while (!stream.atEnd())
        {
            QString line = stream.readLine().trimmed();
            if (line.isEmpty())
                continue;

            // 提取歌词文本（移除时间标签）
            QString text = line;
            QRegularExpressionMatchIterator matches = timeRegex.globalMatch(line);
            while (matches.hasNext())
            {
                QRegularExpressionMatch match = matches.next();
                text = text.remove(match.captured(0));
            }
            text = text.trimmed();

            // 提取所有时间戳
            matches = timeRegex.globalMatch(line);
            while (matches.hasNext())
            {
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
        std::sort(lyrics.begin(), lyrics.end(), [](const LyricLine &a, const LyricLine &b)
                  { return a.timestamp < b.timestamp; });

        // 先添加顶部填充项
        int paddingCount = calculatePaddingItemsCount();
        for (int i = 0; i < paddingCount; i++)
        {
            auto paddingItem = new QStandardItem("");
            paddingItem->setTextAlignment(Qt::AlignCenter);
            paddingItem->setFlags(Qt::ItemIsEnabled); // 设置为不可选择
            lrcmodel->appendRow(paddingItem);
        }

        // 添加歌词到���型中显示，设置居中对齐
        for (const auto &lyric : lyrics)
        {
            auto item = new QStandardItem(lyric.text);
            item->setTextAlignment(Qt::AlignCenter); // 设置文本居中对齐
            lrcmodel->appendRow(item);
        }

        // 添加底部填充项
        for (int i = 0; i < paddingCount; i++)
        {
            auto paddingItem = new QStandardItem("");
            paddingItem->setTextAlignment(Qt::AlignCenter);
            paddingItem->setFlags(Qt::ItemIsEnabled); // 设置为不可选择
            lrcmodel->appendRow(paddingItem);
        }
    }
    QString MusicPlayer::findLrcFile(const QString &musicFilePath)
    {
        QFileInfo musicInfo(musicFilePath);
        QString baseName = musicInfo.completeBaseName();
        QString titlename = getMusicTitle(musicFilePath);
        QString dirPath = musicInfo.absolutePath();

        // 可能的 LRC 文件路径
        QStringList possiblePaths = {
            dirPath + "/" + baseName + ".lrc",
            dirPath + "/" + baseName + ".LRC",
            dirPath + "/" + titlename + ".lrc",
            dirPath + "/" + titlename + ".LRC",
        };

        // 检查每个可能的路径
        for (const QString &path : possiblePaths)
        {
            if (QFile::exists(path))
            {
                return path;
            }
        }

        return QString(); // 未找到
    }
    void MusicPlayer::updateCurrentLyric()
    {
        // 强化安全检查 - 检查对象有效性
        if (!m_isValid || !player || lyrics.isEmpty() || !lrcmodel || !ui)
        {
            return;
        }

        // 获取当前播放位置
        qint64 currentPosition = player->position();
        int newIndex1 = -1;
        int newIndex = -1;

        // 找到当前���放位置对应的歌词 - 修复数组越界问题
        for (int i = 0; i < lyrics.size(); ++i)
        {
            if (currentPosition >= lyrics[i].timestamp)
            {
                if (newIndex >= 0 && newIndex < lyrics.size() &&
                    lyrics[i].timestamp == lyrics[newIndex].timestamp)
                {
                    newIndex1 = i;
                }
                else
                {
                    newIndex = i;
                    newIndex1 = -1;
                }
            }
            else
            {
                break;
            }
        }

        // 如果歌词索引发生变化，更新高亮
        if ((newIndex != currentLyricIndex && newIndex >= 0) ||
            (newIndex1 != currentIndex && newIndex1 >= 0))
        {
            currentLyricIndex = newIndex;
            currentIndex = newIndex1;

            // 应用新的歌��高亮样��
            updateLyricHighlight();

            // 平滑滚动到当前歌词（需要加上填充项的偏移量）
            int paddingCount = calculatePaddingItemsCount();
            smoothScrollToLyric(currentLyricIndex + paddingCount);
        }
    }

    // 简化：更新歌词高亮的方法 - 非播放歌词半透明
    void MusicPlayer::updateLyricHighlight()
    {
        // 强化安全检查 - 检查对象有效性
        if (!m_isValid || !lrcmodel || !ui || !ui->lrclist)
        {
            return;
        }

        int paddingCount = calculatePaddingItemsCount();
        int displayCurrentLyricIndex = currentLyricIndex + paddingCount; // 显示索引需要加上填充项偏移量
        int displayCurrentIndex = (currentIndex >= 0) ? currentIndex + paddingCount : -1;

        // 清除所有歌词的样式
        for (int i = 0; i < lrcmodel->rowCount(); ++i)
        {
            auto item = lrcmodel->item(i);
            if (!item)
                continue; // 跳过空项

            QFont font = item->font();
            font.setBold(false);
            font.setPointSize(12);

            if (i == displayCurrentLyricIndex)
            {
                // 当前歌词：亮白色，粗体，较大字体
                font.setBold(true);
                font.setPointSize(14);
                item->setFont(font);
                item->setForeground(QBrush(QColor(255, 255, 255, 255))); // 完全不透明白色
                item->setBackground(QBrush(QColor(74, 144, 226, 100)));  // 蓝色背景
            }
            else
            {
                // 非播放歌词：半透明白色，正常字体
                item->setFont(font);
                item->setForeground(QBrush(QColor(255, 255, 255, 128))); // 半透明白色
                item->setBackground(QBrush(Qt::transparent));
            }
        }

        // 处理双歌词情况
        if (displayCurrentIndex >= 0 && displayCurrentIndex < lrcmodel->rowCount())
        {
            auto item = lrcmodel->item(displayCurrentIndex);
            if (item)
            {
                QFont font = item->font();
                font.setBold(true);
                font.setPointSize(14);
                item->setFont(font);
                item->setForeground(QBrush(QColor(255, 255, 255, 255))); // 完全不透明白色
                item->setBackground(QBrush(QColor(74, 144, 226, 100)));
            }
        }

        // 强制刷新视图
        if (ui->lrclist)
        {
            ui->lrclist->update();
        }
    }

    // 新增：平滑滚动到指定歌词的方法
    void MusicPlayer::smoothScrollToLyric(int lyricIndex)
    {
        // 强化安全检查 - 检查对象有效性和边界
        if (!m_isValid || !ui || !ui->lrclist || !lrcmodel ||
            lyricIndex < 0 || lyricIndex >= lrcmodel->rowCount())
        {
            return;
        }

        // 获取滚动条
        QScrollBar *scrollBar = ui->lrclist->verticalScrollBar();
        if (!scrollBar)
            return;

        // 停止之前���动画避免冲突
        if (fadeAnimation)
        {
            fadeAnimation->stop();
            fadeAnimation->deleteLater();
            fadeAnimation = nullptr;
        }

        // 计算目标位置（让当前歌词显示在视图的中央）
        QModelIndex index = lrcmodel->index(lyricIndex, 0);
        QRect itemRect = ui->lrclist->visualRect(index);

        int viewHeight = ui->lrclist->viewport()->height();
        int currentScrollValue = scrollBar->value();

        // 计算目标滚动位��，让歌词显示在中央偏上位置
        int targetScrollValue = itemRect.top() + scrollBar->value() - (viewHeight * 0.4);

        // 限制滚动范围
        targetScrollValue = qBound(scrollBar->minimum(), targetScrollValue, scrollBar->maximum());

        // 如果目标位置和当前位置差距很小，就不需要动画
        if (abs(targetScrollValue - currentScrollValue) < 10)
        {
            ui->lrclist->scrollTo(index, QAbstractItemView::PositionAtCenter);
            QTimer::singleShot(50, [this]()
                               { pulseCurrentLyric(); });
            return;
        }

        // 创建平滑滚动动画
        fadeAnimation = new QPropertyAnimation(scrollBar, "value", this);

        // 根据距离调整动画时间，距离越远动画时间越长，但有上下限
        int distance = abs(targetScrollValue - currentScrollValue);
        int animationDuration = qBound(300, distance * 2, 1000); // 300ms到1000ms之间

        fadeAnimation->setDuration(animationDuration);
        fadeAnimation->setEasingCurve(QEasingCurve::OutCubic); // ��用更平滑的缓动曲线
        fadeAnimation->setStartValue(currentScrollValue);
        fadeAnimation->setEndValue(targetScrollValue);

        // 连接动画完成信号
        connect(fadeAnimation, &QPropertyAnimation::finished, [this]()
                {
            // 滚动完成后添加脉冲效果
            QTimer::singleShot(100, [this]() {
                pulseCurrentLyric();
            }); });

        // 添加动画值变化时的回调，可以实现更复杂的效果
        connect(fadeAnimation, &QPropertyAnimation::valueChanged, [this](const QVariant &value)
                {
            // 可以在这里添加滚动过程中的额外效果
            Q_UNUSED(value) });

        fadeAnimation->start();
    }

    // 新增：当前歌词脉冲效果
    void MusicPlayer::pulseCurrentLyric()
    {
        // 强化安全检查 - 检查对象有效性和边界
        if (!m_isValid || !lrcmodel ||
            currentLyricIndex < 0 || currentLyricIndex >= lrcmodel->rowCount())
        {
            return;
        }

        auto item = lrcmodel->item(currentLyricIndex);
        if (!item)
            return;

        // 避免���动画过程中直接修改item，使用更安全的方式
        // 检查item是否仍然有效
        if (currentLyricIndex >= lrcmodel->rowCount())
        {
            return; // item���能已经被删除
        }

        // 创建一个安全���动画更新机制 - 使用QTimer替代opacity动画
        QTimer *pulseTimer = new QTimer(this);
        pulseTimer->setSingleShot(false);
        pulseTimer->setInterval(50); // 50ms间隔更新

        // 使用简单的计数器来控制动画效果
        int *animationStep = new int(0);
        const int maxSteps = 6; // 总共6步完成动画

        connect(pulseTimer, &QTimer::timeout, [this, animationStep, maxSteps, pulseTimer]()
                {
            // 在每次���新时重新验证item的有效性
            if (!m_isValid || !lrcmodel ||
                currentLyricIndex < 0 || currentLyricIndex >= lrcmodel->rowCount()) {
                pulseTimer->stop();
                pulseTimer->deleteLater();
                delete animationStep;
                return;
            }

            auto currentItem = lrcmodel->item(currentLyricIndex);
            if (!currentItem) {
                pulseTimer->stop();
                pulseTimer->deleteLater();
                delete animationStep;
                return;
            }

            // 计算当前alpha值 (0 到 1.0)
            double progress = static_cast<double>(*animationStep) / maxSteps;
            double alpha = 0.0 + (1.0 * progress);
            QColor highlightColor = QColor(74, 144, 226, static_cast<int>(alpha * 150 + 100));

            // 使用try-catch保护关闭操作
            try {
                currentItem->setBackground(QBrush(highlightColor));
            } catch (...) {
                // 如果设置背景失败，忽略错误
                qWarning() << "Failed to set background color for lyric item";
            }

            (*animationStep)++;
            if (*animationStep >= maxSteps) {
                pulseTimer->stop();
                pulseTimer->deleteLater();
                delete animationStep;
            } });

        pulseTimer->start();
    }
    // 添加到 musicplayer.cpp 文件中的方法实现

    QString MusicPlayer::getArtistName(const QString &filePath)
    {
        TagLib::FileRef file(filePath.toLocal8Bit().constData());
        if (file.isNull())
        {
            return QString();
        }

        // 尝试从 MP3 文件中获取歌手
        if (auto mp3File = dynamic_cast<TagLib::MPEG::File *>(file.file()))
        {
            if (auto id3v2Tag = mp3File->ID3v2Tag())
            {
                return QString::fromStdString(id3v2Tag->artist().to8Bit(true));
            }
        }

        // 对于 FLAC 文件，从 Vorbis 注释中获取歌手
        if (auto flacFile = dynamic_cast<TagLib::FLAC::File *>(file.file()))
        {
            auto properties = flacFile->properties();
            if (properties.contains("ARTIST"))
            {
                return QString::fromStdString(properties["ARTIST"].front().to8Bit(true));
            }
        }

        // 通用方法获取歌手信息
        if (file.tag())
        {
            return QString::fromStdString(file.tag()->artist().to8Bit(true));
        }

        return QString();
    }

    QString MusicPlayer::getAlbumName(const QString &filePath)
    {
        TagLib::FileRef file(filePath.toLocal8Bit().constData());
        if (file.isNull())
        {
            return QString();
        }

        // 尝试从 MP3 文件中获取专辑
        if (auto mp3File = dynamic_cast<TagLib::MPEG::File *>(file.file()))
        {
            if (auto id3v2Tag = mp3File->ID3v2Tag())
            {
                return QString::fromStdString(id3v2Tag->album().to8Bit(true));
            }
        }

        // 对于 FLAC 文件，从 Vorbis 注释中获取专辑
        if (auto flacFile = dynamic_cast<TagLib::FLAC::File *>(file.file()))
        {
            auto properties = flacFile->properties();
            if (properties.contains("ALBUM"))
            {
                return QString::fromStdString(properties["ALBUM"].front().to8Bit(true));
            }
        }

        // 通用方法获取专辑信息
        if (file.tag())
        {
            return QString::fromStdString(file.tag()->album().to8Bit(true));
        }

        return QString();
    }

    void MusicPlayer::updateMusicInfo(const QString &filePath)
    {
        // 获取歌曲信息
        QString title = getMusicTitle(filePath);
        QString artist = getArtistName(filePath);
        QString album = getAlbumName(filePath);

        // 如果没有标题，使用文件名
        if (title.isEmpty())
        {
            QFileInfo fileInfo(filePath);
            title = fileInfo.baseName();
        }

        // 如果没有歌手信息，使用"未知歌手"
        if (artist.isEmpty())
        {
            artist = "未知歌手";
        }

        // 如果没有专辑信息，��用"未知专辑"
        if (album.isEmpty())
        {
            album = "未知专辑";
        }

        // 组合格式：歌名-歌手-专辑
        QString displayText = QString("%1\n%2\n%3").arg(title, artist, album);
        ui->songName->setText(displayText);
    }
    // 更新播放状态
    void MusicPlayer::updatePlaybackState()
    {
        // 强化安全检查 - 检查对象有效性
        if (!m_isValid || !player || !ui)
            return;

        QMediaPlayer::PlaybackState state = player->playbackState();

        // 根据播放状态更新���钮文本和状态
        switch (state)
        {
        case QMediaPlayer::PlayingState:
            // 可以在这里更新播放按钮图标或��本
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
    void MusicPlayer::updatePlayerInfo(const QString &filePath)
    {
        if (filePath.isEmpty())
            return;

        QFileInfo fileInfo(filePath);
        QString fileName = fileInfo.completeBaseName();

        // 更新当前播放歌曲索引
        for (int i = 0; i < musicmodel->rowCount(); ++i)
        {
            auto item = musicmodel->item(i);
            if (item && item->data(Qt::UserRole + 1).toString() == filePath)
            {
                currentIndex = i;
                ui->musiclist->setCurrentIndex(musicmodel->index(i, 0));
                break;
            }
        }

        // 添加到播放历史
        if (currentIndex >= 0 && (playHistory.isEmpty() || playHistory.last() != currentIndex))
        {
            playHistory.append(currentIndex);
            historyIndex = playHistory.size() - 1;

            // 限制历史记录长度
            if (playHistory.size() > 50)
            {
                playHistory.removeFirst();
                historyIndex--;
            }
        }

        // 保�����放状态
        savePlayerState();
    }

    // 处理播放完成
    void MusicPlayer::handlePlaybackFinished()
    {
        switch (currentPlaybackMode)
        {
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
            if (musicmodel->rowCount() > 0)
            {
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
    void MusicPlayer::updatePlaybackMode(PlaybackMode mode)
    {
        currentPlaybackMode = mode;

        // 可以在这里更新UI显示当前播放模式
        QString modeText;
        switch (mode)
        {
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

    // 新音乐列表
    void MusicPlayer::refreshMusicList()
    {
        // 保存当前选择的项目
        QString currentFilePath;
        auto currentIdx = ui->musiclist->currentIndex();
        if (currentIdx.isValid())
        {
            currentFilePath = currentIdx.data(Qt::UserRole + 1).toString();
        }

        // 重新扫描音乐文件夹（如果需要的话）
        // 这里可以添加自动刷新逻辑

        // 恢复选择状态
        if (!currentFilePath.isEmpty())
        {
            for (int i = 0; i < musicmodel->rowCount(); ++i)
            {
                auto item = musicmodel->item(i);
                if (item && item->data(Qt::UserRole + 1).toString() == currentFilePath)
                {
                    ui->musiclist->setCurrentIndex(musicmodel->index(i, 0));
                    break;
                }
            }
        }
    }

    // 更新音量显示
    void MusicPlayer::updateVolumeDisplay()
    {
        // 强化安全检查 - 检查对象有���性
        if (!m_isValid || !audioOutput || !ui || !ui->voice)
        {
            return;
        }

        if (audioOutput)
        {
            int volume = static_cast<int>(audioOutput->volume() * 100);
            if (ui->voice->value() != volume)
            {
                ui->voice->setValue(volume);
            }
        }
    }

    void MusicPlayer::Lrc_clicked(const QModelIndex &index)
    {
        // 强化安全检查 - 检��对象有效性
        if (!m_isValid || !index.isValid() || !player || lyrics.isEmpty() ||
            !ui || !ui->lrclist || !lrcmodel)
        {
            return;
        }

        int clickedRow = index.row();
        int paddingCount = calculatePaddingItemsCount();

        // 计算实际的歌词索引（需要减去��部填充项的数量）
        int actualLyricIndex = clickedRow - paddingCount;

        // 边界检查 - 确保点击的是实际的歌词行，不是填充项
        if (actualLyricIndex < 0 || actualLyricIndex >= lyrics.size())
        {
            return; // 点击的是填充项，不处理
        }

        // ��止重复快速���击导致的问题
        static QTime lastClickTime;
        QTime currentTime = QTime::currentTime();
        if (lastClickTime.isValid() && lastClickTime.msecsTo(currentTime) < 100)
        {
            return; // 100毫秒内的重��点击将被忽略
        }
        lastClickTime = currentTime;

        // 停止当前动画避免冲突
        if (fadeAnimation)
        {
            fadeAnimation->stop();
            fadeAnimation->deleteLater();
            fadeAnimation = nullptr;
        }

        // 设置播放位置到点击的歌词时间戳
        qint64 targetTimestamp = lyrics[actualLyricIndex].timestamp;
        if (player)
        {
            player->setPosition(targetTimestamp);
        }

        // 更新当前歌词��引
        currentLyricIndex = actualLyricIndex;
        currentIndex = -1; // 重置双歌词索引

        // 处理相同时间戳的情况（添加边界检查）
        if (actualLyricIndex > 0 && actualLyricIndex < lyrics.size() &&
            lyrics[actualLyricIndex].timestamp == lyrics[actualLyricIndex - 1].timestamp)
        {
            currentLyricIndex = actualLyricIndex - 1;
            currentIndex = actualLyricIndex;
        }
        else if (actualLyricIndex < lyrics.size() - 1 && actualLyricIndex >= 0 &&
                 lyrics[actualLyricIndex].timestamp == lyrics[actualLyricIndex + 1].timestamp)
        {
            currentLyricIndex = actualLyricIndex;
            currentIndex = actualLyricIndex + 1;
        }

        // 安全清除选择
        if (ui && ui->lrclist)
        {
            ui->lrclist->clearSelection();
        }

        // 立即更新歌词高亮显示
        updateLyricHighlight();

        // 滚动到点击的歌词（需要加上填充项的偏移量）
        smoothScrollToLyric(currentLyricIndex + paddingCount);
    }

    // 保持播放器状态
    void MusicPlayer::savePlayerState()
    {
        // 保存歌曲列表到文件
        QFile file("music_list.dat");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            for (int i = 0; i < musicmodel->rowCount(); ++i) {
                auto item = musicmodel->item(i);
                if (item) {
                    QString filePath = item->data(Qt::UserRole + 1).toString();
                    if (!filePath.isEmpty()) {
                        out << filePath << "\n";
                    }
                }
            }
            file.close();
        }

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
    void MusicPlayer::loadPlayerState()
    {
        // 加载歌曲列表
        QFile file("music_list.dat");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString filePath = in.readLine().trimmed();
                if (!filePath.isEmpty() && QFile::exists(filePath)) {
                    auto item = new QStandardItem(getMusicTitle(filePath));
                    item->setData(filePath, Qt::UserRole + 1);
                    musicmodel->appendRow(item);
                }
            }
            file.close();
        }

        // 从配置文件或注册表加载播放器状态

        // 例：从QSettings加载
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

    // 设���动画效果
    void MusicPlayer::setupAnimations()
    {
        // 初始化歌词列表的透明度效果
        opacityEffect = new QGraphicsOpacityEffect(this);
        opacityEffect->setOpacity(1.0);
        ui->lrclist->setGraphicsEffect(opacityEffect);
        scrollBar = ui->lrclist->verticalScrollBar();
        ui->lrclist->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        ui->lrclist->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->lrclist->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    // 淡入效果
    void MusicPlayer::fadeInWidget(QWidget *widget, int duration)
    {
        if (!widget)
            return;

        QGraphicsOpacityEffect *effect = qobject_cast<QGraphicsOpacityEffect *>(widget->graphicsEffect());
        if (!effect)
        {
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
    void MusicPlayer::fadeOutWidget(QWidget *widget, int duration)
    {
        if (!widget)
            return;

        QGraphicsOpacityEffect *effect = qobject_cast<QGraphicsOpacityEffect *>(widget->graphicsEffect());
        if (!effect)
        {
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
    void MusicPlayer::onDurationChanged(qint64 duration)
    {
        if (!ui || !ui->full || !ui->bofangtiao)
            return; // 安全检查
        if (duration % 60000 / 1000 < 10)
            ui->full->setText(QString::number(duration / 60000) + ":0" + QString::number((duration % 60000) / 1000));
        else
            ui->full->setText(QString::number(duration / 60000) + ":" + QString::number((duration % 60000) / 1000));
        ui->bofangtiao->setMaximum(static_cast<int>(duration));
    }

    void MusicPlayer::onPositionChanged(qint64 position)
    {
        if (!ui || !ui->now || !ui->bofangtiao)
            return; // 安全检查
        if (position % 60000 / 1000 < 10)
            ui->now->setText(QString::number(position / 60000) + ":0" + QString::number((position % 60000) / 1000));
        else
            ui->now->setText(QString::number(position / 60000) + ":" + QString::number((position % 60000) / 1000));
        if (!isSliderBeingDragged)
        {
            ui->bofangtiao->setValue(static_cast<int>(position));
        }
    }

    void MusicPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
    {
        if (status == QMediaPlayer::EndOfMedia)
        {
            handlePlaybackFinished();
        }
    }

    void MusicPlayer::onPlaybackStateChanged(QMediaPlayer::PlaybackState state)
    {
        updatePlaybackState();
    }

    // 音量控制鼠标悬停事件处理函数
    void MusicPlayer::onVolumeButtonEntered()
    {
        // 鼠标进入音量按钮时显示音量滑块
        showVolumeSlider();
    }

    void MusicPlayer::onVolumeButtonLeft()
    {
        // 鼠标离开音量按钮时启动隐藏定时器
        if (volumeHideTimer && ui && ui->voice)
        {
            // 检查鼠标是否在音量滑块区域内
            QPoint globalPos = QCursor::pos();
            QPoint sliderPos = ui->voice->mapFromGlobal(globalPos);

            // 如果鼠标不在滑块区域内，启动隐藏定时器
            if (!ui->voice->rect().contains(sliderPos))
            {
                volumeHideTimer->start();
            }
        }
    }

    void MusicPlayer::onVolumeSliderEntered()
    {
        // 鼠标进入音量滑块时停止隐藏定时器
        if (volumeHideTimer)
        {
            volumeHideTimer->stop();
        }
    }

    void MusicPlayer::onVolumeSliderLeft()
    {
        // 鼠标离开音量滑块时，只有在没有被拖拽的情况下才启动隐藏定时器
        if (isVolumeSliderBeingDragged)
        {
            return; // 如果正在拖拽，不隐藏
        }

        if (volumeHideTimer && ui && ui->voiBtn)
        {
            // 检查鼠标是否在音量按钮区域内
            QPoint globalPos = QCursor::pos();
            QPoint buttonPos = ui->voiBtn->mapFromGlobal(globalPos);

            // 如果鼠标不在按钮区域内，启动隐藏定时器
            if (!ui->voiBtn->rect().contains(buttonPos))
            {
                volumeHideTimer->start();
            }
        }
    }
    bool MusicPlayer::eventFilter(QObject *obj, QEvent *event)
    {
        if (!m_isValid || !ui)
        {
            return QWidget::eventFilter(obj, event);
        }

        // 处理音量按钮的鼠标事件
        if (obj == ui->voiBtn)
        {
            switch (event->type())
            {
            case QEvent::Enter:
                onVolumeButtonEntered();
                break;
            case QEvent::Leave:
                onVolumeButtonLeft();
                break;
            default:
                break;
            }
        }
        // 处理音量滑块的鼠标事件
        else if (obj == ui->voice)
        {
            switch (event->type())
            {
            case QEvent::Enter:
                onVolumeSliderEntered();
                break;
            case QEvent::Leave:
                onVolumeSliderLeft();
                break;
            case QEvent::MouseButtonPress:
                // 鼠标按下时标记为正在拖拽
                isVolumeSliderBeingDragged = true;
                if (volumeHideTimer)
                {
                    volumeHideTimer->stop(); // 停止隐藏定��器
                }
                break;
            case QEvent::MouseButtonRelease:
                // 鼠标释放���取消拖拽标记
                isVolumeSliderBeingDragged = false;
                // 延迟一小段时间后再启动隐藏定时器，避免立即隐藏
                QTimer::singleShot(500, [this]()
                                   {
                        if (!isVolumeSliderBeingDragged && volumeHideTimer) {
                            volumeHideTimer->start();
                        } });
                break;
            default:
                break;
            }
        }

        // ��用父类的事件过滤器
        return QWidget::eventFilter(obj, event);
    }

    // 新增：计算需要的填充项数量
    int MusicPlayer::calculatePaddingItemsCount()
    {
        // 强化安全检查
        if (!m_isValid || !ui || !ui->lrclist)
        {
            return 0;
        }

        // 获取歌词列表���图的高度
        int viewHeight = ui->lrclist->viewport()->height();

        // 估算每行歌词的高度（包括间距）
        int itemHeight = 30; // 假设每行歌词高度约30像素

        // 计算���图中能显示的行数
        int visibleRows = viewHeight / itemHeight;

        // 需��的填充行数 = 视图高度的一半 / 每行高度
        int paddingRows = (visibleRows / 2) + 1;

        return paddingRows;
    }

    // 新增：添加歌词填充项，使歌词能够居中显示
    void MusicPlayer::addLyricPaddingItems()
    {
        // 强化安全检查
        if (!m_isValid || !lrcmodel)
        {
            return;
        }

        int paddingCount = calculatePaddingItemsCount();

        // 在歌词列表开��添加空行
        for (int i = 0; i < paddingCount; i++)
        {
            auto paddingItem = new QStandardItem("");
            paddingItem->setTextAlignment(Qt::AlignCenter);
            paddingItem->setFlags(Qt::ItemIsEnabled); // 设置为不可选择
            lrcmodel->insertRow(0, paddingItem);
        }

        // 在歌词列表末尾添加空行
        for (int i = 0; i < paddingCount; i++)
        {
            auto paddingItem = new QStandardItem("");
            paddingItem->setTextAlignment(Qt::AlignCenter);
            paddingItem->setFlags(Qt::ItemIsEnabled); // 设置为不可选择
            lrcmodel->appendRow(paddingItem);
        }

        // 更新当前歌词索引，因为在开头添加了填充项
        if (currentLyricIndex >= 0)
        {
            currentLyricIndex += paddingCount;
        }
    }
    void MusicPlayer::DeleteBtn_clicked()
    {
        if (!m_isValid || !ui || !ui->musiclist || !musicmodel)
            return;

        auto currentIndex = ui->musiclist->currentIndex();
        if (!currentIndex.isValid())
            return;

        // 获取当前播放的歌曲索引
        QString currentFilePath = currentIndex.data(Qt::UserRole + 1).toString();

        // 删除选中的歌曲
        musicmodel->removeRow(currentIndex.row());

        // 如果删除的是当前播放的歌曲，停止播放
        if (player && player->playbackState() == QMediaPlayer::PlayingState &&
            currentFilePath == player->source().toString().remove("file:///"))
        {
            player->stop();
            ui->startBtn->setIcon(QIcon(":/images/pause.png"));
            ui->songName->setText("请选择歌曲播放");
        }

        // 保存更新后的列表
        savePlayerState();
    }

    void MusicPlayer::ModeBtn_clicked()
    {
        // 循环播放模式
        switch (currentPlaybackMode) {
            case Sequential:
                currentPlaybackMode = Loop;
                ui->modeBtn->setText("单曲循环");
                break;
            case Loop:
                currentPlaybackMode = LoopAll;
                ui->modeBtn->setText("列表循环");
                break;
            case LoopAll:
                currentPlaybackMode = Random;
                ui->modeBtn->setText("随机播放");
                break;
            case Random:
                currentPlaybackMode = Sequential;
                ui->modeBtn->setText("顺序播放");
                break;
        }
    }
}
