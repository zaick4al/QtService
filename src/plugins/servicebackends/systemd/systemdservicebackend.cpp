#include "systemdservicebackend.h"
#include "systemdserviceplugin.h"

#include <QtCore/QCommandLineParser>

#include <chrono>
#include <csignal>
#include <unistd.h>

#define SD_JOURNAL_SUPPRESS_LOCATION
#include <systemd/sd-journal.h>
#include <systemd/sd-daemon.h>

using namespace QtService;

Q_LOGGING_CATEGORY(logBackend, "qt.service.plugin.systemd.backend")

const QString SystemdServiceBackend::DBusObjectPath = QStringLiteral("/de/skycoder42/QtService/SystemdServiceBackend");

SystemdServiceBackend::SystemdServiceBackend(Service *service) :
	ServiceBackend{service},
	_dbusAdapter{new SystemdAdaptor{this}}
{
	// manually connect signals from service to adaptor
	connect(service, &Service::reloaded,
			_dbusAdapter, &SystemdAdaptor::serviceReloaded);
	connect(service, &Service::stopped,
			_dbusAdapter, &SystemdAdaptor::serviceStopped);
}

int SystemdServiceBackend::runService(int &argc, char **argv, int flags)
{
	qInstallMessageHandler(SystemdServiceBackend::systemdMessageHandler);
	QCoreApplication app{argc, argv, flags};

	QCommandLineParser parser;
	parser.addHelpOption();
	{
		QCommandLineOption backendOpt{QStringLiteral("backend"), {}, QStringLiteral("backend")};
		backendOpt.setFlags(QCommandLineOption::HiddenFromHelp);
		parser.addOption(backendOpt);
		QCommandLineOption terminalOpt{QStringLiteral("terminal")};
		terminalOpt.setFlags(QCommandLineOption::HiddenFromHelp);
		parser.addOption(terminalOpt);
	}
	parser.addOption({
						 QStringLiteral("system"),
						 tr("Run the service as system service, independend of the current user id")
					 });
	parser.addOption({
						 QStringLiteral("user"),
						 tr("Run the service as user service, independend of the current user id")
					 });
	parser.addPositionalArgument(QStringLiteral("command"),
								 tr("A command to execute on the primary service process. "
									"Can be either 'reload' to let the service reload its configuration "
									"or 'stop' to halt the service"),
								 QStringLiteral("[stop|reload]"));

	parser.process(app);
	if (parser.isSet(QStringLiteral("user")))
		_userService = true;
	else if (parser.isSet(QStringLiteral("system")))
		_userService = false;
	else
		_userService = ::geteuid() != 0;

	if (parser.positionalArguments().startsWith(QStringLiteral("stop")))
		return stop();
	else if (parser.positionalArguments().startsWith(QStringLiteral("reload")))
		return reload();
	else
		return run();
}

void SystemdServiceBackend::quitService()
{
	connect(service(), &Service::stopped,
			this, &SystemdServiceBackend::onStopped,
			Qt::UniqueConnection);
	sd_notify(false, "STOPPING=1");
	processServiceCommand(ServiceCommand::Stop);
}

void SystemdServiceBackend::reloadService()
{
	sd_notify(false, "RELOADING=1");
	processServiceCommand(ServiceCommand::Reload);
}

QList<int> SystemdServiceBackend::getActivatedSockets(const QByteArray &name)
{
	if (_sockets.isEmpty()) {
		char **namesRaw = nullptr;
		auto cnt = sd_listen_fds_with_names(false, &namesRaw);
		QScopedPointer<char*, QScopedPointerPodDeleter> names{namesRaw};
		if (cnt > 0) {
			_sockets.reserve(cnt);
			for (auto i = 0; i < cnt; i++) {
				QByteArray sockName;
				if (names.data()[i])
					sockName = names.data()[i];
				_sockets.insert(sockName, SD_LISTEN_FDS_START + i);
			}
		}
	}

	if (name.isNull())
		return _sockets.isEmpty() ? QList<int>{} : QList<int>{SD_LISTEN_FDS_START};
	else
		return _sockets.values(name);
}

void SystemdServiceBackend::signalTriggered(int signal)
{
	qCDebug(logBackend) << "Processing signal" << signal;
	switch(signal) {
	case SIGINT:
	case SIGTERM:
	case SIGQUIT:
		quitService();
		break;
	case SIGHUP:
		reloadService();
		break;
	case SIGTSTP:
		processServiceCommand(ServiceCommand::Pause);
		break;
	case SIGCONT:
		processServiceCommand(ServiceCommand::Resume);
		break;
	case SIGUSR1:
		processServiceCallback("SIGUSR1");
		break;
	case SIGUSR2:
		processServiceCallback("SIGUSR2");
		break;
	default:
		ServiceBackend::signalTriggered(signal);
		break;
	}
}

void SystemdServiceBackend::sendWatchdog()
{
	sd_notify(false, "WATCHDOG=1");
}

void SystemdServiceBackend::onStarted(bool success)
{
	if (success)
		sd_notify(false, "READY=1");
	else
		onStopped(EXIT_FAILURE);
}

void SystemdServiceBackend::onReloaded(bool success)
{
	Q_UNUSED(success)
	sd_notify(false, "READY=1");
}

void SystemdServiceBackend::onStopped(int exitCode)
{
	if (_watchdogTimer)
		_watchdogTimer->stop();
	qApp->exit(exitCode);
}

void SystemdServiceBackend::onPaused(bool success)
{
	if (success)
		kill(getpid(), SIGSTOP);  // now actually stop
}

int SystemdServiceBackend::run()
{
	// prepare the app
	prepareWatchdog();  // do as early as possible
	if (!preStartService())
		return EXIT_FAILURE;

	connect(service(), QOverload<bool>::of(&Service::started),
			this, &SystemdServiceBackend::onStarted);
	connect(service(), QOverload<bool>::of(&Service::reloaded),
			this, &SystemdServiceBackend::onReloaded);
	connect(service(), QOverload<bool>::of(&Service::paused),
			this, &SystemdServiceBackend::onPaused);

	for (const auto signal : {SIGINT, SIGTERM, SIGQUIT, SIGHUP, SIGTSTP, SIGCONT, SIGUSR1, SIGUSR2})
		registerForSignal(signal);

	// register the D-Bus service
	auto connection = dbusConnection();
	if (!connection.registerObject(DBusObjectPath, this)) {
		printDbusError(connection.lastError());
		return EXIT_FAILURE;
	}
	const auto dId = dbusId();
	if (!connection.registerService(dId)) {
		printDbusError(connection.lastError());
		return EXIT_FAILURE;
	}
	qCInfo(logBackend) << "Activated D-BUS control service as" << dId;

	// start the eventloop
	QMetaObject::invokeMethod(this, "processServiceCommand", Qt::QueuedConnection,
							  Q_ARG(QtService::ServiceBackend::ServiceCommand, ServiceCommand::Start));
	return QCoreApplication::exec();
}

int SystemdServiceBackend::stop()
{
	using namespace de::skycoder42::QtService::ServicePlugin;
	sd_notify(false, "STOPPING=1");

	auto connection = dbusConnection();
	auto dbusInterface = new systemd{dbusId(), DBusObjectPath, connection, this};
	if (!dbusInterface->isValid()) {
		printDbusError(connection.lastError());
		return EXIT_FAILURE;
	}

	qCDebug(logBackend) << "Sending stop signal to main process via D-BUS service"
						<< dbusInterface->service();
	// forward service stop result
	connect(dbusInterface, &systemd::serviceStopped,
			qApp, &QCoreApplication::exit);
	// handle service removal, in case stop result is not received
	connect(connection.interface(), &QDBusConnectionInterface::serviceUnregistered,
			this, [this](const QString &svcName) {
		if(svcName == dbusId()) {
			qCWarning(logBackend) << "D-Bus Service of main process was removed before the stop result was received! Assuming successful exit";
			qApp->quit();
		}
	}, Qt::QueuedConnection);
	// send the stop request and handle possible direct failures
	auto watcher = new QDBusPendingCallWatcher{dbusInterface->quitService(), this};
	connect(watcher, &QDBusPendingCallWatcher::finished,
			this, [this](QDBusPendingCallWatcher *w){
		if (w->isError()) {
			printDbusError(w->error());
			qApp->exit(EXIT_FAILURE);
		}
	});
	// run eventloop until done
	return QCoreApplication::exec();
}

int SystemdServiceBackend::reload()
{
	using namespace de::skycoder42::QtService::ServicePlugin;
	sd_notify(false, "RELOADING=1");

	auto connection = dbusConnection();
	auto dbusInterface = new systemd{dbusId(), DBusObjectPath, connection, this};
	if (!dbusInterface->isValid()) {
		printDbusError(connection.lastError());
		return EXIT_FAILURE;
	}

	qCDebug(logBackend) << "Sending reload signal to main process via D-BUS service"
						<< dbusInterface->service();
	// forward service reload result
	connect(dbusInterface, &systemd::serviceReloaded,
			qApp, [](bool success) {
		qApp->exit(success ? EXIT_SUCCESS : EXIT_FAILURE);
	});
	// send the reload request and handle possible direct failures
	auto watcher = new QDBusPendingCallWatcher{dbusInterface->reloadService(), this};
	connect(watcher, &QDBusPendingCallWatcher::finished,
			this, [this](QDBusPendingCallWatcher *w){
		if (w->isError()) {
			printDbusError(w->error());
			qApp->exit(EXIT_FAILURE);
		}
	});
	// run eventloop until done
	return QCoreApplication::exec();
}

void SystemdServiceBackend::prepareWatchdog()
{
	using namespace std::chrono;
	using namespace std::chrono_literals;
	uint64_t usec = 0;
	if (sd_watchdog_enabled(false, &usec) > 0) {
		auto mSecs = duration_cast<milliseconds>(microseconds{usec} / 2);
		if (mSecs.count() <= 1) {
			qCWarning(logBackend) << "Watchdog timout <= 1 millisecond - QTimer does not support intervals that small!";
			mSecs = 1ms;
		}

		_watchdogTimer = new QTimer(this);
		_watchdogTimer->setTimerType(Qt::PreciseTimer);
		_watchdogTimer->setInterval(mSecs);
		connect(_watchdogTimer, &QTimer::timeout,
				this, &SystemdServiceBackend::sendWatchdog);
		_watchdogTimer->start();
		qCDebug(logBackend) << "Enabled watchdog with an interval of" << mSecs.count() << "ms";
	}
}

QDBusConnection SystemdServiceBackend::dbusConnection() const
{
	if (_userService)
		return QDBusConnection::sessionBus();
	else
		return QDBusConnection::systemBus();
}

QString SystemdServiceBackend::dbusId() const
{
	auto dId = QCoreApplication::organizationDomain();
	if (dId.isEmpty())
		dId = QCoreApplication::applicationName();
	else
		dId = dId + QLatin1Char('.') + QCoreApplication::applicationName();
	return dId + QStringLiteral(".systemd-service");
}

void SystemdServiceBackend::printDbusError(const QDBusError &error) const
{
	if (error.type() != QDBusError::NoError) {
		qCCritical(logBackend).noquote().nospace() << error.name() << ": "
												   << error.message();
	}
}

void SystemdServiceBackend::systemdMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
	auto formattedMessage = qFormatLogMessage(type, context, message);

	int priority;
	switch (type) {
	case QtDebugMsg:
		priority = LOG_DEBUG;
		break;
	case QtInfoMsg:
		priority = LOG_INFO;
		break;
	case QtWarningMsg:
		priority = LOG_WARNING;
		break;
	case QtCriticalMsg:
		priority = LOG_CRIT;
		break;
	case QtFatalMsg:
		priority = LOG_ALERT;
		break;
	default:
		Q_UNREACHABLE();
		break;
	}

	sd_journal_send("MESSAGE=%s", formattedMessage.toUtf8().constData(),
					"PRIORITY=%i", priority,
					"CODE_FUNC=%s", context.function ? context.function : "unknown",
					"CODE_LINE=%d", context.line,
					"CODE_FILE=%s", context.file ? context.file : "unknown",
					"QT_CATEGORY=%s", context.category ? context.category : "unknown",
					nullptr);
}
