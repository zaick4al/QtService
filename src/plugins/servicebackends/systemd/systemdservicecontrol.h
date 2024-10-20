#ifndef SYSTEMDSERVICECONTROL_H
#define SYSTEMDSERVICECONTROL_H

#include <QtCore/QLoggingCategory>

#include <servicecontrol.h>

class SystemdServiceControl : public QtService::ServiceControl
{
	Q_OBJECT

	Q_PROPERTY(bool runAsUser READ isRunAsUser WRITE setRunAsUser RESET resetRunAsUser NOTIFY runAsUserChanged)

public:
	explicit SystemdServiceControl(QString &&serviceId, QObject *parent = nullptr);

	QString backend() const override;
	SupportFlags supportFlags() const override;
	bool serviceExists() const override;
	Status status() const override;
	bool isAutostartEnabled() const override;
	BlockMode blocking() const override;
	bool isRunAsUser() const;

	QVariant callGenericCommand(const QByteArray &kind, const QVariantList &args) override;

public Q_SLOTS:
	bool start() override;
	bool stop() override;
	bool restart() override;
	bool reload() override;
	bool enableAutostart() override;
	bool disableAutostart() override;
	bool setBlocking(bool blocking) override;
	void setRunAsUser(bool runAsUser);
	void resetRunAsUser();

Q_SIGNALS:
	void runAsUserChanged(bool runAsUser);

protected:
	QString serviceName() const override;

private:
	enum class SvcExists {
		Unknown = -1,
		Yes = true,
		No = false
	};
	mutable SvcExists _svcInfo = SvcExists::Unknown;
	bool _blocking = true;
	bool _runAsUser;

	int runSystemctl(const QByteArray &command,
					 const QStringList &extraArgs = {},
					 QByteArray *outData = nullptr,
					 bool noPrepare = false) const;
};

Q_DECLARE_LOGGING_CATEGORY(logControl)

#endif // SYSTEMDSERVICECONTROL_H
