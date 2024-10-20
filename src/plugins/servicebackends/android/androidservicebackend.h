#ifndef ANDROIDSERVICEBACKEND_H
#define ANDROIDSERVICEBACKEND_H

#include <QtCore/QLoggingCategory>

#include <QtService/ServiceBackend>
#include <QtCore/private/qandroidextras_p.h>

class AndroidServiceBackend : public QtService::ServiceBackend
{
	Q_OBJECT

public:
	explicit AndroidServiceBackend(QtService::Service *service);

	int runService(int &argc, char **argv, int flags) override;
	void quitService() override;
	void reloadService() override;

	// helper stuff
	static jint JNICALL callStartCommand(JNIEnv *env, jobject object, jobject intent, jint flags, jint startId, jint oldId);
	static jboolean JNICALL exitService(JNIEnv *env, jobject object);

protected Q_SLOTS:
    void onStopped(int exitCode) override;

private Q_SLOTS:
	void onStarted(bool success);
	void onExit();

private:
	static QPointer<AndroidServiceBackend> _backendInstance;
    QJniObject _javaService;
	bool _startupFailed = false;

	QAndroidBinder* onBind(const QAndroidIntent &intent);
};

Q_DECLARE_LOGGING_CATEGORY(logBackend)

#endif // ANDROIDSERVICEBACKEND_H
