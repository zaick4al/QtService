#include "androidservicebackend.h"
#include "androidserviceplugin.h"
#include <QtCore/QPointer>
#include <QtCore/QDebug>

#include <QtCore/private/qandroidextras_p.h>
using namespace QtService;

Q_LOGGING_CATEGORY(logBackend, "qt.service.plugin.android.backend")

QPointer<AndroidServiceBackend> AndroidServiceBackend::_backendInstance = nullptr;

AndroidServiceBackend::AndroidServiceBackend(Service *service) :
	ServiceBackend{service}
{
	_backendInstance = this;
}

int AndroidServiceBackend::runService(int &argc, char **argv, int flags)
{
	QAndroidService app(argc, argv,
						std::bind(&AndroidServiceBackend::onBind, this, std::placeholders::_1),
						flags);
	if (!preStartService())
		return EXIT_FAILURE;

	//NOTE check if onStartCommand is supported with QAndroidService yet
	qCDebug(logBackend) << "registering service JNI natives";
    _javaService = QtAndroidPrivate::service();
    QJniEnvironment env;
	static const JNINativeMethod methods[] = {
		{"callStartCommand", "(Landroid/content/Intent;III)I", reinterpret_cast<void*>(&AndroidServiceBackend::callStartCommand)},
		{"exitService", "()Z", reinterpret_cast<void*>(&AndroidServiceBackend::exitService)}
	};
	env->RegisterNatives(env->FindClass("de/skycoder42/qtservice/AndroidService"),
						 methods,
						 sizeof(methods)/sizeof(JNINativeMethod));
	qCDebug(logBackend) << "Continue service startup";
	_javaService.callMethod<void>("nativeReady");

	// handle start result
	connect(service(), QOverload<bool>::of(&Service::started),
			this, &AndroidServiceBackend::onStarted);

	// start the eventloop
	QMetaObject::invokeMethod(this, "processServiceCommand", Qt::QueuedConnection,
							  Q_ARG(QtService::ServiceBackend::ServiceCommand, ServiceCommand::Start));
	return app.exec();
}

void AndroidServiceBackend::quitService()
{
    QJniEnvironment env;
    env.checkAndClearExceptions();
	_javaService.callMethod<void>("stopSelf");
}

void AndroidServiceBackend::reloadService()
{
	processServiceCommand(ServiceCommand::Reload);
}

jint AndroidServiceBackend::callStartCommand(JNIEnv *, jobject, jobject intent, jint flags, jint startId, jint oldId)
{
	qCDebug(logBackend) << "JNI callStartCommand on" << _backendInstance;
	if (_backendInstance) {
		auto var = _backendInstance->processServiceCallbackImpl("onStartCommand", QVariantList {
																	QVariant::fromValue(QAndroidIntent{intent}),
																	static_cast<int>(flags),
																	static_cast<int>(startId)
																});
		auto ok = false;
		auto res = var.toInt(&ok);
		if (ok)
			return res;
	}

	return oldId;
}

jboolean AndroidServiceBackend::exitService(JNIEnv *, jobject)
{
	qCDebug(logBackend) << "JNI exitService on" << _backendInstance;
	if (_backendInstance)
		return QMetaObject::invokeMethod(_backendInstance, "onExit", Qt::QueuedConnection);
	else
		return false;
}

void AndroidServiceBackend::onStarted(bool success)
{
	if (!success) {
		_startupFailed = true;
		quitService();
	}
}

void AndroidServiceBackend::onExit()
{
	if (_startupFailed)
		onStopped(EXIT_FAILURE);
	else {
		connect(service(), &Service::stopped,
				this, &AndroidServiceBackend::onStopped,
				Qt::UniqueConnection);
		processServiceCommand(ServiceCommand::Stop);
	}
}

void AndroidServiceBackend::onStopped(int exitCode)
{
	qCInfo(logBackend) << "QAndroidService exited with code:" << exitCode;
	_javaService.callMethod<void>("nativeExited");
}

QAndroidBinder *AndroidServiceBackend::onBind(const QAndroidIntent &intent)
{
	return processServiceCallback<QAndroidBinder*>("onBind", intent);
}
