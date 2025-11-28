// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2026 Harald Sitter <sitter@kde.org>

#include "timesettings.h"

#include <QtCore/qtsymbolmacros.h>

// We want to assert that the declarative type registration actually works. For this purpose
// we first QT_DECLARE_EXTERN_SYMBOL_VOID to access it and later QT_KEEP_SYMBOL to force linking.
// If the symbol is not present at link time then that effectively becomes our assertion.
#ifdef TESTQML2
QT_DECLARE_EXTERN_SYMBOL_VOID(qml_register_types_org_kde_kcm_testqml2)
#endif

TimeSettings::TimeSettings(QObject *parent, const KPluginMetaData &data)
        : KQuickManagedConfigModule(parent, data)
{
#ifdef TESTQML2
    QT_KEEP_SYMBOL(qml_register_types_org_kde_kcm_testqml2)
#endif
    qWarning() << Q_FUNC_INFO;
}

void TimeSettings::load()
{
    qWarning() << Q_FUNC_INFO;
}
