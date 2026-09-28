#ifndef CAMERATAB_H
#define CAMERATAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class CameraTab; }
QT_END_NAMESPACE

class CameraTab : public QWidget {
    Q_OBJECT
public:
    explicit CameraTab(QWidget *parent = nullptr);
    ~CameraTab() override;

private slots:
    void refreshCameras();
    void onCameraSelected();
    void applyConfiguration();

private:
    Ui::CameraTab *ui;
};

#endif // CAMERATAB_H
