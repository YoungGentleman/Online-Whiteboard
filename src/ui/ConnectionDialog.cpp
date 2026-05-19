#include "ConnectionDialog.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QNetworkInterface>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageBox>

ConnectionDialog::ConnectionDialog(Mode mode, QWidget *parent)
    : QDialog(parent), m_mode(mode)
{
    setModal(true);
    setFixedWidth(380);
    if (m_mode == Mode::Host) { setWindowTitle(tr("Create room")); buildHostUi(); }
    else                      { setWindowTitle(tr("Join room"));   buildClientUi(); }
}

void ConnectionDialog::buildHostUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    QString localIp = tr("(unavailable)");
    for (const QHostAddress &addr : QNetworkInterface::allAddresses()) {
        if (!addr.isLoopback() && addr.protocol() == QAbstractSocket::IPv4Protocol) {
            localIp = addr.toString(); break;
        }
    }

    auto *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);

    m_lblLocalIp = new QLabel(localIp);
    m_lblLocalIp->setTextInteractionFlags(Qt::TextSelectableByMouse);
    form->addRow(tr("Local IP:"), m_lblLocalIp);

    m_lblPublicIp = new QLabel(tr("Fetching…"));
    m_lblPublicIp->setTextInteractionFlags(Qt::TextSelectableByMouse);
    form->addRow(tr("Public IP:"), m_lblPublicIp);

    m_hostPort = new QSpinBox();
    m_hostPort->setRange(1024, 65535);
    m_hostPort->setValue(kDefaultPort);
    form->addRow(tr("Port:"), m_hostPort);
    layout->addLayout(form);

    auto *note = new QLabel(tr("<i>Share your Public IP and port with participants.<br>"
                               "Make sure the port is open in your firewall / router.</i>"));
    note->setWordWrap(true);
    note->setStyleSheet("color: gray; font-size: 11px;");
    layout->addWidget(note);

    auto *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btnBox, &QDialogButtonBox::accepted, this, &ConnectionDialog::onConfirm);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    m_btnConfirm = btnBox->button(QDialogButtonBox::Ok);
    m_btnConfirm->setText(tr("Start hosting"));
    layout->addWidget(btnBox);

    onFetchPublicIp();
}

void ConnectionDialog::buildClientUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    auto *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);

    m_editIp = new QLineEdit();
    m_editIp->setPlaceholderText(tr("e.g. 203.0.113.42"));
    form->addRow(tr("Host IP:"), m_editIp);

    m_clientPort = new QSpinBox();
    m_clientPort->setRange(1024, 65535);
    m_clientPort->setValue(kDefaultPort);
    form->addRow(tr("Port:"), m_clientPort);
    layout->addLayout(form);

    auto *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btnBox, &QDialogButtonBox::accepted, this, &ConnectionDialog::onConfirm);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    m_btnConfirm = btnBox->button(QDialogButtonBox::Ok);
    m_btnConfirm->setText(tr("Connect"));
    layout->addWidget(btnBox);
}

void ConnectionDialog::onConfirm()
{
    if (m_mode == Mode::Client && m_editIp && m_editIp->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Missing address"), tr("Please enter the host IP address."));
        return;
    }
    accept();
}

void ConnectionDialog::onFetchPublicIp()
{
    if (m_mode != Mode::Host || !m_lblPublicIp) return;
    auto *nam = new QNetworkAccessManager(this);
    QNetworkReply *reply = nam->get(QNetworkRequest(QUrl("https://api.ipify.org")));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError)
            m_lblPublicIp->setText(QString::fromUtf8(reply->readAll()).trimmed());
        else
            m_lblPublicIp->setText(tr("(unavailable)"));
        reply->deleteLater();
    });
}

QString ConnectionDialog::hostAddress() const
{
    return (m_mode == Mode::Client && m_editIp) ? m_editIp->text().trimmed() : QString{};
}

int ConnectionDialog::port() const
{
    if (m_mode == Mode::Host   && m_hostPort)   return m_hostPort->value();
    if (m_mode == Mode::Client && m_clientPort) return m_clientPort->value();
    return kDefaultPort;
}
