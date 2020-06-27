#include "twitteraccountdialogue.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QChar>

TwitterAccountDialogue::TwitterAccountDialogue(QWidget *parent) :
    QDialog(parent), m_pLayout(NULL), m_pIUsername(NULL), m_pIPassword(NULL), m_pRemember(NULL),
    m_pLogin(NULL), m_bCloseOnLogin(false), m_pUsernameOut(NULL), m_pPasswordOut(NULL), m_bRememberOut(NULL)
{
    init();
    setModal(true);
    setSizeGripEnabled(false);

    adjustSize();
    setFixedSize(size());
}

void TwitterAccountDialogue::init()
{
    m_pLayout = new QFormLayout();
    setLayout(m_pLayout);

    m_pIUsername = new QLineEdit();
    m_pIPassword = new QLineEdit();
    m_pIUsername->setPlaceholderText(tr("@username"));
    m_pIPassword->setEchoMode(QLineEdit::Password);
    m_pIPassword->setPlaceholderText(tr("password"));
    m_pLayout->addRow(tr("&Username:"), m_pIUsername);
    m_pLayout->addRow(tr("&Password:"), m_pIPassword);

    m_pRemember = new QCheckBox(tr("&Remember me"));
    m_pLayout->addWidget(m_pRemember);

    m_pLogin = new QPushButton(tr("&Log in"));
    m_pLogin->setEnabled(false);
    m_pLayout->addWidget(m_pLogin);

    connect(m_pLogin, SIGNAL(pressed()), this, SLOT(submit()));
    connect(m_pIUsername, SIGNAL(textChanged(QString)), this, SLOT(dataChanged(const QString&)));
    connect(m_pIPassword, SIGNAL(textChanged(QString)), this, SLOT(dataChanged(const QString&)));
}

void TwitterAccountDialogue::dataChanged(const QString &data)
{
    Q_UNUSED(data);

    if ( m_pIUsername->text().isEmpty() || m_pIPassword->text().isEmpty() ) m_pLogin->setEnabled(false);
    else m_pLogin->setEnabled(true);
}

void TwitterAccountDialogue::submit()
{
    if ( m_pUsernameOut ) *m_pUsernameOut = withoutPrecedingAt(m_pIUsername->text());
    if ( m_pPasswordOut ) *m_pPasswordOut = m_pIPassword->text();
    if ( m_bRememberOut ) *m_bRememberOut = m_pRemember->isChecked();
    if ( m_bCloseOnLogin ) close();
}

bool TwitterAccountDialogue::closeOnLogin() const
{
    return m_bCloseOnLogin;
}

void TwitterAccountDialogue::setCloseOnLogin(bool b)
{
    m_bCloseOnLogin = b;
}

void TwitterAccountDialogue::setUsernameOut(QString *out)
{
    m_pUsernameOut = out;
}

QString* TwitterAccountDialogue::usernameOut() const
{
    return m_pUsernameOut;
}

void TwitterAccountDialogue::setPasswordOut(QString *out)
{
    m_pPasswordOut = out;
}

QString* TwitterAccountDialogue::passwordOut() const
{
    return m_pPasswordOut;
}

void TwitterAccountDialogue::setRememberOut(bool *out)
{
    m_bRememberOut = out;
}

bool* TwitterAccountDialogue::rememberOut() const
{
    return m_bRememberOut;
}

QString TwitterAccountDialogue::withoutPrecedingAt(const QString &str)
{
    if ( str.length() < 1 || str.at(0) != '@' ) return str;

    return str.mid(1);
}
