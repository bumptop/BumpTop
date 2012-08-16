/*  This file is part of the KDE project.

Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).

This library is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 or 3 of the License.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef PHONON_MMF_DOWNLOAD_H
#define PHONON_MMF_DOWNLOAD_H

#include <QtCore/QMetaType>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <downloadmgrclient.h>

QT_FORWARD_DECLARE_CLASS(QByteArray)
QT_FORWARD_DECLARE_CLASS(QFile)

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace MMF
{

class Download;

class DownloadPrivate : public QObject
                      , public MHttpDownloadMgrObserver
{
    Q_OBJECT
public:
    DownloadPrivate(Download *parent);
    ~DownloadPrivate();
    bool start(int iap);
    void resume();
signals:
    void error();
    void targetFileNameChanged();
    void lengthChanged(qint64 length);
    void complete();
private:
    // MHttpDownloadMgrObserver
    void HandleDMgrEventL(RHttpDownload &aDownload, THttpDownloadEvent aEvent);
private:
    Download *m_parent;
    RHttpDownloadMgr m_downloadManager;
    RHttpDownload *m_download;
    qint64 m_length;
};

class Download : public QObject
{
    Q_OBJECT
    friend class DownloadPrivate;
public:
    Download(const QUrl &url, QObject *parent = 0);
    ~Download();
    const QUrl &sourceUrl() const;
    const QString &targetFileName() const;
    void start(int iap);
    void resume();

    enum State {
        Idle,
        Initializing,
        Downloading,
        Complete,
        Error
    };

signals:
    void lengthChanged(qint64 length);
    void stateChanged(Download::State state);

private:
    void setState(State state);

    // Called by DownloadPrivate
    void error();
    void downloadStarted(const QString &targetFileName);
    void complete();

private:
    DownloadPrivate *m_private;
    QUrl m_sourceUrl;
    QString m_targetFileName;
    State m_state;
};

}
}

QT_END_NAMESPACE

Q_DECLARE_METATYPE(Phonon::MMF::Download::State)

#endif
