#ifndef SIMPLETWEET_H
#define SIMPLETWEET_H

#include <QObject>
#include <QString>
#include <QJsonObject>

// For finding relevant values from the QJsonObject.
#define KEY_ID          "id_str"
#define KEY_DISPLAYNAME "name"
#define KEY_ACCOUNTNAME "screen_name"
#define KEY_TEXT        "text"
#define KEY_USER_SUBKEY "user"

class SimpleTweet : public QObject
{
    Q_OBJECT
public:
    explicit SimpleTweet(QObject *parent = 0);
    explicit SimpleTweet(const QJsonObject &obj);

    QString id;
    QString displayName;
    QString accountName;
    QString text;

    void fromTweetObject(const QJsonObject &obj);

signals:

public slots:

};

#endif // SIMPLETWEET_H
