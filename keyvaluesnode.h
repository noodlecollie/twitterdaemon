#ifndef KEYVALUESNODE_H
#define KEYVALUESNODE_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QList>
#include <QDebug>
#include <QDataStream>
#include <QPair>

#define KV_BOPEN    "{"
#define KV_CBOPEN   '{'
#define KV_BCLOSE   "}"
#define KV_CBCLOSE  '}'

// If a KeyValuesNode has children, its value should be ignored when writing to a file.
class KeyValuesNode : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString key READ key WRITE setKey)
    Q_PROPERTY(QVariant value READ value WRITE setValue)
public:
    explicit KeyValuesNode(const QString &key, const QVariant &value, KeyValuesNode* parent = NULL);
    explicit KeyValuesNode(const QString &key, KeyValuesNode* parent = NULL);
    explicit KeyValuesNode(const QPair<QString, QString> &pair, KeyValuesNode* parent = NULL);
    explicit KeyValuesNode(KeyValuesNode* parent = NULL);
    ~KeyValuesNode();

    QString key() const;
    QVariant value() const;
    int childCount() const;
    KeyValuesNode* childAt(int i) const;
    void addChild(int at, KeyValuesNode* node);
    void appendChild(KeyValuesNode* node);
    KeyValuesNode* removeChild(int at);
    void clear();               // Removes key, value and all children recursively.
    void deleteChildren();      // Only removes children recursively.
    QList<KeyValuesNode*>& childNodes();
    bool isRoot();
    KeyValuesNode* parentNode() const;
    bool writeKey() const;
    void setWriteKey(bool b);
    bool isEmpty() const;
    bool contains(const QString &key) const;
    KeyValuesNode* find(const QString &key) const;
    
    // T must implement operator <<.
    template<typename T>
    T& writeToStream(T &stream, const QString &tab = "\t", int depth = 0) const;
    void constructFrom(const QString &input);

signals:

public slots:
    void setKey(const QString &key);
    void setKey(const char* key);
    void setValue(const QVariant &value);
    void setValue(const QString &value);
    void setValue(const char* value);
    void setValue(int value);
    void setValue(float value);
    void setValue(bool value);

private:
    typedef QList<KeyValuesNode*> KVList;
    typedef QPair<QString, QString> KVPair;
    int readFrom(const QString &input);
    static int getToken(const QString &input, QString &tokenOut);
    static int getKeyAndValue(const QString &input, KVPair &pair);
    static bool isInvalid(const KVPair &pair);
    static bool isLeaf(const KVPair &pair);
    static bool isSectionCreator(const KVPair &pair);
    static bool isSectionTerminator(const KVPair &pair);

    QString         m_szKey;
    QVariant        m_vValue;
    KVList          m_Children;
    bool            m_bWriteKey;
};

QDataStream& operator<<(QDataStream &stream, const KeyValuesNode &node);
QTextStream& operator<<(QTextStream &stream, const KeyValuesNode &node);

#endif // KEYVALUESNODE_H
