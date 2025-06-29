#include <QApplication>
#include <QStyleFactory>
#include <QIcon>

#include "musicplayer.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    rsh::MusicPlayer w;
    w.show();
    return QApplication::exec();
}
