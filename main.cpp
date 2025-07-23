#include <QApplication>
#include <QStyleFactory>
#include <QIcon>
#include <memory>

#include "musicplayer.h"
#include "restarhalf.hpp"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    restarhalf();
    auto w = std::make_unique<rsh::MusicPlayer>();
    w->show();
    int result = a.exec();
    w.reset();
    return result;
}
