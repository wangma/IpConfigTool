#ifndef NETWORKCONFIGURATOR_H
#define NETWORKCONFIGURATOR_H

#include <QString>

class NetworkConfigurator {
public:
    // Physical link state of a NIC. Determined on Linux by reading
    // /sys/class/net/<iface>/carrier and, as a fallback, operstate.
    // On other platforms, always returns Unknown.
    enum class LinkStatus {
        Unknown,
        Down,
        Up
    };

    // Apply IPv4 settings to a NIC.
    // Linux: uses pkexec + iproute2. Windows: uses netsh.
    static bool setInterfaceIp(const QString &interface,
                               const QString &ip,
                               const QString &mask,
                               const QString &gateway,
                               QString &output);

    // Add a Link-Local (169.254.0.0/16) address to the given NIC so that
    // Aravis' GigE discovery can reach cameras that are in LLA state.
    // Linux only. Uses pkexec + iproute2.
    static bool addLlaAddress(const QString &interface,
                              QString &output);

    // Read-only, root-free query of the physical link state of a NIC.
    static LinkStatus getLinkStatus(const QString &interface);

private:
    static int maskToPrefix(const QString &mask);
};

#endif // NETWORKCONFIGURATOR_H
