#ifndef ANDROIDSERVICEPLUGIN_H
#define ANDROIDSERVICEPLUGIN_H

#include <QtService/ServicePlugin>

#include <QtCore/private/qandroidextras_p.h>

class AndroidServicePlugin : public QObject, public QtService::ServicePlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QtService_ServicePlugin_Iid FILE "android.json")
	Q_INTERFACES(QtService::ServicePlugin)

public:
	AndroidServicePlugin(QObject *parent = nullptr);

	QString findServiceId(const QString &backend,const QString &serviceName, const QString &domain) const override;
	QtService::ServiceBackend *createServiceBackend(const QString &backend, QtService::Service *service) override;
	QtService::ServiceControl *createServiceControl(const QString &backend, QString &&serviceId, QObject *parent) override;
};

Q_DECLARE_METATYPE(QAndroidBinder*)
Q_DECLARE_METATYPE(QAndroidIntent)
Q_DECLARE_METATYPE(QAndroidServiceConnection*)

Q_DECLARE_METATYPE(QtAndroidPrivate::BindFlags)

#endif // ANDROIDSERVICEPLUGIN_H
