#ifndef TWITTERMANAGER_H
#define TWITTERMANAGER_H

#include <QObject>
#include <QScopedPointer>
#include "twitcurl.h"
#include <QList>

class QSystemTrayIcon;
class QMenu;
class QIcon;
class QAction;
class twitCurl;
class SimpleTweet;
class JSONWindow;

#define ACCOUNT_DETAILS_FILE    "account.txt"
#define ACCOUNT_ROOT            "AccountData"

#define LAST_MENTION_FILE       "mentions.txt"
#define LAST_MENTION_ROOT       "Mentions"
#define LAST_MENTION_KEY        "LastMentionId"

#define NO_LOGIN                "Not logged in"
#define LOGGED_IN_AS            "Logged in as:"
#define MAX_STORED_TWEETS       20

class TwitterManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool debug READ debug WRITE setDebug);
public:
    explicit TwitterManager(QObject *parent = 0);
    ~TwitterManager();
    bool login();
    twitCurl& tc();
    bool loggedIn() const;
    bool refreshMentions();
    bool debug() const;
    void setDebug(bool enabled);

signals:

public slots:
    void slotRefreshMentions();
    void displayLatestMention() const;
    void displayMentions() const;

private:
    typedef QScopedPointer<twitCurl> twitCurl_P;

    void init();
    QString loggedInString() const;
    void deleteStoredTweets();
    void saveLatestMentionId() const;
    void readLastMentionId();
    void showDebugwindow(const QJsonDocument &doc);

    twitCurl_P          m_tc;
    QSystemTrayIcon*    m_pTray;
    QMenu*              m_pTrayMenu;
    QAction*            m_pActionRefreshMentions;
    QAction*            m_pActionQuit;
    QString             m_szLoggedInAs;
    QAction*            m_pLoggedInAs;
    bool                m_bLoggedIn;
    QList<SimpleTweet*> m_TweetList;
    QString             m_szLastMentionID;
    JSONWindow*         m_pDebugWindow;
    bool                m_bDebug;
};

#endif // TWITTERMANAGER_H
