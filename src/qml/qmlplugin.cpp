#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include "../core/kpluginproxymodel.h"

class KCMUtilsQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
    void initializeEngine(QQmlEngine * /*engine*/, const char *uri) override
    {
        qmlRegisterType<KPluginProxyModel>(uri, 1, 0, "ProxyModel");
    }

    void registerTypes(const char * /*uri*/) override{};
};

#include "qmlplugin.moc"
