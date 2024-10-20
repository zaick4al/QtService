#ifndef STANDARDSERVICECONTROL_H
#define STANDARDSERVICECONTROL_H

#include <QtCore/QLockFile>
#include <QtCore/QLoggingCategory>

#include <servicecontrol.h>

class StandardServiceControl : public QtService::ServiceControl
{
	Q_OBJECT

public:
	explicit StandardServiceControl(bool debugMode, QString &&serviceId, QObject *parent = nullptr);

	QString backend() const override;
	SupportFlags supportFlags() const override;
	bool serviceExists() const override;
	Status status() const override;
	BlockMode blocking() const override;

	QVariant callGenericCommand(const QByteArray &kind, const QVariantList &args) override;

public Q_SLOTS:
	bool start() override;
	bool stop() override;

protected:
	QString serviceName() const override;

private:
	const bool _debugMode;

	QSharedPointer<QLockFile> statusLock() const;
	qint64 getPid();
};

Q_DECLARE_LOGGING_CATEGORY(logControl)

#endif // STANDARDSERVICECONTROL_H
