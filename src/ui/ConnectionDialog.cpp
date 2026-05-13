#include "ConnectionDialog.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>

ConnectionDialog::ConnectionDialog(Mode mode, QWidget *parent)
    : QDialog(parent), m_mode(mode)
{
    setModal(true);
    setFixedWidth(380);

    if (m_mode == Mode::Host) {
        setWindowTitle(tr("Create room"));
        buildHostUi();
    } else {
        setWindowTitle(tr("Join room"));
        buildClientUi();
    }
}

// ---------------------------------------------------------------------------
// UI builders
// ---------------------------------------------------------------------------

void ConnectionDialog::buildHostUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    // Detect local IP
    QString localIp = tr("(unavailable)");
    const auto ifaces = QNetworkInterface::allAddresses();
    for (const QHostAddress &addr : ifaces) {
        if (!addr.isLoopback() && addr.protocol() == QAbstractSocket::IPv4Protocol) {
            localIp = addr.toString();
            break;
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

    auto *note = new QLabel(
        tr("<i>Share your Public IP and port with participants.<br>"
           "Make sure the port is open in your firewall / router.</i>")
    );
    note->setWordWrap(true);
    note->setStyleSheet("color: gray; font-size: 11px;");
    layout->addWidget(note);

    auto *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btnBox, &QDialogButtonBox::accepted, this, &ConnectionDialog::onConfirm);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    m_btnConfirm = btnBox->button(QDialogButtonBox::Ok);
    m_btnConfirm->setText(tr("Start hosting"));
    layout->addWidget(btnBox);

    // Fetch public IP asynchronously
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

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------

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
    QNetworkRequest req(QUrl("https://api.ipify.org"));
    QNetworkReply *reply = nam->get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            const QString ip = QString::fromUtf8(reply->readAll()).trimmed();
            if (m_lblPublicIp) m_lblPublicIp->setText(ip);
        } else {
            if (m_lblPublicIp) m_lblPublicIp->setText(tr("(unavailable)"));
        }
        reply->deleteLater();
    });
}

// ---------------------------------------------------------------------------
// Getters
// ---------------------------------------------------------------------------

QString ConnectionDialog::hostAddress() const
{
    if (m_mode == Mode::Client && m_editIp)
        return m_editIp->text().trimmed();
    return {};
}

int ConnectionDialog::port() const
{
    if (m_mode == Mode::Host && m_hostPort) return m_hostPort->value();
    if (m_mode == Mode::Client && m_clientPort) return m_clientPort->value();
    return kDefaultPort;
}
