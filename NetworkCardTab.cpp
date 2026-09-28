#include "NetworkCardTab.h"
#include "ui_NetworkCardTab.h"
#include "NetworkConfigurator.h"

#include <QListWidgetItem>
#include <QMessageBox>
#include <QNetworkInterface>
#include <QAbstractSocket>
#include <QColor>
#include <QList>

namespace {

struct IfaceRow {
    QString name;
    QString humanName;
    QString addressText;          // "192.168.1.10/24" or "Link down"
    NetworkConfigurator::LinkStatus status;
};

int statusRank(NetworkConfigurator::LinkStatus s) {
    switch (s) {
        case NetworkConfigurator::LinkStatus::Up:      return 0;
        case NetworkConfigurator::LinkStatus::Unknown: return 1;
        case NetworkConfigurator::LinkStatus::Down:    return 2;
    }
    return 3;
}

QString statusPrefix(NetworkConfigurator::LinkStatus s) {
    switch (s) {
        case NetworkConfigurator::LinkStatus::Up:      return QStringLiteral("[UP]");
        case NetworkConfigurator::LinkStatus::Down:    return QStringLiteral("[--]");
        case NetworkConfigurator::LinkStatus::Unknown: return QStringLiteral("[??]");
    }
    return QStringLiteral("[??]");
}

QColor statusColor(NetworkConfigurator::LinkStatus s) {
    switch (s) {
        case NetworkConfigurator::LinkStatus::Up:      return QColor(0, 128, 0);   // dark green
        case NetworkConfigurator::LinkStatus::Down:    return QColor(128, 128, 128); // grey
        case NetworkConfigurator::LinkStatus::Unknown: return QColor(0, 0, 0);     // black
    }
    return QColor(0, 0, 0);
}

} // namespace

NetworkCardTab::NetworkCardTab(QWidget *parent)
    : QWidget(parent), ui(new Ui::NetworkCardTab) {
    ui->setupUi(this);

    connect(ui->refreshBtn, &QPushButton::clicked,
            this, &NetworkCardTab::refreshInterfaces);
    connect(ui->interfaceList, &QListWidget::itemSelectionChanged,
            this, &NetworkCardTab::onInterfaceSelected);
    connect(ui->applyBtn, &QPushButton::clicked,
            this, &NetworkCardTab::applyConfiguration);
    connect(ui->enableLlaBtn, &QPushButton::clicked,
            this, &NetworkCardTab::enableLlaForSelected);

    refreshInterfaces();
}

NetworkCardTab::~NetworkCardTab() {
    delete ui;
}

void NetworkCardTab::refreshInterfaces() {
    ui->interfaceList->clear();

    QList<IfaceRow> rows;

    const auto ifaces = QNetworkInterface::allInterfaces();
    for (const auto &iface : ifaces) {
        if (iface.flags() & QNetworkInterface::IsLoopBack) continue;

        IfaceRow row;
        row.name      = iface.name();
        row.humanName = iface.humanReadableName();
        row.status    = NetworkConfigurator::getLinkStatus(row.name);

        // Pick the first IPv4 address, if any, and render it in CIDR form.
        QString ipText;
        for (const auto &entry : iface.addressEntries()) {
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
                continue;
            ipText = QString("%1/%2")
                         .arg(entry.ip().toString())
                         .arg(entry.netmask().toString());
            break;
        }

        if (!ipText.isEmpty())
            row.addressText = ipText;
        else if (row.status == NetworkConfigurator::LinkStatus::Down)
            row.addressText = QStringLiteral("Link down");
        else if (row.status == NetworkConfigurator::LinkStatus::Unknown)
            row.addressText = QStringLiteral("Link unknown");
        else
            row.addressText = QStringLiteral("(no IPv4)");

        rows.append(row);
    }

    std::sort(rows.begin(), rows.end(),
              [](const IfaceRow &a, const IfaceRow &b) {
                  const int ra = statusRank(a.status);
                  const int rb = statusRank(b.status);
                  if (ra != rb) return ra < rb;
                  return a.name < b.name;
              });

    for (const auto &row : rows) {
        const QString text = QString("%1 %2  (%3)   %4")
                                 .arg(statusPrefix(row.status),
                                      row.name,
                                      row.humanName,
                                      row.addressText);
        auto *item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, row.name);
        item->setForeground(statusColor(row.status));
        ui->interfaceList->addItem(item);
    }

    ui->log->append(QString("[i] Found %1 interface(s).")
                        .arg(ui->interfaceList->count()));
}

void NetworkCardTab::onInterfaceSelected() {
    auto items = ui->interfaceList->selectedItems();
    if (items.isEmpty()) return;
    const QString name = items.first()->data(Qt::UserRole).toString();
    for (const auto &iface : QNetworkInterface::allInterfaces()) {
        if (iface.name() != name) continue;
        for (const auto &entry : iface.addressEntries()) {
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
                continue;
            ui->ipEdit->setText(entry.ip().toString());
            ui->maskEdit->setText(entry.netmask().toString());
            ui->gatewayEdit->clear();
            return;
        }
    }
}

void NetworkCardTab::applyConfiguration() {
    auto items = ui->interfaceList->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::warning(this, "Warning",
                             "Please select an interface first.");
        return;
    }
    const QString iface = items.first()->data(Qt::UserRole).toString();
    const QString ip    = ui->ipEdit->text().trimmed();
    const QString mask  = ui->maskEdit->text().trimmed();
    const QString gw    = ui->gatewayEdit->text().trimmed();

    if (ip.isEmpty() || mask.isEmpty()) {
        QMessageBox::warning(this, "Warning",
                             "IP and Subnet Mask are required.");
        return;
    }
    QString output;
    const bool ok = NetworkConfigurator::setInterfaceIp(iface, ip, mask, gw, output);
    ui->log->append(output);
    if (ok) QMessageBox::information(this, "Success", "Interface IP updated.");
    else    QMessageBox::critical(this, "Error",
                                  "Failed to update interface IP.");
}

void NetworkCardTab::enableLlaForSelected() {
    auto items = ui->interfaceList->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::warning(this, "Warning",
                             "Please select an interface first.");
        return;
    }
    const QString iface = items.first()->data(Qt::UserRole).toString();

    QString output;
    const bool ok = NetworkConfigurator::addLlaAddress(iface, output);
    ui->log->append(output);
    if (ok) {
        QMessageBox::information(
            this, "Success",
            "LLA address added to " + iface + ".\n\n"
            "You can now switch to the \"Camera (Aravis)\" tab and "
            "click \"Discover / Refresh\" to find cameras whose IP "
            "was previously unreachable.");
    } else {
        QMessageBox::critical(this, "Error",
                              "Failed to add LLA address.\n"
                              "See the log for details.");
    }
}
