#include <QCoreApplication>
#include <QString>
#include "secs_gem.h"
#include <variant>
#include <QDebug>
#include <qudpsocket.h>

#define Debug(x) qDebug() << #x << " ==> " << (x);

using namespace Secs;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString msg = "00 00 00 1f 00 ff 01 0e 00 00 00 00 00 02 01 02 25 01 00 01 02 41 03 4a 4e 47 41 07 31 2e 30 2e 31 2e 31 ";
    msg = msg.split(" ").join("");
    QByteArray data = QByteArray::fromHex(msg.toLocal8Bit());
    Message message;
    message.decode(data);
    try {
        ItemPtr list = message.getItem(0);
        Visitor v;
        std::visit(v, list->values[0]);
        Debug(list->getList()->getBool());
        Debug(list->getList(1)->getList()->getString().c_str());
        Debug(list->getList(1)->getList(1)->getString().c_str());
    } catch (const std::bad_variant_access& e) {
        qDebug() << "fail !!!";
    }

    ItemPtr item1 = Item::Bool();
    item1->append(false);

    ItemPtr item2 = Item::List();
    item2->append(item1);

    ItemPtr item3 = Item::List();
    item3->append(item2);

    Message msgObj;
    msgObj.appendItem(item3);

    Message msgObj2;
    QByteArray data1 = msgObj.encode();

    msgObj2.decode(data1);

    ItemPtr list = msgObj2.getItem(0);
    try
    {
        Debug(list->getList(0)->getList(0)->getBool(0));
    }
    catch (const std::bad_variant_access&e)
    {
        Debug(e.what());
    }

    return a.exec();
}
