#ifndef CAMERACONFIGURATOR_H
#define CAMERACONFIGURATOR_H

#include <QString>
#include <QList>

struct CameraInfo {
    QString id;
    QString ip;
    QString vendor;
};

class CameraConfigurator {
public:
    static QList<CameraInfo> discoverCameras(QString &output);

    // mode: "PERSISTENT" | "DHCP" | "LLA"
    static bool setCameraIp(const QString &cameraId,
                            const QString &ip,
                            const QString &mask,
                            const QString &gateway,
                            const QString &mode,
                            QString &output);
};

#endif // CAMERACONFIGURATOR_H
