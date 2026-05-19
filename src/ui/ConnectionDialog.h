#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>

class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Mode { Host, Client };

    explicit ConnectionDialog(Mode mode, QWidget *parent = nullptr);

    Mode    mode()        const { return m_mode; }
    QString hostAddress() const;    
    int     port()        const;
    QString roomName()    const;

private slots:
    void onConfirm();
    void onFetchPublicIp();   // download from https://api.ipify.org

private:
    void buildHostUi();
    void buildClientUi();

    Mode        m_mode;

    // Host
    QLabel     *m_lblLocalIp   = nullptr;
    QLabel     *m_lblPublicIp  = nullptr;
    QSpinBox   *m_hostPort     = nullptr;
    QLineEdit  *m_hostRoom     = nullptr;

    // Client
    QLineEdit  *m_editIp       = nullptr;
    QSpinBox   *m_clientPort   = nullptr;
    QLineEdit  *m_clientRoom   = nullptr;

    QPushButton *m_btnConfirm  = nullptr;

    static constexpr int kDefaultPort = 45000;
};