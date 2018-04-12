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

#include "config.h"

#include <QtGui>
#include <QtNetwork>
#include <QtWebEngine>
#include <QDebug>
#include "mainwindow.h"

#include <QStandardPaths>
#include <QApplication>
#include <QDesktopWidget>
#include <QtWebEngineWidgets>

// #include "cachingnm.h"
// #include "persistentcookiejar.h"

class WebView;

MainWindow::MainWindow() : QMainWindow()
{
    progress = 0;
    diskCache = NULL;
    mainSettings = NULL;

    isUrlRealyChanged = false;
    handler = new UnixSignals();
    connect(handler, SIGNAL(sigBREAK()), SLOT(unixSignalQuit()));
    connect(handler, SIGNAL(sigTERM()), SLOT(unixSignalQuit()));
    connect(handler, SIGNAL(sigINT()), SLOT(unixSignalQuit()));
    connect(handler, SIGNAL(sigHUP()), SLOT(unixSignalHup()));

    delayedResize = new QTimer();
    delayedLoad = new QTimer();

#ifdef USE_TESTLIB
    simulateClick = new QTestEventList();
#endif

}

void MainWindow::init(QCommandLineParser &options)
{

    nam = new QNetworkAccessManager(this);
    manualScreen = 0;


    defaultSettings = {
        {"application/organization", "Organization"},
        {"application/organization-domain", "www.example.com"},
        {"application/name", "Qt WebEngine Kiosk"},
        {"application/version", VERSION},
        {"application/icon", ICON },
        {"application/opengl_mode", "auto"},
        {"proxy/enable", false },
        {"proxy/system", true },
        {"proxy/host", "proxy.example.com" },
        {"proxy/port", 3128},
        {"proxy/auth", false },
        {"proxy/username", "username"},
        {"proxy/password", "password"},
        {"view/fullscreen", true},
        {"view/maximized", false},
        {"view/fixed-size", false},
        {"view/fixed-width", 800},
        {"view/fixed-height", 600},
        {"view/minimal-width", 320},
        {"view/minimal-height", 200},
        {"view/fixed-centered", true},
        {"view/fixed-x", 0},
        {"view/fixed-y", 0},
        {"view/startup_resize_delayed", true},
        {"view/startup_resize_delay", 200},
        {"view/hide_scrollbars", true},
        {"view/stay_on_top", false},
        {"view/disable_selection", true},
        {"view/show_load_progress", true},
        {"view/scale_with_dpi", true},
        {"view/page_scale", 1.0},
        {"browser/homepage", RESOURCES"default.html"},
        {"browser/javascript", true},
        {"browser/javascript_can_open_windows", false},
        {"browser/javascript_can_close_windows", false},
        {"browser/webgl", false},
        {"browser/java", false},
        {"browser/plugins", true},
        {"browser/ignore_ssl_errors", true},     // Don't break on SSL errors
        {"browser/cookiejar", false},
        {"browser/show_homepage_on_window_close", true},        // Show default homepage if window closed by javascript
        {"browser/startup_load_delayed", true},
        {"browser/startup_load_delay", 100},
        {"browser/disable_hotkeys", false},
        {"signals/enable", true},
        {"signals/SIGUSR1", ""},
        {"signals/SIGUSR2", ""},
        {"inspector/enable", false},
        {"inspector/visible", false},
        {"event-sounds/enable", false},
        {"event-sounds/window-clicked", RESOURCES"window-clicked.ogg"},
        {"event-sounds/link-clicked", RESOURCES"window-clicked.ogg"},
        {"cache/enable", false},
        {"cache/location", QStandardPaths::writableLocation(QStandardPaths::CacheLocation)},
        {"cache/size", 100*1000*1000},
        {"cache/clear-on-start", false},
        {"cache/clear-on-exit", false},
        {"printing/enable", false},
        {"printing/show-printer-dialog", false},
        {"printing/printer", "default"},
        {"printing/page_margin_left", 5},
        {"printing/page_margin_top", 5},
        {"printing/page_margin_right", 5},
        {"printing/page_margin_bottom", 5},
        {"attach/javascripts", ""},
        {"attach/styles", ""},
        {"view/hide_mouse_cursor", false}
    };

    QString location = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);

    QDir d = QDir(location);
    location += d.separator();
    location += defaultSettings["application/name"].toString();
    d.setPath(location);
    defaultSettings["cache/location"] = d.absolutePath();

    if (!(configPath = options.value("config")).isEmpty()) {
        qDebug(">> Config option in command prompt...");
    }
    loadSettings(configPath);

    QString monitorString;
    if (!(monitorString = options.value("monitor")).isEmpty()) {
        qDebug("setting monitor");
        bool ok;
        int monitorNum = monitorString.toInt(&ok);
        manualScreen = ok ? monitorNum : 0;
    }



    if (mainSettings->value("signals/enable").toBool()) {
        connect(handler, SIGNAL(sigUSR1()), SLOT(unixSignalUsr1()));
        connect(handler, SIGNAL(sigUSR2()), SLOT(unixSignalUsr2()));
    }
    handler->start();

    setMinimumWidth(320);
    setMinimumHeight(200);

    quint16 minimalWidth = mainSettings->value("view/minimal-width").toUInt();
    quint16 minimalHeight = mainSettings->value("view/minimal-height").toUInt();
    if (minimalWidth) {
        setMinimumWidth(minimalWidth);
    }
    if (minimalHeight) {
        setMinimumHeight(minimalHeight);
    }

    qDebug() << "Application icon: " << mainSettings->value("application/icon").toString();
    setWindowIcon(QIcon(
       mainSettings->value("application/icon").toString()
    ));

    QString uri;
    if (!(uri = options.value("uri")).isEmpty()) {
        qDebug(">> Uri option in command prompt...");
        mainSettings->setValue("browser/homepage", uri);
    }

    QCoreApplication::setOrganizationName(
            mainSettings->value("application/organization").toString()
            );
    QCoreApplication::setOrganizationDomain(
            mainSettings->value("application/organization-domain").toString()
            );
    QCoreApplication::setApplicationName(
            mainSettings->value("application/name").toString()
            );
    QCoreApplication::setApplicationVersion(
            mainSettings->value("application/version").toString()
            );

    // --- Network --- //

    if (mainSettings->value("proxy/enable").toBool()) {
        bool system = mainSettings->value("proxy/system").toBool();
        if (system) {
            QNetworkProxyFactory::setUseSystemConfiguration(system);
        } else {
            QNetworkProxy proxy;
            proxy.setType(QNetworkProxy::HttpProxy);
            proxy.setHostName(
                    mainSettings->value("proxy/host").toString()
            );
            proxy.setPort(mainSettings->value("proxy/port").toUInt());
            if (mainSettings->value("proxy/auth").toBool()) {
                proxy.setUser(mainSettings->value("proxy/username").toString());
                proxy.setPassword(mainSettings->value("proxy/password").toString());
            }
            QNetworkProxy::setApplicationProxy(proxy);
        }
    }

    // --- Web View --- //
    view = new WebView(this);

    if (mainSettings->value("view/show_load_progress").toBool()) {
        // --- Progress Bar --- //
        loadProgress = new QProgressBar(view);
        loadProgress->setContentsMargins(2, 2, 2, 2);
        loadProgress->setMinimumWidth(100);
        loadProgress->setMinimumHeight(16);
        loadProgress->setFixedHeight(16);
        loadProgress->setAutoFillBackground(true);
        QPalette palette = this->palette();
        palette.setColor(QPalette::Window, QColor(255,255,255,63));
        loadProgress->setPalette(palette);

        // Do not work... Need Layout...
        loadProgress->setAlignment(Qt::AlignTop);
        loadProgress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        loadProgress->hide();
    }

    if (mainSettings->value("view/hide_mouse_cursor").toBool()) {
        QApplication::setOverrideCursor(Qt::BlankCursor);
    }

    setCentralWidget(view);

    view->setSettings(mainSettings);
    view->setPage(new QWebEnginePage(view));
    view->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled,
        mainSettings->value("browser/javascript").toBool()
    );

    view->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows,
        mainSettings->value("browser/javascript_can_open_windows").toBool()
    );



    connect(view, SIGNAL(titleChanged(QString)), SLOT(adjustTitle()));
    connect(view, SIGNAL(loadStarted()), SLOT(startLoading()));
    connect(view, SIGNAL(urlChanged(const QUrl &)), SLOT(urlChanged(const QUrl &)));
    connect(view, SIGNAL(loadProgress(int)), SLOT(setProgress(int)));
    connect(view, SIGNAL(loadFinished(bool)), SLOT(finishLoading(bool)));
    connect(view, SIGNAL(iconChanged()), SLOT(pageIconLoaded()));

    QDesktopWidget *desktop = QApplication::desktop();
    connect(desktop, SIGNAL(resized(int)), SLOT(desktopResized(int)));

    shortcutKeys = {
        //{Qt::Key_Up, [](){view->scrollUp();},
        //{Qt::Key_Down, [](){view->scrollDown();},
        //{Qt::Key_PageUp, [](){view->scrollPageUp();},
        //{Qt::Key_PageDown, [](){view->scrollPageDown();},
        //{Qt::Key_End, [](){view->scrollEnd();},
        {
            Qt::Key_HomePage,
            [this](){view->loadHomepage();}
        },
        //{Qt::Key_Home, [](){ view->scrollHome(); }},
        {
            Qt::Key_Home + Qt::CTRL,
            [this](){view->loadHomepage();}
        },
        {
            QKeySequence::Back,
            [this](){view->page()->triggerAction(QWebEnginePage::Back);}
        },
        {
            QKeySequence::Forward,
            [this](){view->page()->triggerAction(QWebEnginePage::Forward);}
        },
        {
            QKeySequence::Quit,
            [this]() {
                clearCacheOnExit();
                QApplication::exit(0);
            }
        },
        {
            QKeySequence(Qt::Key_F5 + Qt::CTRL, Qt::Key_R+Qt::CTRL+Qt::SHIFT),
            [this]() {
                clearCache();
                view->page()->triggerAction(QWebEnginePage::ReloadAndBypassCache);
            }
        },
        {
            QKeySequence::Refresh,
            [this]() {view->reload();}
        },
        /*{
            Qt::Key_F12,
            [this]() {
                if (mainSettings->value("inspector/enable").toBool()) {
                    if (!inspector->isVisible()) {
                        inspector->setVisible(true);
                    } else {
                        inspector->setVisible(false);
                    }
                }
            }
        },*/
        {
            QKeySequence::FullScreen,
            [this]() {
                if (isFullScreen()) {
                    showNormal();
                } else {
                    showFullScreen();
                }
            }
        }
    };

    auto i = shortcutKeys.constBegin();
    while (i != shortcutKeys.constEnd()) {
        QAction* tempAction = new QAction(this);
        tempAction->setShortcut(i.key());
        tempAction->setShortcutContext(Qt::ApplicationShortcut);
        connect(tempAction, &QAction::triggered,
                i.value());
        this->addAction(tempAction);
        ++i;
    }



    show();
    view->page()->view()->setFocusPolicy(Qt::StrongFocus);
    view->setFocusPolicy(Qt::StrongFocus);

    int delay_resize = 0;
    if (mainSettings->value("view/startup_resize_delayed").toBool()) {
        delay_resize = mainSettings->value("view/startup_resize_delay").toInt();
    }
    delayedResize->singleShot(delay_resize, this, SLOT(delayedWindowResize()));

    int delay_load = 0;
    if (mainSettings->value("browser/startup_load_delayed").toBool()) {
        delay_load = mainSettings->value("browser/startup_load_delay").toInt();
    }
    delayedLoad->singleShot(delay_load, this, SLOT(delayedPageLoad()));

}

void MainWindow::delayedWindowResize()
{

    if (mainSettings->value("view/fullscreen").toBool()) {
        showFullScreen();
    } else if (mainSettings->value("view/maximized").toBool()) {
        showMaximized();
    } else if (mainSettings->value("view/fixed-size").toBool()) {
        centerFixedSizeWindow();
    }

    this->setFocusPolicy(Qt::StrongFocus);
    this->focusWidget();

    if (mainSettings->value("view/stay_on_top").toBool()) {
        setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    }
}

void MainWindow::delayedPageLoad()
{
    view->loadHomepage();
}

void MainWindow::clearCache()
{
    if (mainSettings->value("cache/enable").toBool()) {
        if (diskCache) {
            diskCache->clear();
        }
    }
}

void MainWindow::clearCacheOnExit()
{
    if (mainSettings->value("cache/enable").toBool()) {
        if (mainSettings->value("cache/clear-on-exit").toBool()) {
            if (diskCache) {
                diskCache->clear();
            }
        }
    }
}

void MainWindow::cleanupSlot()
{
    qDebug("Cleanup Slot (application exit)");
    handler->stop();
    clearCacheOnExit();
    //QWebEngineSettings::clearMemoryCaches();
}

void MainWindow::showFullScreen() {

    int screen = computedScreen();
    if (screen >= 0) {
        if (this->windowHandle()) {
            this->windowHandle()->setScreen(qApp->screens()[screen]);
        }
        QRect screenGeometry = qApp->desktop()->availableGeometry(screen);
        setGeometry(screenGeometry);
        view->updateZoom();
        qDebug() << "setting geometry:" << screenGeometry;
    }

    QMainWindow::showFullScreen();
    updatePixelRatio();
}

void MainWindow::moveEvent(QMoveEvent *) {
    updatePixelRatio();
}

void MainWindow::updatePixelRatio() {
    qreal _pixelRatio = (windowHandle()->screen()->logicalDotsPerInch()/96.0);
    if (pixelRatio != _pixelRatio) {
        pixelRatio = _pixelRatio;
        view->updateZoom();
        qDebug() << "pixel ratio set to: " << pixelRatio;
    }
}

qreal MainWindow::getPixelRatio() {
    return pixelRatio;
}

int MainWindow::computedScreen() {
    const QList<QScreen*> screens = qApp->screens();
    int numScreens = screens.size();
    if (manualScreen >= numScreens) {
        qDebug() << "invalid monitor" << manualScreen << ", you only have " << numScreens << "screens.";
        return -1;
    } else {
        qDebug() << "setting screen" << manualScreen+1 << "/" << numScreens;
        return manualScreen;
    }
}

void MainWindow::centerFixedSizeWindow()
{
    quint16 widowWidth = mainSettings->value("view/fixed-width").toUInt();
    quint16 widowHeight = mainSettings->value("view/fixed-height").toUInt();

    int screen = computedScreen();
    QRect screenRect = qApp->desktop()->screenGeometry(screen);
    quint16 screenWidth = screenRect.width();
    quint16 screenHeight = screenRect.height();
    QPoint screenOrigin = screenRect.topLeft();

    qDebug() << "Screen size: " << screenWidth << "x" << screenHeight;

    quint16 x = 0;
    quint16 y = 0;

    if (mainSettings->value("view/fixed-centered").toBool()) {
        x = screenOrigin.x() + (screenWidth - widowWidth) / 2;
        y = screenOrigin.y() + (screenHeight - widowHeight) / 2;
    } else {
        x = mainSettings->value("view/fixed-x").toUInt();
        y = mainSettings->value("view/fixed-y").toUInt();
    }

    qDebug() << "Move window to: (" << x << ";" << y << ")";

    move ( x, y );
    setFixedSize( widowWidth, widowHeight );
    updatePixelRatio();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (mainSettings->value("browser/disable_hotkeys").toBool()) {
        QMainWindow::keyPressEvent(event);
        return;
    }
    qDebug() << "got key: " << event->key();

}

/**
 * @TODO: move to separate class
 *
 * @brief MainWindow::loadSettings
 * @param ini_file
 */
void MainWindow::loadSettings(QString ini_file)
{
    if (mainSettings != NULL) {
        delete mainSettings;
    }
    if (!ini_file.length()) {
        mainSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "QtWebkitKiosk", "config", this);
    } else {
        mainSettings = new QSettings(ini_file, QSettings::IniFormat, this);
    }
    qDebug() << "Ini file: " << mainSettings->fileName();

    QHash<QString, QVariant>::const_iterator i = defaultSettings.constBegin();
    while (i != defaultSettings.constEnd()) {
        if (!mainSettings->contains(i.key())) {
            mainSettings->setValue(i.key(),i.value());
        }
        i++;
    }

    mainSettings->sync();
}

void MainWindow::adjustTitle()
{
    if (progress <= 0 || progress >= 100) {
        setWindowTitle(view->title());
    } else {
        setWindowTitle(QString("%1 (%2%)").arg(view->title()).arg(progress));
    }
}

void MainWindow::desktopResized(int p)
{
    qDebug() << "Desktop resized event: " << p;
    if (mainSettings->value("view/fullscreen").toBool()) {
        showFullScreen();
    } else if (mainSettings->value("view/maximized").toBool()) {
        showMaximized();
    } else if (mainSettings->value("view/fixed-size").toBool()) {
        centerFixedSizeWindow();
    }
}

void MainWindow::startLoading()
{
    progress = 0;
    isScrollBarsHidden = false;
    isSelectionDisabled = false;
    isUrlRealyChanged = false;

    adjustTitle();

    //QWebEngineSettings::clearMemoryCaches();

    if (mainSettings->value("view/show_load_progress").toBool()) {
        loadProgress->show();
    }

    qDebug("Start loading...");
}

void MainWindow::setProgress(int p)
{
    progress = p;
    adjustTitle();

    qDebug() << "Loading progress: " << p;

    if (mainSettings->value("view/show_load_progress").toBool()) {
        loadProgress->setValue(p);
    }


}

void MainWindow::urlChanged(const QUrl &url)
{
    qDebug() << "URL changes: " << url.toString();

    // Where is a real change in webframe! Drop flags.
    isScrollBarsHidden = false;
    isSelectionDisabled = false;
    isUrlRealyChanged = true;

    view->updateZoom();

    // This is real link clicked
    view->playSound("event-sounds/link-clicked");
}

void MainWindow::finishLoading(bool)
{
    qDebug("Finish loading...");

    progress = 100;
    adjustTitle();

    if (mainSettings->value("view/show_load_progress").toBool()) {
        loadProgress->hide();
    }


    // 2. Add more styles which can override previous styles...
    attachStyles();
    attachJavascripts();

    // 3. Focus window and click into it to stimulate event loop after signal handling
    putWindowUp();
}


void MainWindow::attachJavascripts()
{
    //TODO: use runJavascript()
    return;
}

void MainWindow::attachStyles()
{
    //TODO: probably impossible :(
    return;
}


void MainWindow::pageIconLoaded()
{
    setWindowIcon(view->icon());
}

// ----------------------- SIGNALS -----------------------------

/**
 * Force quit on Unix SIGTERM or SIGINT signals
 * @brief MainWindow::unixSignalQuit
 */
void MainWindow::unixSignalQuit()
{
    // No cache clean - quick exit
    qDebug(">> Quit Signal catched. Exiting...");
    QApplication::exit(0);
}

/**
 * Activate window after signal
 */
void MainWindow::putWindowUp()
{
    qDebug("Try to activate window...");

    QApplication::setActiveWindow(this);
    this->focusWidget();

#ifdef USE_TESTLIB
    qDebug("... by click simulation...");
    simulateClick->clear();
    simulateClick->addMouseClick(Qt::LeftButton, 0, this->pos(), -1);
    simulateClick->simulate(this);
#endif

}

/**
 * Do something on Unix SIGHUP signal
 * Usualy:
 *  1. Reload config
 *
 * @brief MainWindow::unixSignalHup
 */
void MainWindow::unixSignalHup()
{
    loadSettings(configPath);
}

/**
 * Do something on Unix SIGUSR1 signal
 * Usualy:
 *  1. Reload config and load home page URI
 *  2. If option 'signals/SIGUSR1' defined and not empty - try to load defined URI
 *
 * @brief MainWindow::unixSignalUsr1
 */
void MainWindow::unixSignalUsr1()
{
    if (mainSettings->contains("signals/SIGUSR1") && !mainSettings->value("signals/SIGUSR1").toString().isEmpty()) {
        qDebug(">> SIGUSR1 >> Load URI from config file...");
        view->loadCustomPage(mainSettings->value("signals/SIGUSR1").toString());
    } else {
        qDebug(">> SIGUSR1 >> Load config file...");
        loadSettings(configPath);
        view->loadHomepage();
    }
}

/**
 * Do something on Unix SIGUSR2 signal
 * Usualy:
 *  1. Load home page URI
 *  2. If option 'signals/SIGUSR2' defined and not empty - try to load defined URI
 *
 * @brief MainWindow::unixSignalUsr2
 */
void MainWindow::unixSignalUsr2()
{
    if (mainSettings->contains("signals/SIGUSR2") && !mainSettings->value("signals/SIGUSR2").toString().isEmpty()) {
        qDebug(">> SIGUSR2 >> Load URI from config file...");
        view->loadCustomPage(mainSettings->value("signals/SIGUSR2").toString());
    } else {
        qDebug(">> SIGUSR2 >> Load homepage URI...");
        view->loadHomepage();
    }
}
