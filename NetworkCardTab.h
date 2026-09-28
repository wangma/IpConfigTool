#ifndef NETWORKCARDTAB_H
#define NETWORKCARDTAB_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class NetworkCardTab; }
QT_END_NAMESPACE

class NetworkCardTab : public QWidget {
    Q_OBJECT
public:
    explicit NetworkCardTab(QWidget *parent = nullptr);
    ~NetworkCardTab() override;

private slots:
    void refreshInterfaces();
    void onInterfaceSelected();
    void applyConfiguration();
    void enableLlaForSelected();

private:
    Ui::NetworkCardTab *ui;
};

#endif // NETWORKCARDTAB_H
