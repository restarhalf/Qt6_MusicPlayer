//
// Created by restarhalf on 25-6-28.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MusicPlayer.h" resolved

#include "musicplayer.h"
#include "ui_MusicPlayer.h"
#include <QFileDialog>
#include <QDirIterator>

namespace rsh {
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
        if (!isSliderBeingDragged) {
            ui->bofangtiao->setValue(static_cast<int>(position));
        }

    });
}

MusicPlayer::~MusicPlayer() {
    delete ui;
}

void MusicPlayer::DirBtn_clicked() {
    isSliderBeingDragged=false;
    auto dir = QFileDialog::getExistingDirectory(this, "选择文件夹", "D:\\Local Music");
    if (dir.isEmpty()) {
        return;
    }
    QDirIterator it(dir, {"*.flac","*.mp3"},QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        auto info= it.nextFileInfo();
        auto idem = new QStandardItem(info.completeBaseName());
        idem->setData(info.canonicalFilePath(), Qt::UserRole + 1);
        musicmodel->appendRow(idem);
    }

}

void MusicPlayer::musicName_clicked(const QModelIndex &index) {
    isSliderBeingDragged = false; // 确保在点击歌曲时不再拖动进度条
    auto filePath = index.data(Qt::UserRole + 1).toString();
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
        float volume = value / 100.0; // 假设音量范围是0-100
        audioOutput->setVolume(volume);
        ui->voice->setValue(value); // 更新音量滑块的值
    }
}
} // rsh
