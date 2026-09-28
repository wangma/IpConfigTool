#include "NetworkConfigurator.h"
#include <QProcess>
#include <QHostAddress>

#if defined(Q_OS_LINUX)
#include <QFile>
#include <QByteArray>
#endif

int NetworkConfigurator::maskToPrefix(const QString &mask) {
    QHostAddress addr(mask);
    bool ok = false;
    const quint32 m = addr.toIPv4Address(&ok);
    if (!ok) return -1;
    int prefix = 0;
    for (int i = 31; i >= 0; --i) {
        if ((m >> i) & 1u) ++prefix;
        else break;
    }
    return prefix;
}

bool NetworkConfigurator::setInterfaceIp(const QString &interface,
                                         const QString &ip,
                                         const QString &mask,
                                         const QString &gateway,
                                         QString &output) {
    const int prefix = maskToPrefix(mask);
    if (prefix < 0 || prefix > 32) {
        output = "[!] Invalid subnet mask.\n";
        return false;
    }
    const QString cidr = QString("%1/%2").arg(ip).arg(prefix);

#if defined(Q_OS_LINUX)
    QString cmd = QString("ip addr flush dev %1; ip addr add %2 dev %1; ip link set %1 up")
                    .arg(interface, cidr);
    if (!gateway.isEmpty()) {
        cmd += QString("; ip route replace default via %1 dev %2")
                 .arg(gateway, interface);
    }
    output += QString("[i] Running: pkexec sh -c \"%1\"\n").arg(cmd);
    QProcess p;
    p.start("pkexec", {"sh", "-c", cmd});
    p.waitForFinished(60000);
    output += QString::fromUtf8(p.readAllStandardOutput());
    output += QString::fromUtf8(p.readAllStandardError());
    return p.exitCode() == 0;
#elif defined(Q_OS_WIN)
    const QString gwArg = gateway.isEmpty() ? QString("none") : gateway;
    QString cmd = QString("netsh interface ip set address \"%1\" static %2 %3 %4")
                    .arg(interface, ip, mask, gwArg);
    output += QString("[i] Running: cmd /c %1\n").arg(cmd);
    QProcess p;
    p.start("cmd", {"/c", cmd});
    p.waitForFinished(60000);
    output += QString::fromUtf8(p.readAllStandardOutput());
    output += QString::fromUtf8(p.readAllStandardError());
    return p.exitCode() == 0;
#else
    output = "[!] Unsupported platform.\n";
    return false;
#endif
}

bool NetworkConfigurator::addLlaAddress(const QString &interface,
                                        QString &output) {
#if defined(Q_OS_LINUX)
    // Add an LLA (169.254.0.0/16) address to the NIC. We try "add"
    // first; if it already exists, fall back to "change" so the
    // operation is idempotent.
    const QString addr = QStringLiteral("169.254.100.1/16");
    QString cmd = QString(
        "ip addr add %1 dev %2 2>/dev/null "
        "|| ip addr change %1 dev %2 "
        "; ip link set %2 up")
        .arg(addr, interface);

    output += QString("[i] Running: pkexec sh -c \"%1\"\n").arg(cmd);
    QProcess p;
    p.start("pkexec", {"sh", "-c", cmd});
    p.waitForFinished(60000);
    output += QString::fromUtf8(p.readAllStandardOutput());
    output += QString::fromUtf8(p.readAllStandardError());

    if (p.exitCode() == 0) {
        output += QString("[+] LLA address %1 added to %2.\n")
                    .arg(addr, interface);
        output += "[i] You can now click 'Discover / Refresh' on the "
                  "Camera tab.\n";
        return true;
    }
    output += "[!] Failed to add LLA address (see message above).\n";
    return false;
#else
    output = "[!] LLA compatibility mode is only supported on Linux.\n";
    return false;
#endif
}

NetworkConfigurator::LinkStatus
NetworkConfigurator::getLinkStatus(const QString &interface) {
#if defined(Q_OS_LINUX)
    // Preferred source: carrier. Note that reading /carrier on an
    // interface whose link is administratively down returns EINVAL,
    // so we must handle a read failure and fall back to operstate.
    QFile carrier(QString("/sys/class/net/%1/carrier").arg(interface));
    if (carrier.open(QIODevice::ReadOnly)) {
        const QByteArray value = carrier.readAll().trimmed();
        if (value == "1") return LinkStatus::Up;
        if (value == "0") return LinkStatus::Down;
    }

    QFile operstate(QString("/sys/class/net/%1/operstate").arg(interface));
    if (operstate.open(QIODevice::ReadOnly)) {
        const QByteArray value = operstate.readAll().trimmed();
        if (value == "up")                return LinkStatus::Up;
        if (value == "down" ||
            value == "lowerlayerdown" ||
            value == "notpresent")        return LinkStatus::Down;
    }
    return LinkStatus::Unknown;
#else
    Q_UNUSED(interface);
    return LinkStatus::Unknown;
#endif
}
