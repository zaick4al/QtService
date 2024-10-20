#ifndef SYSTEMDSERVICEPLUGIN_H
#define SYSTEMDSERVICEPLUGIN_H

#include <serviceplugin.h>

class SystemdServicePlugin : public QObject, public QtService::ServicePlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QtService_ServicePlugin_Iid FILE "systemd.json")
	Q_INTERFACES(QtService::ServicePlugin)

public:
	SystemdServicePlugin(QObject *parent = nullptr);

	QString findServiceId(const QString &backend, const QString &serviceName, const QString &domain) const override;
	QtService::ServiceBackend *createServiceBackend(const QString &backend, QtService::Service *service) override;
	QtService::ServiceControl *createServiceControl(const QString &backend, QString &&serviceId, QObject *parent) override;
};

#endif // SYSTEMDSERVICEPLUGIN_H
