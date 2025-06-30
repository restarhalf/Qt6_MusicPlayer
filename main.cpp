#include <QApplication>
#include <QStyleFactory>
#include <QIcon>
#include <memory>

#include "musicplayer.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 使用智能指针管理MusicPlayer对象，避免栈对象过早销毁
    auto w = std::make_unique<rsh::MusicPlayer>();
    w->show();

    // 确保应用程序正常退出时清理资源
    int result = a.exec();

    // 显式重置智能指针，确保在QApplication销毁前清理
    w.reset();

    return result;
}
