#include "CameraConfigurator.h"
#include <arv.h>
#include <QByteArray>

QList<CameraInfo> CameraConfigurator::discoverCameras(QString &output) {
    QList<CameraInfo> result;
    arv_update_device_list();
    const unsigned int n = arv_get_n_devices();
    output = QString("[i] Found %1 camera(s).\n").arg(n);

    for (unsigned int i = 0; i < n; ++i) {
        CameraInfo info;
        const char *id      = arv_get_device_id(i);
        const char *vendor  = arv_get_device_vendor(i);
        const char *address = arv_get_device_address(i);
        info.id     = QString::fromUtf8(id ? id : "");
        info.vendor = QString::fromUtf8(vendor ? vendor : "");
        info.ip     = QString::fromUtf8(address ? address : "");
        result.append(info);
    }
    return result;
}

bool CameraConfigurator::setCameraIp(const QString &cameraId,
                                     const QString &ip,
                                     const QString &mask,
                                     const QString &gateway,
                                     const QString &mode,
                                     QString &output) {
    GError *error = nullptr;
    ArvCamera *camera = arv_camera_new(cameraId.toUtf8().constData(), &error);
    if (!camera) {
        output += QString("[!] Failed to open camera: %1\n")
                    .arg(error ? error->message : "unknown");
        if (error) g_error_free(error);
        return false;
    }

    bool ok = true;
    const QByteArray ipBa   = ip.toUtf8();
    const QByteArray maskBa = mask.toUtf8();
    const QByteArray gwBa   = gateway.toUtf8();

    // 1) Set the persistent IP triplet
    arv_camera_gv_set_persistent_ip_from_string(
        camera, ipBa.constData(), maskBa.constData(), gwBa.constData(), &error);
    if (error) {
        output += QString("[!] set_persistent_ip failed: %1\n").arg(error->message);
        g_error_free(error); error = nullptr; ok = false;
    } else {
        output += "[i] Persistent IP triplet updated.\n";
    }

    // 2) Switch the IP configuration mode
    ArvGvIpConfigurationMode m = ARV_GV_IP_CONFIGURATION_MODE_PERSISTENT_IP;
    if (mode == "DHCP")       m = ARV_GV_IP_CONFIGURATION_MODE_DHCP;
    else if (mode == "LLA")   m = ARV_GV_IP_CONFIGURATION_MODE_LLA;

    arv_camera_gv_set_ip_configuration_mode(camera, m, &error);
    if (error) {
        output += QString("[!] set_ip_configuration_mode failed: %1\n").arg(error->message);
        g_error_free(error); error = nullptr; ok = false;
    } else {
        output += QString("[i] Configuration mode set to %1.\n").arg(mode);
    }

    g_object_unref(camera);
    if (ok) output += "[+] Camera IP configuration applied.\n";
    return ok;
}
