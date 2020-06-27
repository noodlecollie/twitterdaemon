#ifndef USERGETPINDIALOGUE_H
#define USERGETPINDIALOGUE_H

#include <QDialog>
#include <QScopedPointer>

class QVBoxLayout;
class QLabel;
class QPushButton;
class QLineEdit;
class QHBoxLayout;

class UserGetPinDialogue : public QDialog
{
    Q_OBJECT
public:
    explicit UserGetPinDialogue(QWidget *parent = 0);
    QString url() const { return m_Url; }

signals:

public slots:
    void setPinOut(QString* out) { m_pPinOut = out; }
    void submitPin();
    void openUrl();
    void setUrl(QString url);

private:
    void init();

    QVBoxLayout*    m_pLayout;
    QLabel*         m_pDescription;
    QPushButton*    m_pGoToURL;
    QLabel*         m_pInputPin;
    QLineEdit*      m_pPinIn;
    QWidget*        m_pWidgetBottom;
    QHBoxLayout*    m_pLayoutBottom;
    QPushButton*    m_pSubmit;
    QPushButton*    m_pCancel;

    QString*        m_pPinOut;
    QString         m_Url;
};

#endif // USERGETPINDIALOGUE_H
