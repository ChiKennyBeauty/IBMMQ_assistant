#include <QApplication>

#include "app/MainWindow.h"
#include "app/MetaTypeRegistry.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    registerAppMetaTypes();
    MainWindow window;
    window.show();
    return app.exec();
}
