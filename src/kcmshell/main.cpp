/*
  SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
  SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>

  SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "main.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QIcon>
#include <QRegularExpression>
#include <QStandardPaths>

#include <KAboutData>
#include <KActivities/ResourceInstance>
#include <KAuthorized>
#include <KCModule>
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

KCMShellMultiDialog::KCMShellMultiDialog(KPageDialog::FaceType dialogFace, QWidget *parent)
    : KCMultiDialog(parent)
{
    setFaceType(dialogFace);
    setModal(false);

    connect(this, &KCMShellMultiDialog::currentPageChanged, this, [](KPageWidgetItem *newPage) {
        if (KCModule *activeModule = newPage->widget()->findChild<KCModule *>()) {
            KActivities::ResourceInstance::notifyAccessed(QUrl(QLatin1String("kcm:") + activeModule->metaData().pluginId()),
                                                          QStringLiteral("org.kde.systemsettings"));
        }
    });
}

int main(int argc, char *argv[])
{
    const bool qpaVariable = qEnvironmentVariableIsSet("QT_QPA_PLATFORM");
    QApplication app(argc, argv);
    if (!qpaVariable) {
        // don't leak the env variable to processes we start
        qunsetenv("QT_QPA_PLATFORM");
    }
    KLocalizedString::setApplicationDomain("kcmshell6");
    KAboutData aboutData(QStringLiteral("kcmshell6"),
                         QString(),
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
    parser.addOption(QCommandLineOption(QStringLiteral("highlight"), i18n("Show an indicator when settings have changed from their default value")));

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
        QVariantList list;
        for (const auto &arg : moduleArgs) {
            list << QVariant::fromValue(arg);
        }
        dlg->addModule(m, list);
    }

    if (parser.isSet(QStringLiteral("icon"))) {
        dlg->setWindowIcon(QIcon::fromTheme(parser.value(QStringLiteral("icon"))));
    } else {
        dlg->setWindowIcon(QIcon::fromTheme(metaDataList.constFirst().iconName()));
    }

    if (parser.isSet(QStringLiteral("highlight"))) {
        dlg->setDefaultsIndicatorsVisible(true);
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
