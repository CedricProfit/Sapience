#ifndef ANDROIDTTSCLIENT_H
#define ANDROIDTTSCLIENT_H

#include <QtAndroidExtras/QAndroidJniObject>
#include <QObject>

class AndroidTTSClient: public QObject
{
    Q_OBJECT
public:
    explicit AndroidTTSClient(QObject *parent = 0);
    Q_INVOKABLE void Say(QString msg );

};


#endif // ANDROIDTTSCLIENT_H
