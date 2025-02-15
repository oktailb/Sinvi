#include "include/mainwindow.h"
#include "configuration.h"
#include "defs.h"

#include <string>

#include <QApplication>

int main(int argc, char *argv[])
{
    std::map<std::string, std::string> configuration;
    ThreadSafeQueue queue;

    configuration = readConfiguration(argv[1]);
    init(configuration, queue);

    QApplication a(argc, argv);
    DebugWindow w(configuration, queue);

    w.show();

    return a.exec();
}
