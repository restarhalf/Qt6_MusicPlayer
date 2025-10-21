/********************************************************************************
** Form generated from reading UI file 'MusicPlayer.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MUSICPLAYER_H
#define UI_MUSICPLAYER_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace rsh {

class Ui_MusicPlayer
{
public:
    QVBoxLayout *mainLayout;
    QSplitter *splitter_2;
    QSplitter *splitter;
    QListView *musiclist;
    QLabel *songName;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout_2;
    QLabel *now;
    QLabel *xiegang;
    QLabel *full;
    QSlider *bofangtiao;
    QSlider *voice;
    QWidget *layoutWidget1;
    QHBoxLayout *horizontalLayout;
    QPushButton *dirBtn;
    QPushButton *preBtn;
    QPushButton *startBtn;
    QPushButton *nextBtn;
    QPushButton *voiBtn;
    QListView *lrclist;

    void setupUi(QWidget *rsh__MusicPlayer)
    {
        if (rsh__MusicPlayer->objectName().isEmpty())
            rsh__MusicPlayer->setObjectName("rsh__MusicPlayer");
        rsh__MusicPlayer->resize(1488, 820);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(rsh__MusicPlayer->sizePolicy().hasHeightForWidth());
        rsh__MusicPlayer->setSizePolicy(sizePolicy);
        QFont font;
        font.setFamilies({QString::fromUtf8("Microsoft YaHei UI")});
        rsh__MusicPlayer->setFont(font);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/app_icon.ico"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        rsh__MusicPlayer->setWindowIcon(icon);
        rsh__MusicPlayer->setStyleSheet(QString::fromUtf8("QWidget\n"
"{\n"
"	background-image: url(:/images/background.png);\n"
"	color: white;\n"
"	font-family: \"Microsoft YaHei UI\", \"\345\271\274\345\234\206\";\n"
"}\n"
"\n"
"\n"
"\n"
"/* \346\214\211\351\222\256\346\240\267\345\274\217 - \351\200\217\346\230\216\350\203\214\346\231\257\344\273\245\347\273\247\346\211\277\347\210\266\347\273\204\344\273\266\350\203\214\346\231\257 */\n"
"QPushButton {\n"
"	background: transparent;\n"
"	border: none;\n"
"	border-radius: 8px;\n"
"	padding: 5px;\n"
"}\n"
"\n"
"QPushButton:hover {\n"
"	background: rgba(255, 255, 255, 0.15);\n"
"	border-radius: 8px;\n"
"	border: 1px solid rgba(255, 255, 255, 0.3);\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"	background: rgba(255, 255, 255, 0.3);\n"
"	border-radius: 8px;\n"
"	border: 2px solid rgba(74, 144, 226, 0.6);\n"
"}\n"
"\n"
"QSlider::groove:horizontal {\n"
"	border: none;\n"
"	height: 4px;\n"
"	background: rgba(255, 255, 255, 0.15);\n"
"	margin: 2px 0;\n"
"	border-radius: 2px;\n"
"}\n"
"\n"
"QSlider::handle:horizontal {\n"
"	back"
                        "ground: #EC4141;\n"
"	border: none;\n"
"	width: 14px;\n"
"	height: 14px;\n"
"	margin: -5px 0;\n"
"	border-radius: 7px;\n"
"}\n"
"\n"
"QSlider::handle:horizontal:hover {\n"
"	background: #FF5555;\n"
"	width: 16px;\n"
"	height: 16px;\n"
"	margin: -6px 0;\n"
"	border-radius: 8px;\n"
"	border: 2px solid rgba(236, 65, 65, 0.3);\n"
"}\n"
"\n"
"QSlider::handle:horizontal:pressed {\n"
"	background: #D63031;\n"
"	width: 12px;\n"
"	height: 12px;\n"
"	margin: -4px 0;\n"
"	border-radius: 6px;\n"
"}\n"
"\n"
"QSlider::sub-page:horizontal {\n"
"	background: #EC4141;\n"
"	border-radius: 2px;\n"
"}\n"
"\n"
"QSlider::add-page:horizontal {\n"
"	background: rgba(255, 255, 255, 0.15);\n"
"	border-radius: 2px;\n"
"}\n"
"\n"
"QSlider::groove:vertical {\n"
"	border: none;\n"
"	width: 4px;\n"
"	background: rgba(255, 255, 255, 0.15);\n"
"	margin: 0 2px;\n"
"	border-radius: 2px;\n"
"}\n"
"\n"
"QSlider::handle:vertical {\n"
"	background: #EC4141;\n"
"	border: none;\n"
"	height: 14px;\n"
"	width: 14px;\n"
"	margin: 0 -5px;\n"
"	border-rad"
                        "ius: 7px;\n"
"}\n"
"\n"
"QSlider::handle:vertical:hover {\n"
"	background: #FF5555;\n"
"	height: 16px;\n"
"	width: 16px;\n"
"	margin: 0 -6px;\n"
"	border-radius: 8px;\n"
"	border: 2px solid rgba(236, 65, 65, 0.3);\n"
"}\n"
"\n"
"QSlider::handle:vertical:pressed {\n"
"	background: #D63031;\n"
"	height: 12px;\n"
"	width: 12px;\n"
"	margin: 0 -4px;\n"
"	border-radius: 6px;\n"
"}\n"
"\n"
"QSlider::sub-page:vertical {\n"
"	background: rgba(255, 255, 255, 0.15);\n"
"	border-radius: 2px;\n"
"}\n"
"\n"
"QSlider::add-page:vertical {\n"
"	background: #EC4141;\n"
"	border-radius: 2px;\n"
"}\n"
"\n"
"/* \345\210\227\350\241\250\350\247\206\345\233\276\346\240\267\345\274\217 */\n"
"QListView {\n"
"	background: rgba(0, 0, 0, 120);\n"
"	border: 2px solid rgba(255, 255, 255, 50);\n"
"	border-radius: 10px;\n"
"	color: white;\n"
"	selection-background-color: rgba(74, 144, 226, 180);\n"
"	outline: none;\n"
"	padding: 5px;\n"
"}\n"
"\n"
"QListView::item {\n"
"	padding: 8px;\n"
"	border-bottom: 1px solid rgba(255, 255, 255, 30);\n"
""
                        "	border-radius: 5px;\n"
"	margin: 2px;\n"
"}\n"
"\n"
"QListView::item:selected {\n"
"	background: rgba(74, 144, 226, 200);\n"
"	color: white;\n"
"}\n"
"\n"
"QListView::item:hover {\n"
"	background: rgba(255, 255, 255, 50);\n"
"}\n"
"\n"
"/* \346\240\207\347\255\276\346\240\267\345\274\217 */\n"
"QLabel {\n"
"	color: white;\n"
"	font-weight: bold;\n"
"	background: transparent;\n"
"	padding: 2px;\n"
"}\n"
"\n"
"/* \345\210\206\345\211\262\345\231\250\346\240\267\345\274\217 */\n"
"QSplitter::handle {\n"
"	background: rgba(255, 255, 255, 100);\n"
"	border-radius: 2px;\n"
"}\n"
"\n"
"QSplitter::handle:horizontal {\n"
"	width: 3px;\n"
"}\n"
"\n"
"QSplitter::handle:vertical {\n"
"	height: 3px;\n"
"}\n"
"\n"
"QListView#lrclist {\n"
"	background: rgba(0, 0, 0, 150);\n"
"	border: 2px solid rgba(74, 144, 226, 100);\n"
"	border-radius: 15px;\n"
"	color: white;\n"
"	selection-background-color: rgba(74, 144, 226, 200);\n"
"	outline: none;\n"
"	padding: 15px;\n"
"	font-size: 13px;\n"
"	line-height: 1.6;\n"
"}\n"
"\n"
"QList"
                        "View#lrclist::item {\n"
"	padding: 12px 8px;\n"
"	border: none;\n"
"	border-radius: 8px;\n"
"	margin: 2px 0px;\n"
"	background: transparent;\n"
"	color: rgba(255, 255, 255, 180);\n"
"	text-align: center;\n"
"}\n"
"\n"
"QListView#lrclist::item:hover {\n"
"	background: rgba(255, 255, 255, 20);\n"
"	color: rgba(255, 255, 255, 220);\n"
"}\n"
"\n"
"QListView#lrclist::item:selected {\n"
"	background: rgba(74, 144, 226, 120);\n"
"	color: white;\n"
"	font-weight: bold;\n"
"}\n"
"\n"
"QScrollBar:vertical {\n"
"	background: rgba(0, 0, 0, 80);\n"
"	width: 8px;\n"
"	border-radius: 4px;\n"
"	margin: 0px;\n"
"}\n"
"\n"
"QScrollBar::handle:vertical {\n"
"	background: rgba(74, 144, 226, 150);\n"
"	border-radius: 4px;\n"
"	min-height: 20px;\n"
"}\n"
"\n"
"QScrollBar::handle:vertical:hover {\n"
"	background: rgba(74, 144, 226, 200);\n"
"}\n"
"\n"
"QScrollBar::add-line:vertical,\n"
"QScrollBar::sub-line:vertical {\n"
"	height: 0px;\n"
"	background: none;\n"
"}\n"
"\n"
"QScrollBar::add-page:vertical,\n"
"QScrollBar::sub-page:vert"
                        "ical {\n"
"	background: none;\n"
"}\n"
""));
        mainLayout = new QVBoxLayout(rsh__MusicPlayer);
        mainLayout->setSpacing(0);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(0, 0, 0, 0);
        splitter_2 = new QSplitter(rsh__MusicPlayer);
        splitter_2->setObjectName("splitter_2");
        splitter_2->setOrientation(Qt::Orientation::Horizontal);
        splitter = new QSplitter(splitter_2);
        splitter->setObjectName("splitter");
        splitter->setStyleSheet(QString::fromUtf8("background:transparent"));
        splitter->setOrientation(Qt::Orientation::Vertical);
        musiclist = new QListView(splitter);
        musiclist->setObjectName("musiclist");
        musiclist->setMinimumSize(QSize(0, 600));
        musiclist->setStyleSheet(QString::fromUtf8("/* \345\210\227\350\241\250\350\247\206\345\233\276\346\240\267\345\274\217 */\n"
"QListView {\n"
"	background: rgba(0, 0, 0, 120);\n"
"	border: 2px solid rgba(255, 255, 255, 50);\n"
"	border-radius: 10px;\n"
"	color: white;\n"
"	selection-background-color: rgba(74, 144, 226, 180);\n"
"	outline: none;\n"
"	padding: 5px;\n"
"}\n"
"\n"
"QListView::item {\n"
"	padding: 8px;\n"
"	border-bottom: 1px solid rgba(255, 255, 255, 30);\n"
"	border-radius: 5px;\n"
"	margin: 2px;\n"
"}\n"
"\n"
"QListView::item:selected {\n"
"	background: rgba(74, 144, 226, 200);\n"
"	color: white;\n"
"}\n"
"\n"
"QListView::item:hover {\n"
"	background: rgba(255, 255, 255, 50);\n"
"}\n"
""));
        musiclist->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
        splitter->addWidget(musiclist);
        songName = new QLabel(splitter);
        songName->setObjectName("songName");
        songName->setStyleSheet(QString::fromUtf8("/* \346\240\207\347\255\276\346\240\267\345\274\217 */\n"
"QLabel {\n"
"	color: black;\n"
"	font-weight: bold;\n"
"	background: transparent;\n"
"	padding: 2px;\n"
"}"));
        songName->setAlignment(Qt::AlignmentFlag::AlignCenter);
        splitter->addWidget(songName);
        layoutWidget = new QWidget(splitter);
        layoutWidget->setObjectName("layoutWidget");
        horizontalLayout_2 = new QHBoxLayout(layoutWidget);
        horizontalLayout_2->setSpacing(10);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        now = new QLabel(layoutWidget);
        now->setObjectName("now");
        now->setMinimumSize(QSize(40, 0));
        now->setMaximumSize(QSize(40, 16777215));
        now->setStyleSheet(QString::fromUtf8(""));
        now->setAlignment(Qt::AlignmentFlag::AlignCenter);

        horizontalLayout_2->addWidget(now);

        xiegang = new QLabel(layoutWidget);
        xiegang->setObjectName("xiegang");
        xiegang->setMinimumSize(QSize(10, 0));
        xiegang->setMaximumSize(QSize(10, 16777215));
        xiegang->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_2->addWidget(xiegang);

        full = new QLabel(layoutWidget);
        full->setObjectName("full");
        full->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_2->addWidget(full);

        bofangtiao = new QSlider(layoutWidget);
        bofangtiao->setObjectName("bofangtiao");
        bofangtiao->setStyleSheet(QString::fromUtf8(""));
        bofangtiao->setOrientation(Qt::Orientation::Horizontal);
        bofangtiao->setInvertedAppearance(false);

        horizontalLayout_2->addWidget(bofangtiao);

        voice = new QSlider(layoutWidget);
        voice->setObjectName("voice");
        voice->setMaximum(100);
        voice->setValue(50);
        voice->setOrientation(Qt::Orientation::Vertical);

        horizontalLayout_2->addWidget(voice);

        splitter->addWidget(layoutWidget);
        layoutWidget1 = new QWidget(splitter);
        layoutWidget1->setObjectName("layoutWidget1");
        layoutWidget1->setEnabled(true);
        layoutWidget1->setContextMenuPolicy(Qt::ContextMenuPolicy::DefaultContextMenu);
        horizontalLayout = new QHBoxLayout(layoutWidget1);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        dirBtn = new QPushButton(layoutWidget1);
        dirBtn->setObjectName("dirBtn");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(dirBtn->sizePolicy().hasHeightForWidth());
        dirBtn->setSizePolicy(sizePolicy1);
        dirBtn->setMinimumSize(QSize(0, 0));
        dirBtn->setStyleSheet(QString::fromUtf8(""));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/dir.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        dirBtn->setIcon(icon1);
        dirBtn->setIconSize(QSize(45, 45));

        horizontalLayout->addWidget(dirBtn);

        preBtn = new QPushButton(layoutWidget1);
        preBtn->setObjectName("preBtn");
        sizePolicy1.setHeightForWidth(preBtn->sizePolicy().hasHeightForWidth());
        preBtn->setSizePolicy(sizePolicy1);
        preBtn->setStyleSheet(QString::fromUtf8(""));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/images/prew.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        preBtn->setIcon(icon2);
        preBtn->setIconSize(QSize(45, 45));

        horizontalLayout->addWidget(preBtn);

        startBtn = new QPushButton(layoutWidget1);
        startBtn->setObjectName("startBtn");
        sizePolicy1.setHeightForWidth(startBtn->sizePolicy().hasHeightForWidth());
        startBtn->setSizePolicy(sizePolicy1);
        startBtn->setStyleSheet(QString::fromUtf8(""));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/images/pause.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        startBtn->setIcon(icon3);
        startBtn->setIconSize(QSize(45, 45));

        horizontalLayout->addWidget(startBtn);

        nextBtn = new QPushButton(layoutWidget1);
        nextBtn->setObjectName("nextBtn");
        sizePolicy1.setHeightForWidth(nextBtn->sizePolicy().hasHeightForWidth());
        nextBtn->setSizePolicy(sizePolicy1);
        nextBtn->setAutoFillBackground(false);
        nextBtn->setStyleSheet(QString::fromUtf8(""));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/images/next.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        nextBtn->setIcon(icon4);
        nextBtn->setIconSize(QSize(45, 45));
        nextBtn->setAutoDefault(false);

        horizontalLayout->addWidget(nextBtn);

        voiBtn = new QPushButton(layoutWidget1);
        voiBtn->setObjectName("voiBtn");
        sizePolicy1.setHeightForWidth(voiBtn->sizePolicy().hasHeightForWidth());
        voiBtn->setSizePolicy(sizePolicy1);
        voiBtn->setStyleSheet(QString::fromUtf8(""));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/images/voice.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        voiBtn->setIcon(icon5);
        voiBtn->setIconSize(QSize(45, 45));

        horizontalLayout->addWidget(voiBtn);

        splitter->addWidget(layoutWidget1);
        splitter_2->addWidget(splitter);
        lrclist = new QListView(splitter_2);
        lrclist->setObjectName("lrclist");
        lrclist->setEnabled(true);
        lrclist->setMinimumSize(QSize(1100, 820));
        lrclist->setContextMenuPolicy(Qt::ContextMenuPolicy::DefaultContextMenu);
        lrclist->setStyleSheet(QString::fromUtf8("/* \346\255\214\350\257\215\345\210\227\350\241\250\350\247\206\345\233\276\344\274\230\345\214\226\346\240\267\345\274\217 */\n"
"QListView {\n"
"	background: rgba(0, 0, 0, 150);\n"
"	border: 2px solid rgba(74, 144, 226, 100);\n"
"	border-radius: 15px;\n"
"	color: white;\n"
"	selection-background-color: rgba(74, 144, 226, 200);\n"
"	outline: none;\n"
"	padding: 15px;\n"
"	font-size: 13px;\n"
"	line-height: 1.6;\n"
"}\n"
"\n"
"QListView::item {\n"
"	padding: 12px 8px;\n"
"	border: none;\n"
"	border-radius: 8px;\n"
"	margin: 2px 0px;\n"
"	background: transparent;\n"
"	color: rgba(255, 255, 255, 180);\n"
"	text-align: center;\n"
"}\n"
"\n"
"QListView::item:hover {\n"
"	background: rgba(255, 255, 255, 20);\n"
"	color: rgba(255, 255, 255, 220);\n"
"}\n"
"\n"
"QListView::item:selected {\n"
"	background: rgba(74, 144, 226, 120);\n"
"	color: white;\n"
"	font-weight: bold;\n"
"}\n"
"\n"
"/* \346\273\232\345\212\250\346\235\241\347\276\216\345\214\226 */\n"
"QScrollBar:vertical {\n"
"	background: rgba(0, 0, 0, 80);\n"
""
                        "	width: 8px;\n"
"	border-radius: 4px;\n"
"	margin: 0px;\n"
"}\n"
"\n"
"QScrollBar::handle:vertical {\n"
"	background: rgba(74, 144, 226, 150);\n"
"	border-radius: 4px;\n"
"	min-height: 20px;\n"
"}\n"
"\n"
"QScrollBar::handle:vertical:hover {\n"
"	background: rgba(74, 144, 226, 200);\n"
"}\n"
"\n"
"QScrollBar::add-line:vertical,\n"
"QScrollBar::sub-line:vertical {\n"
"	height: 0px;\n"
"	background: none;\n"
"}\n"
"\n"
"QScrollBar::add-page:vertical,\n"
"QScrollBar::sub-page:vertical {\n"
"	background: none;\n"
"}\n"
""));
        lrclist->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
        lrclist->setTabKeyNavigation(true);
        lrclist->setProperty("showDropIndicator", QVariant(false));
        lrclist->setMovement(QListView::Movement::Snap);
        lrclist->setViewMode(QListView::ViewMode::ListMode);
        lrclist->setItemAlignment(Qt::AlignmentFlag::AlignCenter);
        splitter_2->addWidget(lrclist);

        mainLayout->addWidget(splitter_2);


        retranslateUi(rsh__MusicPlayer);
        QObject::connect(musiclist, SIGNAL(doubleClicked(QModelIndex)), rsh__MusicPlayer, SLOT(musicName_clicked(QModelIndex)));
        QObject::connect(bofangtiao, SIGNAL(sliderReleased()), rsh__MusicPlayer, SLOT(Audio_release()));
        QObject::connect(bofangtiao, SIGNAL(valueChanged(int)), rsh__MusicPlayer, SLOT(Audio_valueChanged(int)));
        QObject::connect(bofangtiao, SIGNAL(sliderPressed()), rsh__MusicPlayer, SLOT(Audio_pressed()));
        QObject::connect(voiBtn, SIGNAL(clicked()), rsh__MusicPlayer, SLOT(VoiceBtn_clicked()));
        QObject::connect(startBtn, SIGNAL(clicked()), rsh__MusicPlayer, SLOT(StartBtn_clicked()));
        QObject::connect(voice, SIGNAL(valueChanged(int)), rsh__MusicPlayer, SLOT(Voice_valueChanged(int)));
        QObject::connect(nextBtn, SIGNAL(clicked()), rsh__MusicPlayer, SLOT(NextBtn_clicked()));
        QObject::connect(preBtn, SIGNAL(clicked()), rsh__MusicPlayer, SLOT(PrewBtn_clicked()));
        QObject::connect(dirBtn, SIGNAL(clicked()), rsh__MusicPlayer, SLOT(DirBtn_clicked()));
        QObject::connect(lrclist, SIGNAL(clicked(QModelIndex)), rsh__MusicPlayer, SLOT(musicName_clicked(QModelIndex)));
        QObject::connect(lrclist, SIGNAL(clicked(QModelIndex)), rsh__MusicPlayer, SLOT(Lrc_clicked(QModelIndex)));

        QMetaObject::connectSlotsByName(rsh__MusicPlayer);
    } // setupUi

    void retranslateUi(QWidget *rsh__MusicPlayer)
    {
        rsh__MusicPlayer->setWindowTitle(QCoreApplication::translate("rsh::MusicPlayer", "MusicPlayer", nullptr));
        songName->setText(QString());
        now->setText(QCoreApplication::translate("rsh::MusicPlayer", "0:00", nullptr));
        xiegang->setText(QCoreApplication::translate("rsh::MusicPlayer", "/", nullptr));
        full->setText(QCoreApplication::translate("rsh::MusicPlayer", "0:00", nullptr));
        dirBtn->setText(QString());
        preBtn->setText(QString());
        startBtn->setText(QString());
        nextBtn->setText(QString());
        voiBtn->setText(QString());
    } // retranslateUi

};

} // namespace rsh

namespace rsh {
namespace Ui {
    class MusicPlayer: public Ui_MusicPlayer {};
} // namespace Ui
} // namespace rsh

#endif // UI_MUSICPLAYER_H
