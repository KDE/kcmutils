/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2002-2003 Daniel Molkentin <molkentin@kde.org>
    SPDX-FileCopyrightText: 2006 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCMODULELOADER_H
#define KCMODULELOADER_H

#include <KPluginMetaData>
#include <kcmodule.h>

#include <memory>

class QWidget;
class QQmlEngine;

/**
 * @short Loads a KControl Module.
 *
 */
namespace KCModuleLoader
{
/**
 * Loads a @ref KCModule. If loading fails a KCM which displays an error message is returned.
 * Starting from 5.91, the validity of the @p metaData parameter does not need to be checked when calling this function.
 *
 * @param metaData KPluginMetaData for loading the plugin
 * @return a pointer to the loaded @ref KCModule
 * @since 5.84
 */
KCMUTILS_EXPORT KCModule *
loadModule(const KPluginMetaData &metaData, QWidget *parent = nullptr, const QVariantList &args = {}, const std::shared_ptr<QQmlEngine> &engine = {});
}

#endif // KCMODULELOADER_H
