#ifndef WEBVIEW_H
#define WEBVIEW_H


#include <QtWebEngineWidgets/QWebEnginePage>
#include <QtWebEngineWidgets/QWebEngineView>

#include <QPrinter>
#include <QIcon>

#include <qplayer.h>
#include <fakewebview.h>

//#include "mainwindow.h"
class MainWindow;

class WebView : public QWebEngineView
{
    Q_OBJECT

public:
    explicit WebView(MainWindow* parent = 0);

    void setSettings(QSettings *settings);
    void loadCustomPage(QString uri);
    void loadHomepage();
    void initSignals();

    void setPage(QWebEnginePage* page);

    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type);

    void playSound(QString soundSetting);
    void updateZoom();

    QIcon icon();


public slots:
#ifdef PRINTING_POSSIBLE
    void handlePrintRequested(QWebEnginePage *);
#endif
    void handleUrlChanged(const QUrl &);

signals:
    void iconChanged();

protected:
    void mousePressEvent(QMouseEvent *event);
    QPlayer *getPlayer();

private:
    QPlayer *player;
    QSettings *mainSettings;
    FakeWebView *loader;
    QPrinter *printer;

    QIcon pageIcon;
    QNetworkReply* pageIconReply;

private slots:
    void handleWindowCloseRequested();
    void onIconLoaded();
    void onIconUrlChanged(const QUrl&);

};

#endif // WEBVIEW_H
