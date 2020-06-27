#include "twittermanager.h"
#include <QCoreApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QIcon>
#include <QAction>
#include "oauthlib.h"
#include "curl/curl.h"
#include <curl/cURL>
#include <QFile>
#include "twitteraccountdialogue.h"
#include "stringconvert.h"
#include "simplecrypt.h"
#include "oauthfunctions.h"
#include <QMessageBox>
#include "crypto.h"
#include "filemanagement.h"
#include "keyvaluesnode.h"
#include "simpletweet.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <jsonwindow.h>

TwitterManager::TwitterManager(QObject *parent) :
    QObject(parent), m_tc(new twitCurl()), m_pTray(NULL), m_pTrayMenu(NULL), m_pActionRefreshMentions(NULL), m_pActionQuit(NULL),
    m_szLoggedInAs(tr(NO_LOGIN)), m_pLoggedInAs(NULL), m_bLoggedIn(false), m_TweetList(), m_szLastMentionID(),
    m_pDebugWindow(NULL), m_bDebug(false)
{
    init();
}

TwitterManager::~TwitterManager()
{
    if ( m_pTray ) delete m_pTray;
}

void TwitterManager::init()
{
    // Create the tray icon.
    m_pTray = new QSystemTrayIcon();

    // Give it a proper icon.
    QIcon trayIcon(qApp->applicationDirPath() + "/tempicon.png");
    m_pTray->setIcon(trayIcon);

    // Create the debug window.
    m_pDebugWindow = new JSONWindow();
    m_pDebugWindow->setAttribute(Qt::WA_DeleteOnClose, false);

    // Create the menu.
    m_pTrayMenu = new QMenu();

    // Add in a label for who we're logged in as.
    m_pLoggedInAs = m_pTrayMenu->addAction(loggedInString());
    m_pLoggedInAs->setEnabled(false);

    m_pTrayMenu->addSeparator();

    // Create the refresh action.
    m_pActionRefreshMentions = m_pTrayMenu->addAction(tr("&Refresh mentions"));

    // Create the quit action.
    m_pActionQuit = m_pTrayMenu->addAction(tr("&Quit"));

    // Hook up signals
    connect(m_pActionQuit, SIGNAL(triggered()), QCoreApplication::instance(), SLOT(quit()));
    connect(m_pActionRefreshMentions, SIGNAL(triggered()), this, SLOT(slotRefreshMentions()));

    // Set the menu for the system tray icon.
    m_pTray->setContextMenu(m_pTrayMenu);

    // Show the system tray icon.
    m_pTray->show();
}

bool TwitterManager::login()
{
    QString username;
    QString password;
    bool remember = true;
    bool hadSavedData = true;

    // If we don't have a username or password, show the login screen.
    KeyValuesNode loginData;
    if ( !FileManagement::readKVFile(ACCOUNT_DETAILS_FILE, loginData) || loginData.isEmpty() )
    {
        hadSavedData = false;
        QString plainPW;
        TwitterAccountDialogue d;
        d.setUsernameOut(&username);
        d.setPasswordOut(&plainPW);
        d.setRememberOut(&remember);
        d.setCloseOnLogin(true);
        d.exec();

        if ( !plainPW.isEmpty() )
        {
            SimpleCrypt& crypt = standardCrypt();
            password = crypt.encryptToString(plainPW);
        }
    }
    // Read from file succeeded.
    else
    {
        username = loginData.childAt(0)->value().toString();
        password = loginData.childAt(1)->value().toString();
    }

    // If the username and password are empty, we must have closed.
    if ( username.isEmpty() && password.isEmpty() )
    {
        m_bLoggedIn = false;
        return false;
    }

    OAuthFunctions auth(*m_tc);
    auth.setUsername(username);

    SimpleCrypt& crypt = standardCrypt();
    auth.setPassword(crypt.decryptToString(password));

    // If we shouldn't save the data, delete the files.
    if ( !remember )
    {
        FileManagement::remove(ACCOUNT_DETAILS_FILE);
    }

    // If we didn't have saved data, re-save if we need to save.
    else if ( !hadSavedData )
    {
        loginData.clear();
        loginData.setKey(ACCOUNT_ROOT);
        loginData.appendChild(new KeyValuesNode(ACCOUNT_USERNAME_FIELD, QVariant(username)));
        loginData.appendChild(new KeyValuesNode(ACCOUNT_PASSWORD_FIELD, QVariant(password)));
        FileManagement::writeKVFile(ACCOUNT_DETAILS_FILE, loginData);
    }

    QString authOut;
    if ( !auth.authenticate(&authOut) )
    {
        QMessageBox::critical(NULL, "Error", authOut);
        m_bLoggedIn = false;
        return false;
    }

    m_szLoggedInAs = username;
    m_pLoggedInAs->setText(loggedInString());
    m_bLoggedIn = true;
    readLastMentionId();
    return true;
}

twitCurl& TwitterManager::tc()
{
    return *m_tc;
}

QString TwitterManager::loggedInString() const
{
    return QString("%0 %1").arg(LOGGED_IN_AS).arg(m_szLoggedInAs);
}

bool TwitterManager::loggedIn() const
{
    return m_bLoggedIn;
}

bool TwitterManager::refreshMentions()
{
    qDebug() << "Refreshing mentions with id" << m_szLastMentionID;
    if ( !loggedIn() ) return false;
    qDebug() << "Logged in.";

    // Get the ID of the last tweet we saved.
    // This should be at the top of the list.

    // Perform the get.
    tc().mentionsGet(m_szLastMentionID.toStdString());
    std::string out;
    tc().getLastWebResponse(out);

    // Generate a QJsonDocument.
    QString docString = QString::fromStdString(out);
    QJsonDocument doc = QJsonDocument::fromJson(docString.toUtf8());
    if ( m_bDebug ) showDebugwindow(doc);

    // Check the document is an array (it should be).
    if ( !doc.isArray() ) return false;
    qDebug() << "Doc is array.";

    QJsonArray array = doc.array();
    if ( array.isEmpty() ) return false;
    qDebug() << "Array is not empty.";

    // The array is non-empty. Clear our stored list of tweets and add new ones.
    deleteStoredTweets();

    for ( int i = 0; i < MAX_STORED_TWEETS && i < array.count(); i++ )
    {
        QJsonObject obj = array.at(i).toObject();
        if ( obj.isEmpty() ) continue;

        m_TweetList.append(new SimpleTweet(obj));
    }

    m_szLastMentionID = m_TweetList.at(0)->id;
    qDebug() << "About to save mention ID" << m_szLastMentionID;
    saveLatestMentionId();
    displayMentions();
}

void TwitterManager::deleteStoredTweets()
{
    // Delete all dynamically constructed tweets before we clear the list.
    for ( QList<SimpleTweet*>::iterator it = m_TweetList.begin(); it != m_TweetList.end(); it++ )
    {
        delete *it;
    }

    m_TweetList.clear();
}

void TwitterManager::displayMentions() const
{
    qDebug() << "Displaying mentions - count:" << m_TweetList.count();

    if ( m_TweetList.count() < 1 ) return;

    // If we only have one mention, display it.
    if ( m_TweetList.count() == 1 ) displayLatestMention();

    // Otherwise, display the number of mentions we have.
    // TODO: We need to have a way of taking the user to their mentions timeline on clicking the notification.
    else
    {
        if ( m_pTray ) m_pTray->showMessage("New mentions", QString("There are %0 new mentions pending.").arg(m_TweetList.count()));
    }
}

void TwitterManager::displayLatestMention() const
{
    if ( m_TweetList.count() < 1 ) return;

    SimpleTweet* tweet = m_TweetList.at(0);
    if ( m_pTray ) m_pTray->showMessage(tweet->displayName, tweet->text);
}

void TwitterManager::saveLatestMentionId() const
{
    qDebug() << "Saving mention ID" << m_szLastMentionID;
    if ( !loggedIn() ) return;

    KeyValuesNode root;

    // File does not exist - write and exit.
    if ( !FileManagement::readKVFile(LAST_MENTION_FILE, root) || root.isEmpty() )
    {
        qDebug() << "Writing new file.";
        root.setKey(LAST_MENTION_ROOT);

        KeyValuesNode* current = new KeyValuesNode(m_szLoggedInAs);
        current->appendChild(new KeyValuesNode(LAST_MENTION_KEY, QVariant(m_szLastMentionID)));
        root.appendChild(current);
        FileManagement::writeKVFile(LAST_MENTION_FILE, root);
        return;
    }

    // File already exists - find the username. If it does not exist, create it.
    KeyValuesNode* current = root.find(m_szLoggedInAs);
    if ( !current )
    {
        qDebug() << "Username does not exist, creating.";
        current = new KeyValuesNode(m_szLoggedInAs);
        root.appendChild(current);
    }

    if ( current->childCount() < 1 )
    {
        current->appendChild(new KeyValuesNode(LAST_MENTION_KEY, QVariant(QString(""))));
    }

    // Update the ID value.
    current->childAt(0)->setValue(m_szLastMentionID);

    // Write the file and exit.
    FileManagement::writeKVFile(LAST_MENTION_FILE, root);
    qDebug() << "Written.";
}

void TwitterManager::readLastMentionId()
{
    qDebug() << "Reading last mention ID.";
    if ( !loggedIn() ) return;
    qDebug() << "Logged in.";

    KeyValuesNode root;

    if ( !FileManagement::readKVFile(LAST_MENTION_FILE, root) || root.isEmpty() )
    {
        m_szLastMentionID = "";
        return;
    }

    qDebug() << "Read successful.";

    KeyValuesNode* entry = root.find(m_szLoggedInAs);
    if ( !entry || entry->childCount() < 1 )
    {
        m_szLastMentionID = "";
        return;
    }

    qDebug() << "Found node with key" << m_szLoggedInAs;

    KeyValuesNode* subentry = entry->childAt(0);
    if ( !subentry )
    {
        m_szLastMentionID = "";
        return;
    }
    qDebug() << "Found child with id.";

    m_szLastMentionID = subentry->value().toString();
    qDebug() << "ID value is:" << subentry->value().toString();
}

void TwitterManager::slotRefreshMentions()
{
    refreshMentions();
}

void TwitterManager::showDebugwindow(const QJsonDocument &doc)
{
    if ( !m_pDebugWindow ) return;

    m_pDebugWindow->readFrom(doc);
    m_pDebugWindow->show();
}

void TwitterManager::setDebug(bool enabled)
{
    m_bDebug = enabled;
}

bool TwitterManager::debug() const
{
    return m_bDebug;
}
