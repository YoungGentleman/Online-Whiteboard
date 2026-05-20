#include "ConnectionDialog.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageBox>

ConnectionDialog::ConnectionDialog(Mode mode, QWidget *parent)
    : QDialog(parent), m_mode(mode)
{
    setModal(true);
    setFixedWidth(400);

    if (m_mode == Mode::Host) {
        setWindowTitle(tr("Создать комнату"));
        buildHostUi();
    } else {
        setWindowTitle(tr("Войти в комнату"));
        buildClientUi();
    }
}

void ConnectionDialog::buildHostUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    QString localIp = tr("(недоступен)");
    for (const QHostAddress &addr : QNetworkInterface::allAddresses()) {
        if (!addr.isLoopback() && addr.protocol() == QAbstractSocket::IPv4Protocol) {
            localIp = addr.toString();
            break;
        }
    }

    auto *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);

    m_lblLocalIp = new QLabel(localIp);
    m_lblLocalIp->setTextInteractionFlags(Qt::TextSelectableByMouse);
    form->addRow(tr("Локальный IP (LAN):"), m_lblLocalIp);

    m_lblPublicIp = new QLabel(tr("Получаем…"));
    m_lblPublicIp->setTextInteractionFlags(Qt::TextSelectableByMouse);
    form->addRow(tr("Внешний IP (Internet):"), m_lblPublicIp);

    m_hostPort = new QSpinBox();
    m_hostPort->setRange(1024, 65535);
    m_hostPort->setValue(kDefaultPort);
    form->addRow(tr("Порт:"), m_hostPort);

    m_hostRoom = new QLineEdit("default");
    m_hostRoom->setToolTip(tr(
        "На одном сервере может быть много комнат с разными именами.\n"
        "Все, кто введёт это имя при подключении, попадут в эту комнату."));
    form->addRow(tr("Имя комнаты:"), m_hostRoom);

    layout->addLayout(form);

    auto *note = new QLabel(tr(
        "<i>Поделитесь IP, портом и именем комнаты с участниками.<br>"
        "Для LAN — локальный IP.<br>"
        "Для Internet — внешний IP + проброс порта на роутере.</i>"));
    note->setWordWrap(true);
    note->setStyleSheet("color: gray; font-size: 11px;");
    layout->addWidget(note);

    auto *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btnBox, &QDialogButtonBox::accepted, this, &ConnectionDialog::onConfirm);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    m_btnConfirm = btnBox->button(QDialogButtonBox::Ok);
    m_btnConfirm->setText(tr("Создать комнату"));
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
    m_editIp->setPlaceholderText(tr("например 192.168.1.42 или 203.0.113.42"));
    form->addRow(tr("IP хоста:"), m_editIp);

    m_clientPort = new QSpinBox();
    m_clientPort->setRange(1024, 65535);
    m_clientPort->setValue(kDefaultPort);
    form->addRow(tr("Порт:"), m_clientPort);

    m_clientRoom = new QLineEdit("default");
    m_clientRoom->setToolTip(tr(
        "Введите имя нужной комнаты.\n"
        "Один сервер обслуживает много комнат — клиенты с одинаковым\n"
        "именем оказываются на общей доске."));
    form->addRow(tr("Имя комнаты:"), m_clientRoom);

    layout->addLayout(form);

    auto *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btnBox, &QDialogButtonBox::accepted, this, &ConnectionDialog::onConfirm);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    m_btnConfirm = btnBox->button(QDialogButtonBox::Ok);
    m_btnConfirm->setText(tr("Подключиться"));
    layout->addWidget(btnBox);
}

void ConnectionDialog::onConfirm()
{
    if (m_mode == Mode::Client && m_editIp && m_editIp->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Не указан адрес"),
                             tr("Введите IP хоста."));
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
            if (m_lblPublicIp) m_lblPublicIp->setText(tr("(недоступен)"));
        }
        reply->deleteLater();
    });
}

QString ConnectionDialog::hostAddress() const
{
    if (m_mode == Mode::Client && m_editIp)
        return m_editIp->text().trimmed();
    return {};
}

int ConnectionDialog::port() const
{
    if (m_mode == Mode::Host   && m_hostPort)   return m_hostPort->value();
    if (m_mode == Mode::Client && m_clientPort) return m_clientPort->value();
    return kDefaultPort;
}

QString ConnectionDialog::roomName() const
{
    if (m_mode == Mode::Host   && m_hostRoom) {
        const QString r = m_hostRoom->text().trimmed();
        return r.isEmpty() ? QStringLiteral("default") : r;
    }
    if (m_mode == Mode::Client && m_clientRoom) {
        const QString r = m_clientRoom->text().trimmed();
        return r.isEmpty() ? QStringLiteral("default") : r;
    }
    return QStringLiteral("default");
}
