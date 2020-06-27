#include "oauthfunctions.h"
#include "twitcurl.h"
#include <curl/cURL>
#include <QFile>
#include <QTextStream>
#include "stringconvert.h"
#include "usergetpindialogue.h"
#include "simplecrypt.h"
#include "crypto.h"
#include <QtDebug>
#include "keyvaluesnode.h"
#include "filemanagement.h"

OAuthFunctions::OAuthFunctions(twitCurl &tc, QObject *parent) :
    QObject(parent), m_tc(tc), m_Account(), m_Consumer(), m_Auth(), m_bHandlePin(false)
{
    init();
}

void OAuthFunctions::init()
{
    // Create username and password fields for the account details.
    m_Account.appendChild(new KeyValuesNode(QString(ACCOUNT_USERNAME_FIELD), QVariant(QString(""))));
    m_Account.appendChild(new KeyValuesNode(QString(ACCOUNT_PASSWORD_FIELD), QVariant(QString(""))));
    
    // Create key and secret fields for the consumer details.
    m_Consumer.appendChild(new KeyValuesNode(QString(KEY_FIELD), QVariant(QString(""))));
    m_Consumer.appendChild(new KeyValuesNode(QString(SECRET_FIELD), QVariant(QString(""))));
    
    // Create key and secret fields for the auth details.
    m_Auth.appendChild(new KeyValuesNode(QString(KEY_FIELD), QVariant(QString(""))));
    m_Auth.appendChild(new KeyValuesNode(QString(SECRET_FIELD), QVariant(QString(""))));
}

bool OAuthFunctions::dataValid()
{
    return !(
                username().isEmpty() ||
                password().isEmpty()
            );
}

bool OAuthFunctions::authenticate(QString* out)
{
    // Check required data is valid.
    if ( !dataValid() )
    {
        if ( out )
        {
            *out = QString("Username or password not provided.");
        }

        return false;
    }

    // Set the username and password.
    std::string temp = toString(username());
    m_tc.setTwitterUsername(temp);
    temp = toString(password());
    m_tc.setTwitterPassword(temp);

    //qDebug() << "Username for authentication:" << m_szUsername << "Password:" << m_szPassword;

    // Todo: Set up proxy server here!

    // Begin OAuth!

    // Set consumer key and secret.
    FileManagement::readKVFile(CONSUMER_DETAILS_FILE, m_Consumer);
    if ( consumerKey().isEmpty() || consumerSecret().isEmpty() )
    {
        if ( out )
        {
            *out = QString("Consumer key details not found.");
        }

        return false;
    }

    m_tc.getOAuth().setConsumerKey(toString(consumerKey()));
    SimpleCrypt& crypt = standardCrypt();
    m_tc.getOAuth().setConsumerSecret(toString(crypt.decryptToString(consumerSecret())));

    //qDebug() << "Consumer key:" << m_szConsumerKey << "Consumer secret:" << crypt.decryptToString(m_szConsumerSecret);

    // If we already have auth variables, just set them as needed.
    if ( FileManagement::readKVFile(AUTH_DETAILS_FILE, m_Auth) && !m_Auth.isEmpty() )
    {
        m_tc.getOAuth().setOAuthTokenKey(toString(authKey()));
        m_tc.getOAuth().setOAuthTokenSecret(toString(crypt.decryptToString(authSecret())));
        //qDebug() << "Auth key:" << m_szAuthKey << "Auth secret:" << m_szAuthSecret;

        // Verify we authenticated properly.
        if ( !m_tc.accountVerifyCredGet() )
        {
            if ( out )
            {
                std::string er;
                m_tc.getLastCurlError(er);
                *out = QString::fromStdString(er);
                //qDebug() << "Authentication error:" << *out;
            }

            return false;
        }

        if ( out )
        {
            std::string response;
            m_tc.getLastWebResponse(response);
            *out = QString::fromStdString(response);
            //qDebug() << "Authentication response:" << *out;
        }

        return true;
    }

    // Otherwise, get the variables.
    std::string url;
    m_tc.oAuthRequestToken(url);

    // If the PIN is handled automatically, do this now.
    if ( m_bHandlePin )
    {
        m_tc.oAuthHandlePIN(url);
    }
    else
    {
        // Construct a modal window that will return the PIN.
        QString pin = askUserForPin(QString::fromStdString(url));
        m_tc.getOAuth().setOAuthPin(toString(pin));
        //qDebug("PIN returned from user: %s", pin.toLatin1().constData());
    }

    // Exchange request token with access token.
    m_tc.oAuthAccessToken();

    // Save key and secret for later use.
    m_Auth.setKey(AUTH_ROOT);
    temp.clear();
    m_tc.getOAuth().getOAuthTokenKey(temp);
    setAuthKey(QString::fromStdString(temp));
    m_tc.getOAuth().getOAuthTokenSecret(temp);
    setAuthSecret(crypt.encryptToString(QString::fromStdString(temp)));
    //qDebug() << "Auth key:" << m_szAuthKey << "Auth secret:" << QString::fromStdString(temp);
    //qDebug() << "Encrypted auth key:" << m_szAuthSecret;
    FileManagement::writeKVFile(AUTH_DETAILS_FILE, m_Auth);

    // End OAuth!

    // Verify we authenticated properly.
    if ( !m_tc.accountVerifyCredGet() )
    {
        if ( out )
        {
            std::string er;
            m_tc.getLastCurlError(er);
            *out = QString::fromStdString(er);
            //qDebug() << "Authentication error:" << *out;
        }

        return false;
    }

    if ( out )
    {
        std::string response;
        m_tc.getLastWebResponse(response);
        *out = QString::fromStdString(response);
        //qDebug() << "Authentication response:" << *out;
    }

    return true;
}

QString OAuthFunctions::askUserForPin(const QString &url) const
{
    QString pin;
    UserGetPinDialogue dialogue;
    dialogue.setUrl(url);
    dialogue.setPinOut(&pin);
    dialogue.exec();

    return pin;
}
