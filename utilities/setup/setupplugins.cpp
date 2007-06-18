/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2004-01-02
 * Description : setup Kipi plugins tab.
 * 
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// QT includes.

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLayout>
#include <QString>
#include <QLabel>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>

// libkipi includes.

#include <libkipi/pluginloader.h>
#include <libkipi/version.h>

// Local includes.

#include "setupplugins.h"
#include "setupplugins.moc"

namespace Digikam
{

class SetupPluginsPriv
{
public:

    SetupPluginsPriv()
    {
        pluginsNumber = 0;
        kipiConfig    = 0;
    }

    QLabel*             pluginsNumber;

    KIPI::ConfigWidget* kipiConfig;
};

SetupPlugins::SetupPlugins(QWidget* parent )
            : QWidget(parent)
{
    d = new SetupPluginsPriv;
    QVBoxLayout *layout = new QVBoxLayout(parent);
    QHBoxLayout *hlay   = new QHBoxLayout();
    layout->addLayout(hlay);

    d->pluginsNumber    = new QLabel(parent);

    QLabel *KipiVersion = new QLabel(i18n("Kipi library version: %1").arg(kipi_version), parent);
    KipiVersion->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );

    hlay->addWidget(d->pluginsNumber, 1);
    hlay->addStretch(1);
    hlay->addWidget(KipiVersion, 1);

    d->kipiConfig = KIPI::PluginLoader::instance()->configWidget( parent );
    QString pluginsListHelp = i18n("<p>A list of available Kipi plugins "
                                    "appears below.");
    d->kipiConfig->setWhatsThis( pluginsListHelp);
    layout->addWidget( d->kipiConfig );
}

SetupPlugins::~SetupPlugins()
{
    delete d;
}

void SetupPlugins::initPlugins(int kipiPluginsNumber)
{
    d->pluginsNumber->setText(i18np("1 Kipi plugin found",
                                    "%1 Kipi plugins found",
                                    kipiPluginsNumber));
}

void SetupPlugins::applyPlugins()
{
    d->kipiConfig->apply();
}

}  // namespace Digikam

