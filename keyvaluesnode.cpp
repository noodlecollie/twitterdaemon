#include "keyvaluesnode.h"

KeyValuesNode::KeyValuesNode(KeyValuesNode *parent) :
    QObject(parent), m_szKey(), m_vValue(), m_Children(), m_bWriteKey(true)
{
}

KeyValuesNode::KeyValuesNode(const QString &key, KeyValuesNode *parent) :
    QObject(parent), m_szKey(key), m_vValue(), m_Children(), m_bWriteKey(true)
{
}

KeyValuesNode::KeyValuesNode(const QPair<QString, QString> &pair, KeyValuesNode *parent) :
    QObject(parent), m_szKey(pair.first), m_vValue(QVariant(pair.second)), m_Children(), m_bWriteKey(true)
{
}

KeyValuesNode::KeyValuesNode(const QString &key, const QVariant &value, KeyValuesNode *parent) :
    QObject(parent), m_szKey(key), m_vValue(value), m_Children(), m_bWriteKey(true)
{
}

KeyValuesNode::~KeyValuesNode()
{
    deleteChildren();
}

int KeyValuesNode::childCount() const
{
    return m_Children.count();
}

QList<KeyValuesNode*>& KeyValuesNode::childNodes()
{
    return m_Children;
}

KeyValuesNode* KeyValuesNode::parentNode() const
{
    return dynamic_cast<KeyValuesNode*>(parent());
}

KeyValuesNode* KeyValuesNode::childAt(int i) const
{
    return m_Children.at(i);
}

void KeyValuesNode::addChild(int at, KeyValuesNode *node)
{
    m_Children.insert(at, node);
    node->setParent(this);
}

void KeyValuesNode::appendChild(KeyValuesNode *node)
{
    m_Children.append(node);
    node->setParent(this);
}

KeyValuesNode* KeyValuesNode::removeChild(int at)
{
    KeyValuesNode* node = m_Children.takeAt(at);
    node->setParent(NULL);
    return node;
}

void KeyValuesNode::deleteChildren()
{
    for ( int i = m_Children.count() - 1; i >= 0; i-- )
    {
        delete m_Children.at(i);
    }
    
    m_Children.clear();
}

void KeyValuesNode::clear()
{
    m_szKey = QString("");
    m_vValue = QVariant();
    deleteChildren();
}

bool KeyValuesNode::isRoot()
{
    return (parentNode() == NULL);
}

QString KeyValuesNode::key() const
{
    return m_szKey;
}

void KeyValuesNode::setKey(const QString &key)
{
    m_szKey = key;
}

void KeyValuesNode::setKey(const char *key)
{
    m_szKey = QString(key);
}

QVariant KeyValuesNode::value() const
{
    return m_vValue;
}

void KeyValuesNode::setValue(const QVariant &value)
{
    m_vValue = value;
}

void KeyValuesNode::setValue(bool value)
{
    setValue(value ? 1 : 0);
}

void KeyValuesNode::setValue(const QString &value)
{
    m_vValue = QVariant(value);
}

void KeyValuesNode::setValue(float value)
{
    m_vValue = QVariant(value);
}

void KeyValuesNode::setValue(int value)
{
    m_vValue = QVariant(value);
}

void KeyValuesNode::setValue(const char *value)
{
    setValue(QString(value));
}

// This function assumes we are on a new line when we start writing.
// Writing is terminated with a newline.
template <typename T>
T& KeyValuesNode::writeToStream(T &stream, const QString &tab, int depth) const
{
    // If we have no data, return.
    // Trialling removing this - if we have no key but do have children, it's usually
    // clearer to just write the key as an empty string and write the children anyway.
    //if ( m_szKey.isEmpty() && m_bWriteKey ) return stream;

    // Create a string with the correct number of tab characters.
    QString tabs;
    for ( int i = 0; i < depth; i++ )
    {
        tabs.append(tab);
    }

    // Write our key.
    // We only want to forego writing the key if we have a list of children to write.
    if ( m_bWriteKey || m_Children.count() <= 0 )
    {
        stream << tabs << "\"" << m_szKey << "\"";
    }

    // We need to write different things depending on what data we're holding.
    // - If we have children, write them recursively within a set of curly brackets.
    // - If we don't have children but we have a value, write this next to our key.
    // - If we have neither, write a set of curly brackets and exit.

    if ( m_Children.count() > 0 )
    {
        // Write the first curly bracket.
        if ( m_bWriteKey )
        {
            stream << "\n" << tabs << KV_BOPEN << "\n";
        }

        // Write each child.
        for ( int i = 0; i < m_Children.count(); i++ )
        {
            m_Children.at(i)->writeToStream(stream, tab, m_bWriteKey ? depth + 1 : depth);
        }

        // Write the last curly bracket.
        if ( m_bWriteKey )
        {
            stream << tabs << KV_BCLOSE << "\n";
        }
    }
    else if ( m_vValue.isValid() )
    {
        // Write the value next to the key.
        stream << " \"" << m_vValue.toString() << "\"\n";
    }
    else
    {
        // Just write the curly brackets.
        stream << "\n" << tabs << KV_BOPEN << "\n" << tabs << KV_BCLOSE << "\n";
    }

    return stream;
}

// If there are multiple root nodes defined in the input, only the first is used.
void KeyValuesNode::constructFrom(const QString &input)
{
    // Initialise this node with a value before we call read recursively.
    // The value should be a leaf or (more likely) a section creator.
    
    KVPair pair;
    int windOn = getKeyAndValue(input, pair);
    
    // Modifying the current behaviour:
    // Right now, the keyvalue only parses a single leaf if one is encountered at this point.
    // Multiple leaves have to be children of a single root node.
    // However, HL2 files like config.cfg list their options without a single root,
    // so it would be nice to be able to read these types of files too.
    // To do this, we should list all the leaves underneath one root with writeKey() set to false.
    // As a cheap way of checking, we should see if there are two consecutive root leaves -
    // if there's only one then the data can just be kept in this node, but if there's more then one
    // then they (and any others) should be added to this node as children.
    
    if ( isInvalid(pair) || isSectionTerminator(pair) )
    {
        qWarning("First token from string beginning \"%s\" is invalid.", input.left(10).toLatin1().constData());
        return;
    }
    
    else if ( isLeaf(pair) )
    {
        // The root is a leaf.
        // Make this node an "invisible" parent and add the leaves as children.
        setWriteKey(false);
        readFrom(input);
        return;
    }
    
    else if ( isSectionCreator(pair) )
    {
        setKey(pair.first);
        windOn++;
        windOn = readFrom(input.mid(windOn));
        return;
    }
    
    // We should never get here!
    Q_ASSERT(false);
}

// Reading process:
// Head node is the head of the current subtree.
// If a key and value are encountered, create a child node and add it as a child to the head.
// If a key and open curly bracket are encountered, create a node, add it as a child of the head
// and make it the new head.
// If a closing curly bracket is encountered, make the parent of the current node the new head.
// The return is the number of characters to wind on so that the next iteration of a loop will encounter the next character after the
// end of the token.
int KeyValuesNode::readFrom(const QString &input)
{
    // Read from the beginning of the string.
    for ( int i = 0; i < input.length(); i++ )
    {
        KVPair pair;
        i += getKeyAndValue(input.mid(i), pair);
        
        // Do things depending on the key and value we obtained.
        
        // If the key is null, get the next pair.
        // If the string has run out, the loop won't run again.
        if ( isInvalid(pair) )
        {
            continue;
        }
        
        // If the pair is a leaf node (ie. not a section creator or terminator), add it as one of our children.
        else if ( isLeaf(pair) )
        {
            appendChild(new KeyValuesNode(pair));
            continue;
        }
        
        // If the pair is a section creator, add it as one of our child nodes and call readFrom() on the child.
        else if ( isSectionCreator(pair) )
        {
            KeyValuesNode* child = new KeyValuesNode(pair);
            appendChild(child);
            
            // Increment i so that we begin from a new character.
            i++;
            
            // Call readFrom() on the child. The return will be the index before the next character to read in the main loop.
            i += child->readFrom(input.mid(i));
            continue;
        }
        
        // If the pair is a section terminator, relinquish control to our caller.
        else if ( isSectionTerminator(pair) )
        {
            // Return the number of characters to wind on in order for the next loop iteration
            // to start on a fresh character.
            return i;
        }
        
        // We should never get here!
        qCritical("Pair reached invalid section! Key = %s, value = %s", pair.first.toLatin1().constData(), pair.second.toLatin1().constData());
        Q_ASSERT(false);
    }
    
    // We've reached the end - return;
    return input.length() - 1;
}

// If the beginning of the string is a quote mark, seeks to the next non-escaped quote and outputs the substring.
// If not, seeks to the next whitespace character and outputs the substring.
// The return is the number of characters to wind on so that the next iteration of a loop will encounter the next character after the
// end of the token.
// Assumed the input string is trimmed at the beginning.
int KeyValuesNode::getToken(const QString &input, QString &tokenOut)
{
    bool firstCharQuote = input.at(0) == '\"';
    for ( int i = 0; i < input.length(); i++ )
    {
        // If we're looking for whitespace:
        if ( !firstCharQuote )
        {
            if ( input.at(i).isSpace() )
            {
                tokenOut = input.left(i);
                // qDebug() << "Looking for whitespace within" << input.left(10) << "...: found at" << i << "Output token:" << tokenOut << "Returning index:" << tokenOut.length() - 1;
                return tokenOut.length() - 1;
            }
        }
        
        // If we're looking for non-escaped quotes:
        else
        {
            if ( input.at(i) == '\"' && i > 0 && input.at(i-1) != '\\' )
            {
                tokenOut = input.mid(1, i-1);
                // qDebug() << "Looking for unescaped quote within" << input.left(10) << "...: found at" << i << "Output token:" << tokenOut << "Returning index:" << tokenOut.length() + 1;
                return tokenOut.length() + 1;
            }
        }
    }
    
    // We reached the end - return the whole string apart from the first character,
    tokenOut = input.mid(1);
    // qDebug() << "Reach the end of the input string. Output token:" << tokenOut << "Returning index:" << tokenOut.length() - 1;
    return tokenOut.length() - 1;
}

// Outputs the first key-value pair from the current input string.&& !isSectionTerminator(pair)
// If the value is a single unquoted open curly bracket, the node with the specified key should become current.
// If the key is a single unquoted closing curly bracket, the input of children for the current node has ended and the parent should become current.
// If the key is empty, no pair (or an invalid pair) was found.
// Otherwise, the key and value should be added as a child of the current node.
// The return is the number of characters to wind on so that the next iteration of a loop will encounter the next character after the
// end of the token.
int KeyValuesNode::getKeyAndValue(const QString &input, KVPair &pair)
{
    // Wind on while there are still spaces.
    int i = 0;
    while ( i < input.length() && input.at(i).isSpace() ) i++;
    
    // i contains the index of the first non-space. If it's >= the length, return nothing.
    if ( i >= input.length() )
    {
        pair.first = pair.second = QString();
        return input.length() - 1;
    }
    
    // Get the first token from index i;
    QString firstToken;
    int windOn = getToken(input.mid(i), firstToken);
    // qDebug() << "Retrieved token" << firstToken << "Mid index:" << i << "Char:" << input.at(i) << "Wind-on value:" << windOn;
    
    // If the returned token was a closing bracket, return this as the key.
    if ( firstToken == KV_BCLOSE )
    {
        pair.first = firstToken;
        pair.second = QString();
        return i + windOn;
    }
    
    // If the returned token was an opening bracket, this is invalid as a key.
    // Return nothing.
    else if ( firstToken == KV_BOPEN )
    {
        pair.first = pair.second = QString();
        return i + windOn;
    }
    
    // Otherwise, set as the key and prepare to read the value.
    pair.first = firstToken;
    i += windOn + 1;    // Account for what would have been the extra loop increment.
    
    // Wind on past spaces.
    while ( i < input.length() && input.at(i).isSpace() ) i++;
    
    // i again holds the index of the first non-space. If it's >= the length, return nothing.
    if ( i >= input.length() )
    {
        pair.first = pair.second = QString();
        return input.length() - 1;
    }
    
    // Get the next token.
    QString secondToken;
    windOn = getToken(input.mid(i), secondToken);
    
    // If the token is a closing bracket, this is invalid. Return nothing.
    if ( secondToken == KV_BCLOSE )
    {
        pair.first = pair.second = QString();
        return i + windOn;
    }
    
    // Otherwise, set the token as the value and return;
    pair.second = secondToken;
    return i + windOn;
}

bool KeyValuesNode::isInvalid(const KVPair &pair)
{
    return pair.first.isNull();
}

bool KeyValuesNode::isLeaf(const KVPair &pair)
{
    return !isSectionCreator(pair) && !isSectionTerminator(pair);
}

bool KeyValuesNode::isSectionTerminator(const KVPair &pair)
{
    return pair.first == KV_BCLOSE;
}

bool KeyValuesNode::isSectionCreator(const KVPair &pair)
{
    return !isInvalid(pair) && pair.second == KV_BOPEN;
}

bool KeyValuesNode::writeKey() const
{
    return m_bWriteKey;
}

void KeyValuesNode::setWriteKey(bool b)
{
    m_bWriteKey = b;
}

bool KeyValuesNode::isEmpty() const
{
    if ( m_bWriteKey )
    {
        return m_Children.count() <= 0;
    }
    else
    {
        return m_szKey.isEmpty();
    }
}

// This function does not check recursively.
bool KeyValuesNode::contains(const QString &key) const
{
    for ( int i = 0; i < m_Children.count(); i++ )
    {
        if ( m_Children.at(i)->key() == key ) return true;
    }

    return false;
}

KeyValuesNode* KeyValuesNode::find(const QString &key) const
{
    for ( int i = 0; i < m_Children.count(); i++ )
    {
        if ( m_Children.at(i)->key() == key ) return m_Children.at(i);
    }

    return NULL;
}

QTextStream& operator <<(QTextStream &stream, const KeyValuesNode &node)
{
    return node.writeToStream(stream);
}

QDataStream& operator <<(QDataStream &stream, const KeyValuesNode &node)
{
    return node.writeToStream(stream);
}
