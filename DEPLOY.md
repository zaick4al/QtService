Для развёртывания службы с использованием QtService необходимо включить QtService в приложение, подключить его согласно README.md и зарегистрировать его в списке служб системы

Для этого необходимо:
### Windows:
1. Добавить службу в список служб Windows
```bash
  sc.exe create <НАЗВАНИЕ_СЛУЖБЫ> binPath="<АБСОЛЮТНЫЙ_ПУТЬ_К_ИСПОЛНЯЕМОМУ_ФАЙЛУ> --backend windows"
```
Названием службы является название приложения, указанное либо в CMakeLists.txt проекта, либо в QCoreApplication::setApplicationName(const QString &name)

2. Запустить его. Для этого:

    2.1. Используя Ctrl + R и вписав services.msc открыть диспетчер служб Windows

    2.2. Запустить службу через диспетчер

### Linux
1. (ОБЯЗАТЕЛЬНО!) Объявить в коде название службы через QCoreApplication::setApplicationName(const QString &name)
2. В /etc/systemd/system/ создать файл <НАЗВАНИЕ_СЛУЖБЫ>.service с минимальным содержимым для запуска службы
Пример содержимого .service файла:
```ini
[Unit]
Description=QtService Example
After=network-online.target echoservice.socket

[Service]
ExecStart=$${target.path}/$$TARGET --backend systemd

[Install]
WantedBy=default.target
```
3. В /etc/dbus-1/system.d/ создать файл <НАЗВАНИЕ_СЛУЖБЫ>.conf с содержимым
Пример .conf файла:
```xml
<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-Bus Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
  <policy context="default">
    <allow own="<НАЗВАНИЕ_СЛУЖБЫ>.systemd-service"/>
    <allow send_destination="<НАЗВАНИЕ_СЛУЖБЫ>.systemd-service"/>
  </policy>
</busconfig>
```
4. Запустить службу через консоль:
```bash
    sudo systemctl enable <НАЗВАНИЕ_СЛУЖБЫ>
    sudo systemctl start <НАЗВАНИЕ_СЛУЖБЫ>
```
