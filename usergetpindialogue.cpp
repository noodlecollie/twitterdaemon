#include "usergetpindialogue.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QDesktopServices>
#include <QUrl>

UserGetPinDialogue::UserGetPinDialogue(QWidget *parent) :
    QDialog(parent), m_pLayout(NULL), m_pDescription(NULL), m_pGoToURL(NULL), m_pInputPin(NULL),
    m_pPinIn(NULL), m_pWidgetBottom(NULL), m_pLayoutBottom(NULL), m_pSubmit(NULL),
    m_pCancel(NULL), m_pPinOut(NULL), m_Url()
{
    init();
    setModal(true);
    setSizeGripEnabled(false);

    adjustSize();
    //setFixedSize(size());
    setMinimumSize(size());
    setMaximumSize(size());
}

void UserGetPinDialogue::init()
{
    m_pLayout = new QVBoxLayout();
    setLayout(m_pLayout);

    m_pDescription = new QLabel(QString(""));
    m_pLayout->addWidget(m_pDescription);
    m_pDescription->setWordWrap(true);

    m_pGoToURL = new QPushButton(QString(tr("&Open")));
    m_pLayout->addWidget(m_pGoToURL);

    m_pInputPin = new QLabel("Once the PIN is retrieved, enter it here:");
    m_pLayout->addWidget(m_pInputPin);

    m_pPinIn = new QLineEdit();
    m_pLayout->addWidget(m_pPinIn);

    m_pWidgetBottom = new QWidget();
    m_pLayoutBottom = new QHBoxLayout();
    m_pWidgetBottom->setLayout(m_pLayoutBottom);
    m_pSubmit = new QPushButton(QString(tr("&Submit")));
    m_pCancel = new QPushButton(QString(tr("&Cancel")));
    m_pLayoutBottom->addWidget(m_pSubmit);
    m_pLayoutBottom->addWidget(m_pCancel);
    m_pLayout->addWidget(m_pWidgetBottom);

    connect(m_pSubmit, SIGNAL(pressed()), this, SLOT(submitPin()));
    connect(m_pCancel, SIGNAL(pressed()), this, SLOT(close()));
    connect(m_pGoToURL, SIGNAL(pressed()), this, SLOT(openUrl()));
    connect(m_pPinIn, SIGNAL(returnPressed()), m_pSubmit, SLOT(click()));
}

void UserGetPinDialogue::openUrl()
{
    QUrl url(m_Url);
    url.setScheme("https");
    QDesktopServices::openUrl(url);
}

void UserGetPinDialogue::submitPin()
{
    if ( m_pPinOut ) *m_pPinOut = m_pPinIn->text().trimmed();
    close();
}

void UserGetPinDialogue::setUrl(QString url)
{
    m_Url = url.trimmed();

    // This is quite cheap.
    QString desc("To complete authentication, please visit ");
    desc.append(m_Url);
    m_pDescription->setText(desc);
}
