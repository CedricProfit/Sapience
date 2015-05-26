#include <QDebug>

#include "androidttsclient.h"



AndroidTTSClient::AndroidTTSClient(QObject *parent)
    : QObject(parent)
{
    Say("Hi");
}

void AndroidTTSClient::Say(QString msg ){
    QAndroidJniObject javaMessage = QAndroidJniObject::fromString(msg);
    QAndroidJniObject::callStaticMethod<void>("com/blockchainsingularity/apps/sapience_qt/AndroidTTSClient",
                                              "say",
                                              "(Ljava/lang/String;)V",
                                              javaMessage.object<jstring>());
}
