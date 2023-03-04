/*
  SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
  SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>

  SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "main.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusServiceWatcher>
#include <QDebug>
#include <QIcon>
#include <QRegularExpression>
#include <QStandardPaths>

#include <KAboutData>
#include <KActivities/ResourceInstance>
#include <KAuthorized>
#include <KCModuleProxy>
#include <KLocalizedString>
#include <KPluginMetaData>

#if __has_include(<KStartupInfo>)
#include <KStartupInfo>
#include <private/qtx11extras_p.h>
#define HAVE_X11 1
#endif

#include <algorithm>
#include <iostream>

inline QList<KPluginMetaData> findKCMsMetaData()
{
    QList<KPluginMetaData> metaDataList = KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms"));
    metaDataList << KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms/systemsettings"));
    metaDataList << KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms/systemsettings_qwidgets"));
    metaDataList << KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms/kinfocenter"));
    return metaDataList;
}

bool KCMShell::isRunning()
{
    const QString owner = QDBusConnection::sessionBus().interface()->serviceOwner(m_serviceName);
    if (owner == QDBusConnection::sessionBus().baseService()) {
        return false; // We are the one and only.
    }

    qDebug() << "kcmshell6 with modules" << m_serviceName << "is already running.";

#ifdef HAVE_X11
    QDBusInterface iface(m_serviceName, QStringLiteral("/KCModule/dialog"), QStringLiteral("org.kde.KCMShellMultiDialog"));
    QDBusReply<void> reply = iface.call(QStringLiteral("activate"), QX11Info::nextStartupId());
    if (!reply.isValid()) {
        qDebug() << "Calling D-Bus function dialog::activate() failed.";
        return false; // Error, we have to do it ourselves.
    }
#endif

    return true;
}

KCMShellMultiDialog::KCMShellMultiDialog(KPageDialog::FaceType dialogFace, QWidget *parent)
    : KCMultiDialog(parent)
{
    setFaceType(dialogFace);
    setModal(false);

    QDBusConnection::sessionBus().registerObject(QStringLiteral("/KCModule/dialog"), this, QDBusConnection::ExportScriptableSlots);

    connect(this, &KCMShellMultiDialog::currentPageChanged, this, [](KPageWidgetItem *newPage, KPageWidgetItem *oldPage) {
        Q_UNUSED(oldPage);
        KCModuleProxy *activeModule = newPage->widget()->findChild<KCModuleProxy *>();
        if (activeModule) {
            KActivities::ResourceInstance::notifyAccessed(QUrl(QLatin1String("kcm:") + activeModule->metaData().pluginId()),
                                                          QStringLiteral("org.kde.systemsettings"));
        }
    });
}

void KCMShellMultiDialog::activate(const QByteArray &asn_id)
{
#ifdef HAVE_X11
    setAttribute(Qt::WA_NativeWindow, true);
    KStartupInfo::setNewStartupId(windowHandle(), asn_id);
#endif
}

void KCMShell::setServiceName(const QString &dbusName)
{
    m_serviceName = QLatin1String("org.kde.kcmshell_") + dbusName;
    QDBusConnection::sessionBus().registerService(m_serviceName);
}

void KCMShell::waitForExit()
{
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(this);
    watcher->setConnection(QDBusConnection::sessionBus());
    watcher->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
    watcher->addWatchedService(m_serviceName);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &KCMShell::appExit);
    exec();
}

void KCMShell::appExit(const QString &appId, const QString &oldName, const QString &newName)
{
    Q_UNUSED(appId);
    Q_UNUSED(newName);

    if (!oldName.isEmpty()) {
        qDebug() << appId << "closed, quitting.";
        qApp->quit();
    }
}

int main(int _argc, char *_argv[])
{
    const bool qpaVariable = qEnvironmentVariableIsSet("QT_QPA_PLATFORM");
    KCMShell app(_argc, _argv);
    if (!qpaVariable) {
        // don't leak the env variable to processes we start
        qunsetenv("QT_QPA_PLATFORM");
    }
    KLocalizedString::setApplicationDomain("kcmshell6");
    KAboutData aboutData(QStringLiteral("kcmshell6"), //
                         i18n("System Settings Module"),
                         QLatin1String(PROJECT_VERSION),
                         i18n("A tool to start single system settings modules"),
                         KAboutLicense::GPL,
                         i18n("(c) 1999-2016, The KDE Developers"));

    aboutData.addAuthor(i18n("Frans Englich"), i18n("Maintainer"), QStringLiteral("frans.englich@kde.org"));
    aboutData.addAuthor(i18n("Daniel Molkentin"), QString(), QStringLiteral("molkentin@kde.org"));
    aboutData.addAuthor(i18n("Matthias Hoelzer-Kluepfel"), QString(), QStringLiteral("hoelzer@kde.org"));
    aboutData.addAuthor(i18n("Matthias Elter"), QString(), QStringLiteral("elter@kde.org"));
    aboutData.addAuthor(i18n("Matthias Ettrich"), QString(), QStringLiteral("ettrich@kde.org"));
    aboutData.addAuthor(i18n("Waldo Bastian"), QString(), QStringLiteral("bastian@kde.org"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    parser.addOption(QCommandLineOption(QStringLiteral("list"), i18n("List all possible modules")));
    parser.addPositionalArgument(QStringLiteral("module"), i18n("Configuration module to open"));
    parser.addOption(QCommandLineOption(QStringLiteral("lang"), i18n("Specify a particular language"), QLatin1String("language")));
    parser.addOption(QCommandLineOption(QStringLiteral("silent"), i18n("Do not display main window")));
    parser.addOption(QCommandLineOption(QStringLiteral("args"), i18n("Arguments for the module"), QLatin1String("arguments")));
    parser.addOption(QCommandLineOption(QStringLiteral("icon"), i18n("Use a specific icon for the window"), QLatin1String("icon")));
    parser.addOption(QCommandLineOption(QStringLiteral("caption"), i18n("Use a specific caption for the window"), QLatin1String("caption")));

    parser.parse(app.arguments());
    aboutData.processCommandLine(&parser);

    parser.process(app);

    const QString lang = parser.value(QStringLiteral("lang"));
    if (!lang.isEmpty()) {
        std::cout << i18n("--lang is deprecated. Please set the LANGUAGE environment variable instead").toLocal8Bit().constData() << std::endl;
    }

    if (parser.isSet(QStringLiteral("list"))) {
        std::cout << i18n("The following modules are available:").toLocal8Bit().constData() << '\n';

        QList<KPluginMetaData> plugins = findKCMsMetaData();
        int maxLen = 0;

        for (const auto &plugin : plugins) {
            const int len = plugin.pluginId().size();
            maxLen = std::max(maxLen, len);
        }

        for (const auto &plugin : plugins) {
            QString comment = plugin.description();
            if (comment.isEmpty()) {
                comment = i18n("No description available");
            }

            const QString entry = QStringLiteral("%1 - %2").arg(plugin.pluginId().leftJustified(maxLen, QLatin1Char(' ')), comment);

            std::cout << entry.toLocal8Bit().constData() << '\n';
        }

        std::cout << std::endl;

        return 0;
    }

    if (parser.positionalArguments().isEmpty()) {
        parser.showHelp();
        return -1;
    }

    QString serviceName;
    QList<KPluginMetaData> metaDataList;

    QStringList args = parser.positionalArguments();
    args.removeDuplicates();
    for (const QString &arg : args) {
        KPluginMetaData data(arg);
        if (data.isValid()) {
            metaDataList << data;
            if (!serviceName.isEmpty()) {
                serviceName += QLatin1Char('_');
            }
            serviceName += data.pluginId();
        } else {
            // Look in the namespaces for systemsettings/kinfocenter
            const static auto knownKCMs = findKCMsMetaData();
            const QStringList possibleIds{arg, QStringLiteral("kcm_") + arg, QStringLiteral("kcm") + arg};
            bool foundKCM = std::any_of(knownKCMs.begin(), knownKCMs.end(), [&possibleIds, &metaDataList, &serviceName](const KPluginMetaData &data) {
                bool idMatches = possibleIds.contains(data.pluginId());
                if (idMatches) {
                    metaDataList << data;
                    if (!serviceName.isEmpty()) {
                        serviceName += QLatin1Char('_');
                    }
                    serviceName += data.pluginId();
                }
                return idMatches;
            });
            if (foundKCM) {
                continue;
            }
        }
    }

    /* Check if this particular module combination is already running */
    app.setServiceName(serviceName);
    if (app.isRunning()) {
        app.waitForExit();
        return 0;
    }

    KPageDialog::FaceType ftype = KPageDialog::Plain;

    const int modCount = metaDataList.count();
    if (modCount == 0) {
        return -1;
    }

    if (modCount > 1) {
        ftype = KPageDialog::List;
    }

    KCMShellMultiDialog *dlg = new KCMShellMultiDialog(ftype);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    if (parser.isSet(QStringLiteral("caption"))) {
        dlg->setWindowTitle(parser.value(QStringLiteral("caption")));
    } else if (modCount == 1) {
        dlg->setWindowTitle(metaDataList.constFirst().name());
    }

    const QStringList moduleArgs = parser.value(QStringLiteral("args")).split(QRegularExpression(QStringLiteral(" +")));
    for (const KPluginMetaData &m : std::as_const(metaDataList)) {
        dlg->addModule(m, moduleArgs);
    }

    if (parser.isSet(QStringLiteral("icon"))) {
        dlg->setWindowIcon(QIcon::fromTheme(parser.value(QStringLiteral("icon"))));
    } else {
        dlg->setWindowIcon(QIcon::fromTheme(metaDataList.constFirst().iconName()));
    }

    if (app.desktopFileName() == QLatin1String("org.kde.kcmshell6")) {
        const QString path = metaDataList.constFirst().fileName();

        if (path.endsWith(QLatin1String(".desktop"))) {
            app.setDesktopFileName(path);
        } else {
            app.setDesktopFileName(metaDataList.constFirst().pluginId());
        }
    }

    dlg->show();

    app.exec();

    return 0;
}
// vim: sw=4 et sts=4
