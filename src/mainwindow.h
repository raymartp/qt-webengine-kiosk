/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include <functional>
#include <QtGui>
#include <QProgressBar>
#include <QMainWindow>
#include <QtNetwork>
#include <QtWebEngine>

#ifdef USE_TESTLIB
#include <QtTest/QTestEventList>
#endif

//#include <QtWebEngineWidgets/QWebInspector>

#include "webview.h"
#include "unixsignals.h"

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#define qsl(...) QStringList({__VA_ARGS__})

struct keyFunction {
    Qt::Key key;
    std::function<void()> f;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_ENUMS(GlMode)

public:
    explicit MainWindow();

    void init(QCommandLineParser&);
    enum GlMode {  AUTO, NATIVE, ANGLE, SOFTWARE };

    void clearCache();
    void clearCacheOnExit();

    void showFullScreen();
    qreal getPixelRatio();
    QNetworkAccessManager *nam;

protected slots:

    void cleanupSlot();

    void adjustTitle();
    void setProgress(int p);
    void startLoading();
    void urlChanged(const QUrl &);
    void finishLoading(bool);
    void pageIconLoaded();

    void desktopResized(int p);

    void delayedWindowResize();
    void delayedPageLoad();


    // TERM or INT - Quit from App
    void unixSignalQuit();
    // HUP - Reload config
    void unixSignalHup();
    // USR1 - Reload config and back to homepage, or load page from config
    void unixSignalUsr1();
    // USR2 - Back to homepage, or load page from config
    void unixSignalUsr2();

protected:

    void moveEvent(QMoveEvent *);
    void centerFixedSizeWindow();
    void attachJavascripts();
    void attachStyles();
    void putWindowUp();
    void keyPressEvent(QKeyEvent *event);
    void updatePixelRatio();


private:
    WebView* view;                      // Webkit Page View
    QProgressBar *loadProgress;         // progress bar to display page loading

    QHash<QString, QVariant> defaultSettings;
    QSettings *mainSettings;
    QNetworkDiskCache *diskCache;
    //QWebInspector *inspector;

    QMap<QKeySequence, std::function<void()>> shortcutKeys;
    QKeyEvent * eventExit;

    UnixSignals *handler;
    int manualScreen;
    int computedScreen();
    qreal pixelRatio;

#ifdef USE_TESTLIB
    QTestEventList *simulateClick;
#endif

    int progress;
    bool isScrollBarsHidden;
    bool isSelectionDisabled;
    bool isUrlRealyChanged;

    QString configPath;
    void loadSettings(QString ini_file);

    QTimer *delayedResize;
    QTimer *delayedLoad;
};

#endif
