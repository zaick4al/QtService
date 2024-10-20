#ifndef WINDOWSSERVICEBACKEND_H
#define WINDOWSSERVICEBACKEND_H

#include <QMutex>
#include <QWaitCondition>
#include <QThread>
#include <QPointer>
#include <QAbstractNativeEventFilter>
#include <QTimer>
#include <QLoggingCategory>

#include "servicebackend.h"

#include <qt_windows.h>

class WindowsServiceBackend : public QtService::ServiceBackend
{
	Q_OBJECT

public:
	explicit WindowsServiceBackend(QtService::Service *service);

	int runService(int &argc, char **argv, int flags) override;
	void quitService() override;
	void reloadService() override;

private Q_SLOTS:
	void onStarted(bool success);
	void onPaused(bool success);
	void onResumed(bool success);

private:
	class SvcControlThread : public QThread
	{
	public:
		SvcControlThread(WindowsServiceBackend *backend);
	protected:
		void run() override;

	private:
		WindowsServiceBackend *_backend;
	};

	class SvcEventFilter : public QAbstractNativeEventFilter
	{
	public:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
#else
        bool nativeEventFilter(const QByteArray &eventType, void *message, long long *result);
#endif
	};

	static QPointer<WindowsServiceBackend> _backendInstance;

	QMutex _svcLock;
	QWaitCondition _startCondition;

	SERVICE_STATUS _status;
	SERVICE_STATUS_HANDLE _statusHandle = nullptr;

	//temporary stuff
	QByteArrayList _svcArgs;
	QTimer *_opTimer = nullptr;

	void setStatus(DWORD status);

	static void WINAPI serviceMain(DWORD dwArgc, wchar_t** lpszArgv);
	static void WINAPI handler(DWORD dwOpcode);

	static void winsvcMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message);
};

Q_DECLARE_LOGGING_CATEGORY(logBackend)

#endif // WINDOWSSERVICEBACKEND_H
