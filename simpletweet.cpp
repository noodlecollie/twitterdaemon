#include "simpletweet.h"

SimpleTweet::SimpleTweet(QObject *parent) :
    QObject(parent), id(), displayName(), accountName(), text()
{
}

SimpleTweet::SimpleTweet(const QJsonObject &obj) :
    QObject(NULL), id(), displayName(), accountName(), text()
{
    fromTweetObject(obj);
}

void SimpleTweet::fromTweetObject(const QJsonObject &obj)
{
    // We need:
    // id_str (id)
    // user->name (displayName)
    // user->screen_name (accountName)
    // text (text)

    if ( obj.contains(KEY_ID) ) id = obj.find(KEY_ID).value().toString();
    if ( obj.contains(KEY_TEXT) ) text = obj.find(KEY_TEXT).value().toString();

    if ( !obj.contains(KEY_USER_SUBKEY) ) return;
    QJsonObject user = obj.find(KEY_USER_SUBKEY).value().toObject();
    if ( user.isEmpty() ) return;

    if ( user.contains(KEY_DISPLAYNAME) ) displayName = user.find(KEY_DISPLAYNAME).value().toString();
    if ( user.contains(KEY_ACCOUNTNAME) ) accountName = user.find(KEY_ACCOUNTNAME).value().toString();
}
