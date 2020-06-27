#ifndef OAUTHFUNCTIONS_H
#define OAUTHFUNCTIONS_H

#include <QObject>
#include "filemanagement.h"
#include "keyvaluesnode.h"

class twitCurl;

#define CONSUMER_DETAILS_FILE   "consumer.txt"
#define AUTH_DETAILS_FILE       "auth.txt"

#define AUTH_ROOT               "AuthData"
#define CONSUMER_ROOT           "ConsumerData"
#define ACCOUNT_USERNAME_FIELD  "username"
#define ACCOUNT_PASSWORD_FIELD  "password"
#define KEY_FIELD               "key"
#define SECRET_FIELD            "secret"

class OAuthFunctions : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString username READ username WRITE setUsername)
    Q_PROPERTY(QString password READ password WRITE setPassword)
    Q_PROPERTY(QString consumerKey READ consumerKey WRITE setConsumerKey)
    Q_PROPERTY(QString consumerSecret READ consumerSecret WRITE setConsumerSecret)
    Q_PROPERTY(QString authKey READ authKey WRITE setAuthKey)
    Q_PROPERTY(QString authSecret READ authSecret WRITE setAuthSecret)
public:
    explicit OAuthFunctions(twitCurl &tc, QObject *parent = 0);

    QString consumerKey() const { return m_Consumer.childAt(0)->value().toString(); }
    QString consumerSecret() const { return m_Consumer.childAt(1)->value().toString(); }
    QString username() const { return m_Account.childAt(0)->value().toString(); }
    QString password() const { return m_Account.childAt(1)->value().toString(); }
    QString authKey() const { return m_Auth.childAt(0)->value().toString(); }
    QString authSecret() const { return m_Auth.childAt(1)->value().toString(); }

signals:

public slots:
    bool authenticate(QString* out = NULL);
    void setConsumerKey(const QString &key) { m_Consumer.childAt(0)->setValue(key); }
    void setConsumerSecret(const QString &secret) { m_Consumer.childAt(1)->setValue(secret); }
    void setUsername(const QString &str) { m_Account.childAt(0)->setValue(str); }
    void setPassword(const QString &str) { m_Account.childAt(1)->setValue(str); }
    void setAuthKey(const QString &str) { m_Auth.childAt(0)->setValue(str); }
    void setAuthSecret(const QString &str) { m_Auth.childAt(1)->setValue(str); }

private:
    void init();
    bool dataValid();
    QString askUserForPin(const QString &url) const;

    twitCurl&       m_tc;               // twitCurl object we're authenticating for.
    KeyValuesNode   m_Account;
    KeyValuesNode   m_Consumer;
    KeyValuesNode   m_Auth;
    bool            m_bHandlePin;       // twitCurl handles PIN automatically
};

#endif // OAUTHFUNCTIONS_H
