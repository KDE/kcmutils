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

#include <memory>

class QPushButton;
class KPluginWidgetPrivate;

/**
 * A widget that shows a list of available plugins and allows to disable/enable them and open their configuration UI.
 * @since 5.89
 */
class KCMUTILS_EXPORT KPluginWidget : public QWidget
{
    Q_OBJECT

public:
    KPluginWidget(QWidget *parent = nullptr);

    ~KPluginWidget();

    void addPlugins(const QVector<KPluginMetaData> &plugins, const QString &categoryLabel);

    void setConfig(const KConfigGroup &config);

    void clear();

    void save();

    void defaults();

    bool isDefault() const;

    /**
     * Returns true if the plugin selector has any changes that are not yet saved to configuration.
     * @see save()
     */
    bool isSaveNeeded() const;

    /**
     * Sets the @p arguments with which the configuration modules will be initialized
     */
    void setConfigurationArguments(const QStringList &arguments);

    /**
     * Returns the configuration arguments that will be used
     */
    QStringList configurationArguments() const;

    /**
     * Shows the configuration dialog for the plugin @p pluginId if it's available
     */
    void showConfiguration(const QString &pluginId);

    /**
     * Show an indicator when a plugin status is different from default
     */
    void setDefaultsIndicatorsVisible(bool isVisible);

    /**
     * Add additional widgets to each row of the plugin selector
     * @param handler returns the additional button that should be displayed in the row
     * the handler can return a null pointer if no button should be displayed
     * @since 5.74
     */
    void setAdditionalButtonHandler(std::function<QPushButton *(const KPluginMetaData &)> handler);

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
     */
    void defaulted(bool isDefault);

private:
    std::unique_ptr<KPluginWidgetPrivate> const d;
};

#endif