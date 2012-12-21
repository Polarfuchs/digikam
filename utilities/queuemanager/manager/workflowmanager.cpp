/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-12-17
 * Description : workflow manager.
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "workflowmanager.moc"

// Qt includes

#include <QMutex>
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QTextStream>
#include <QTextCodec>

// KDE includes

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>

// Local includes

#include "queuesettings.h"
#include "batchtoolsmanager.h"

namespace Digikam
{

class WorkflowManager::Private
{
public:

    Private()
        :mutex(QMutex::Recursive)
    {
        modified = false;
    }

    bool         modified;

    QList<Workflow> qList;
    QString      file;

    QMutex       mutex;
};

class WorkflowManagerCreator
{
public:

    WorkflowManager object;
};

K_GLOBAL_STATIC(WorkflowManagerCreator, creator)

WorkflowManager* WorkflowManager::instance()
{
    return &creator->object;
}

WorkflowManager::WorkflowManager()
    : d(new Private)
{
    d->file = KStandardDirs::locateLocal("appdata", "queue.xml");
}

WorkflowManager::~WorkflowManager()
{
    save();
    clear();
    delete d;
}

void WorkflowManager::insert(const Workflow& q)
{
    if (q.title.isNull())
    {
        return;
    }

    d->modified = true;
    insertPrivate(q);
}

void WorkflowManager::remove(const Workflow& q)
{
    if (q.title.isNull())
    {
        return;
    }

    d->modified = true;
    removePrivate(q);
}

void WorkflowManager::insertPrivate(const Workflow& q)
{
    if (q.title.isNull())
    {
        return;
    }

    {
        QMutexLocker lock(&d->mutex);
        d->qList.append(q);
        kDebug() << "add : " << q.title;
    }

    emit signalQueueSettingsAdded(q.title);
}

void WorkflowManager::removePrivate(const Workflow& q)
{
    if (q.title.isNull())
    {
        return;
    }

    {
        QMutexLocker lock(&d->mutex);

        for (QList<Workflow>::iterator it = d->qList.begin(); it != d->qList.end(); ++it)
        {
            if (it->title == q.title)
            {
                it = d->qList.erase(it);
                break;
            }
        }
    }

    emit signalQueueSettingsRemoved(q.title);
}

Workflow WorkflowManager::findByTitle(const QString& title) const
{
    QMutexLocker lock(&d->mutex);

    foreach(const Workflow& q, d->qList)
    {
        if (q.title == title)
        {
            return q;
        }
    }

    return Workflow();
}

void WorkflowManager::clear()
{
    QList<Workflow> oldQueues = d->qList;

    {
        QMutexLocker lock(&d->mutex);
        d->qList.clear();
    }

    foreach(const Workflow& q, d->qList)
    {
        emit signalQueueSettingsRemoved(q.title);
    }
}

QList<Workflow> WorkflowManager::queueSettingsList() const
{
    return d->qList;
}

bool WorkflowManager::save()
{
    // If not modified don't save the file
    if (!d->modified)
    {
        return true;
    }

    QDomDocument doc("queuelist");
    doc.setContent(QString("<!DOCTYPE XMLQueueList><queuelist version=\"1.0\" client=\"digikam\" encoding=\"UTF-8\"/>"));
    QDomElement docElem = doc.documentElement();

    {
        QMutexLocker lock(&d->mutex);

        foreach(const Workflow& q, d->qList)
        {

            QDomElement elm = doc.createElement("queue");
            QDomElement data;

            data = doc.createElement(QString::fromLatin1("queuetitle"));
            data.setAttribute(QString::fromLatin1("value"), q.title);
            elm.appendChild(data);

            data = doc.createElement(QString::fromLatin1("queuedesc"));
            data.setAttribute(QString::fromLatin1("value"), q.desc);
            elm.appendChild(data);

            data = doc.createElement(QString::fromLatin1("renamingparser"));
            data.setAttribute(QString::fromLatin1("value"), q.qSettings.renamingParser);
            elm.appendChild(data);

            data = doc.createElement(QString::fromLatin1("workingurl"));
            data.setAttribute(QString::fromLatin1("value"), q.qSettings.workingUrl.path());
            elm.appendChild(data);

            data = doc.createElement(QString::fromLatin1("conflictrule"));
            data.setAttribute(QString::fromLatin1("value"), q.qSettings.conflictRule);
            elm.appendChild(data);

            data = doc.createElement(QString::fromLatin1("renamingrule"));
            data.setAttribute(QString::fromLatin1("value"), q.qSettings.renamingRule);
            elm.appendChild(data);

            data = doc.createElement(QString::fromLatin1("rawloadingrule"));
            data.setAttribute(QString::fromLatin1("value"), q.qSettings.rawLoadingRule);
            elm.appendChild(data);

            // ----------------------

            QDomElement rawdecodingsettings = doc.createElement(QString::fromLatin1("rawdecodingsettings"));
            elm.appendChild(rawdecodingsettings);
            DRawDecoding::decodingSettingsToXml(q.qSettings.rawDecodingSettings, rawdecodingsettings);

            // ----------------------

            foreach(const BatchToolSet& set, q.aTools)
            {
                QDomElement batchtool = doc.createElement("tool");
                elm.appendChild(batchtool);

                data = doc.createElement(QString::fromLatin1("toolname"));
                data.setAttribute(QString::fromLatin1("value"), set.name);
                batchtool.appendChild(data);

                data = doc.createElement(QString::fromLatin1("toolgroup"));
                data.setAttribute(QString::fromLatin1("value"), set.group);
                batchtool.appendChild(data);

                data = doc.createElement(QString::fromLatin1("index"));
                data.setAttribute(QString::fromLatin1("value"), set.index);
                batchtool.appendChild(data);

                data = doc.createElement(QString::fromLatin1("version"));
                data.setAttribute(QString::fromLatin1("value"), set.version);
                batchtool.appendChild(data);

                for (BatchToolSettings::const_iterator it = set.settings.constBegin() ; it != set.settings.constEnd() ; ++it)
                {
                    data = doc.createElement("parameter");
                    data.setAttribute(QString::fromLatin1("name"),  it.key());
                    data.setAttribute(QString::fromLatin1("type"),  it.value().typeName());
                    data.setAttribute(QString::fromLatin1("value"), it.value().toString());
                    batchtool.appendChild(data);
                }
            }

            docElem.appendChild(elm);
        }
    }

    QFile file(d->file);

    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec(QTextCodec::codecForName("UTF-8"));
    stream.setAutoDetectUnicode(true);
    stream << doc.toString(4);
    file.close();

    return true;
}

bool WorkflowManager::load(QStringList& failed)
{
    d->modified = false;

    QFile file(d->file);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QDomDocument doc("queuelist");

    if (!doc.setContent(&file))
    {
        return false;
    }

    QDomElement docElem = doc.documentElement();

    if (docElem.tagName() != "queuelist")
    {
        return false;
    }

    for (QDomNode n = docElem.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        QDomElement e = n.toElement();

        if (e.isNull())
        {
            continue;
        }

        if (e.tagName() != "queue")
        {
            continue;
        }

        Workflow q;
        bool     versionOk = true;

        for (QDomNode n2 = e.firstChild(); !n2.isNull(); n2 = n2.nextSibling())
        {
            QDomElement e2 = n2.toElement();
            if (e2.isNull())
            {
                continue;
            }

            QString name2 = e2.tagName();
            QString val2  = e2.attribute(QString::fromLatin1("value"));
            bool ok       = true;

            if (name2 == "queuetitle")
            {
                q.title = val2;
            }
            else if (name2 == "queuedesc")
            {
                q.desc = val2;
            }
            else if (name2 == "renamingparser")
            {
                q.qSettings.renamingParser = val2;
            }
            else if (name2 == "workingurl")
            {
                q.qSettings.workingUrl = KUrl(val2);
            }
            else if (name2 == "conflictrule")
            {
                q.qSettings.conflictRule = (QueueSettings::ConflictRule)val2.toUInt(&ok);
            }
            else if (name2 == "renamingrule")
            {
                q.qSettings.renamingRule = (QueueSettings::RenamingRule)val2.toUInt(&ok);
            }
            else if (name2 == "rawloadingrule")
            {
                q.qSettings.rawLoadingRule = (QueueSettings::RawLoadingRule)val2.toUInt(&ok);
            }
            else if (name2 == "rawdecodingsettings")
            {
                DRawDecoding::decodingSettingsFromXml(e2, q.qSettings.rawDecodingSettings);
            }
            else if (name2 == "tool")
            {
                BatchToolSet set;

                for (QDomNode n3 = e2.firstChild(); !n3.isNull(); n3 = n3.nextSibling())
                {
                    QDomElement e3 = n3.toElement();
                    if (e3.isNull())
                    {
                        continue;
                    }

                    QString name3  = e3.tagName();
                    QString val3   = e3.attribute(QString::fromLatin1("value"));

                    if (name3 == "toolname")
                    {
                        set.name = val3;
                    }
                    else if (name3 == "toolgroup")
                    {
                        set.group = (BatchTool::BatchToolGroup)val3.toInt(&ok);
                    }
                    else if (name3 == "index")
                    {
                        set.index = val3.toInt(&ok);
                    }
                    else if (name3 == "version")
                    {
                        set.version = val3.toInt(&ok);
                    }
                    else if (name3 == "parameter")
                    {
                        QString pname = e3.attribute(QString::fromLatin1("name"));
                        QString type  = e3.attribute(QString::fromLatin1("type"));
                        set.settings.insert(pname, QVariant(val3).convert(QVariant::nameToType(type.toAscii())));
                    }
                }

                BatchTool* const tool = BatchToolsManager::instance()->findTool(set.name, set.group);
                if (tool)
                {
                    if (set.version == tool->toolVersion())
                    {
                        q.aTools.append(set);
                    }
                }
                else
                {
                    versionOk   = false;
                    d->modified = true;
                }
            }
        }

        if (versionOk)
        {
            // We only instert workflow if all tools version are compatible
            insertPrivate(q);
        }
        else
        {
            failed.append(QString("%1 [%2]").arg(q.title).arg(q.desc));
        }
    }

    return true;
}

}  // namespace Digikam