﻿/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include <QApplication>
#include <QUrl>

class QtLocalPeer;

class QtSingleApplication : public QApplication
{
    Q_OBJECT

public:

    QtSingleApplication(int &argc, char **argv, bool GUIenabled = true);
    QtSingleApplication(const QString &id, int &argc, char **argv);
    virtual ~QtSingleApplication();

    bool isRunning(qint64 pid = -1);

    QString id() const;

    void setActivationWindow(QWidget* aw, bool activateOnMessage = true);
    QWidget* activationWindow() const;
    bool event(QEvent *event);

    QString applicationId() const;
    void setBlock(bool value);


    void handleAction(const QUrl &pUrl)
    {
        emit messageReceived(pUrl.toString(), 0);
    }

public Q_SLOTS:

    bool sendMessage(const QString &message, int timeout = 5000, qint64 pid = -1);
    void activateWindow();

//Obsolete methods:

public:

    void initialize(bool = true);

// end obsolete methods

Q_SIGNALS:

    void messageReceived(const QString &message, QObject *socket);
    void fileOpenRequest(const QString &file);

private:

    void sysInit(const QString &_appId = QString());
    QtLocalPeer *firstPeer;
    QtLocalPeer *pidPeer;
    QWidget *actWin;
    QString appId;
    bool block;
};


