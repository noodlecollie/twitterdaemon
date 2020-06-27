#ifndef TWITTERACCOUNTDIALOGUE_H
#define TWITTERACCOUNTDIALOGUE_H

#include <QDialog>

class QLineEdit;
class QPushButton;
class QFormLayout;
class QCheckBox;

class TwitterAccountDialogue : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool closeOnLogin READ closeOnLogin WRITE setCloseOnLogin)
public:
    explicit TwitterAccountDialogue(QWidget *parent = 0);
    void setCloseOnLogin(bool b);
    bool closeOnLogin() const;

    void setUsernameOut(QString* out);
    QString* usernameOut() const;
    void setPasswordOut(QString* out);
    QString* passwordOut() const;
    void setRememberOut(bool* out);
    bool* rememberOut() const;

signals:

public slots:
    void submit();
    void dataChanged(const QString &data);

private:
    void init();

    // Removes preceding @ symbol if and only if it is at index 0.
    static QString withoutPrecedingAt(const QString &str);

    QFormLayout*    m_pLayout;
    QLineEdit*      m_pIUsername;
    QLineEdit*      m_pIPassword;
    QCheckBox*      m_pRemember;
    QPushButton*    m_pLogin;
    bool            m_bCloseOnLogin;

    QString*        m_pUsernameOut;
    QString*        m_pPasswordOut;
    bool*           m_bRememberOut;
};

#endif // TWITTERACCOUNTDIALOGUE_H
