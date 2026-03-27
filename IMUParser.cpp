#include "IMUParser.h"
#include <QDebug>

void IMUParser::inputPacket(const QByteArray &packet,double timestamp )
{
    (void)timestamp;
    if(!isFirst){
         qDebug()<<"IMUParser" << "ip:"<<m_ip <<"port"<<packet;
         isFirst = true;
         return;

    }


}
