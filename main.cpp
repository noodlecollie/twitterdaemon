#include <QApplication>
#include "twittermanager.h"
#include <QtDebug>
#include <QStringList>
#include "simplecrypt.h"
#include "crypto.h"
#include "oauthfunctions.h"
#include <QFile>
#include "twitcurl.h"
#include <QSystemTrayIcon>

void parseCommandLineArguments(const QStringList &list);
void generateConsumerFile(const QString &key, const QString &secret);
bool DEBUG = false;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    // Check command line arguments.
    if ( argc > 1 )
    {
        parseCommandLineArguments(a.arguments());
    }
    
    // Check whether we can create a system tray icon.
    if ( !QSystemTrayIcon::isSystemTrayAvailable() )
    {
        qFatal("No support for system tray applications.");
        return 1;
    }

    // Create TwitterManager object.
    TwitterManager tw;
    tw.setDebug(DEBUG);

    // Attempt to log in. This should ask for the username and password
    // if they aren't already saved. If it returns false, exit.
    if ( !tw.login() )
    {
        qDebug() << "Login failed.";
        return 1;
    }

    qDebug() << "Login succeeded.";

    tw.refreshMentions();

    return a.exec();
}

void parseCommandLineArguments(const QStringList &list)
{
    for ( int i = 0; i < list.count(); i++ )
    {
        if ( list.at(i) == "--consumer-details" )
        {
            int j = i + 1;
            int k = i + 2;
            if ( j >= list.count() || k >= list.count() ) continue;

            generateConsumerFile(list.at(j), list.at(k));
        }

        else if ( list.at(i) == "--debug" )
        {
            DEBUG = true;
        }
    }
}

void generateConsumerFile(const QString &key, const QString &secret)
{
    qDebug() << "Encrypting consumer details to" << CONSUMER_DETAILS_FILE;
    SimpleCrypt& crypt = standardCrypt();
    QString secret_encrypted(crypt.encryptToString(secret));

    KeyValuesNode root(QString(CONSUMER_ROOT));
    root.appendChild(new KeyValuesNode(KEY_FIELD, QVariant(key)));
    root.appendChild(new KeyValuesNode(SECRET_FIELD, QVariant(secret_encrypted)));
    FileManagement::writeKVFile(CONSUMER_DETAILS_FILE, root);
}
