/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007, 2006 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2002-2003 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KPLUGINWIDGET_H
#define KPLUGINWIDGET_H

#include <QVector>
#include <QWidget>

#include <KPluginMetaData>
#include <KSharedConfig>
#include <kcmutils_export.h>

class QPushButton;
class KPluginWidgetPrivate;

class KCMUTILS_EXPORT KPluginWidget : public QWidget
{
    Q_OBJECT

public:
    KPluginWidget(QWidget *parent = nullptr);

    ~KPluginWidget();

    void addPlugins(const QVector<KPluginMetaData> plugins, const QString &categoryLabel);

    void setConfig(const KConfigGroup &config);

    void clear();

    void save();

    void defaults();

    bool isDefault() const;

    /**
     * Sets the @p arguments with which the configuration modules will be initialized
     *
     * @since 5.9
     */
    void setConfigurationArguments(const QStringList &arguments);

    /**
     * Returns the configuration arguments that will be used
     *
     * @since 5.9
     */
    QStringList configurationArguments() const;

    /**
     * Shows the configuration dialog for the plugin @p pluginId if it's available
     *
     * @since 5.45
     */
    void showConfiguration(const QString &pluginId);

    /**
     * Add additional widgets to each row of the plugin selector
     * @param handler returns the additional button that should be displayed in the row
     * the handler can return a null pointer if no button should be displayed
     * @since 5.74
     */
    //     void setAdditionalButtonHandler(std::function<QPushButton*(const KPluginInfo &)> handler);

Q_SIGNALS:
    void changed(const QString &pluginId, bool enabled);

    /**
     * Emitted after the config of an embedded KCM has been saved. The
     * argument is the name of the parent component that needs to reload
     * its config
     */
    void configCommitted(const QString &pluginId);

    /**
     * Emitted after configuration is changed, tell if configuration represent default or not
     * @since 5.67
     */
    void defaulted(bool isDefault);

private:
    KPluginWidgetPrivate *const d;
};

#endif
