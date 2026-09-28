#include "CameraTab.h"
#include "ui_CameraTab.h"
#include "CameraConfigurator.h"

#include <QListWidgetItem>
#include <QMessageBox>

CameraTab::CameraTab(QWidget *parent)
    : QWidget(parent), ui(new Ui::CameraTab) {
    ui->setupUi(this);

    connect(ui->refreshBtn, &QPushButton::clicked,
            this, &CameraTab::refreshCameras);
    connect(ui->cameraList, &QListWidget::itemSelectionChanged,
            this, &CameraTab::onCameraSelected);
    connect(ui->applyBtn, &QPushButton::clicked,
            this, &CameraTab::applyConfiguration);

    refreshCameras();
}

CameraTab::~CameraTab() {
    delete ui;
}

void CameraTab::refreshCameras() {
    ui->cameraList->clear();
    QString output;
    const auto cams = CameraConfigurator::discoverCameras(output);
    ui->log->append(output);
    for (const auto &c : cams) {
        QString text = QString("%1 | %2 | %3").arg(c.id, c.ip, c.vendor);
        auto *item = new QListWidgetItem(text);
        item->setData(Qt::UserRole,     c.id);
        item->setData(Qt::UserRole + 1, c.ip);
        ui->cameraList->addItem(item);
    }
}

void CameraTab::onCameraSelected() {
    auto items = ui->cameraList->selectedItems();
    if (items.isEmpty()) return;
    ui->ipEdit->setText(items.first()->data(Qt::UserRole + 1).toString());
}

void CameraTab::applyConfiguration() {
    auto items = ui->cameraList->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a camera.");
        return;
    }
    const QString camId = items.first()->data(Qt::UserRole).toString();
    const QString ip    = ui->ipEdit->text().trimmed();
    const QString mask  = ui->maskEdit->text().trimmed();
    const QString gw    = ui->gatewayEdit->text().trimmed();
    const QString mode  = ui->modeCombo->currentData().toString();

    QString output;
    const bool ok = CameraConfigurator::setCameraIp(camId, ip, mask, gw, mode, output);
    ui->log->append(output);
    if (ok) {
        QMessageBox::information(
            this, "Success",
            "Camera IP configuration applied.\n"
            "Note: camera may reconnect with the new address.");
    } else {
        QMessageBox::critical(this, "Error", "Failed to configure camera.");
    }
}
