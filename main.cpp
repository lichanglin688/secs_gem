#include <QCoreApplication>
#include <QString>
#include "secs_gem.h"
#include <variant>
#include <QDebug>

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
        //Visitor v;
        //std::visit(v, list->values[0]);
        Debug(list->getList()->getBool());
        Debug(list->getList(1)->getList()->getString().c_str());
        Debug(list->getList(1)->getList(1)->getString().c_str());
    } catch (const std::bad_variant_access& e) {
        qDebug() << "fail !!! " << e.what();
    }

    ItemPtr item1 = Item::Bool({ true });
    item1->append(false);
    //item1->append(1);

    ItemPtr item2 = Item::List();
    item2->append(item1);

    ItemPtr item3 = Item::I1({3});
    ItemPtr item4 = Item::I2({4});
    ItemPtr item5 = Item::I4({5});
    ItemPtr item6 = Item::I8({6});
    ItemPtr item7 = Item::Binary({7});
    ItemPtr item8 = Item::U1({8});
    ItemPtr item9 = Item::U2({9});
    ItemPtr item10 = Item::U4({10});
    ItemPtr item11 = Item::U8({11});
    ItemPtr item12 = Item::F4({12});
    ItemPtr item13 = Item::F8({13});


    Message msgObj;
    msgObj.appendItem(item1);
    msgObj.appendItem(item2);
    msgObj.appendItem(item3);
    msgObj.appendItem(item4);
    msgObj.appendItem(item5);
    msgObj.appendItem(item6);
    msgObj.appendItem(item7);
    msgObj.appendItem(item8);
    msgObj.appendItem(item9);
    msgObj.appendItem(item10);
    msgObj.appendItem(item11);
    msgObj.appendItem(item12);
    msgObj.appendItem(item13);

    Message msgObj2;
    msgObj2.decode(msgObj.encode());

    try
    {
        Debug(msgObj2.getItem(0)->getBool(1));
        Debug(msgObj2.getItem(1)->getList()->getBool());
        Debug(msgObj2.getItem(2)->getI1());
        Debug(msgObj2.getItem(3)->getI2());
        Debug(msgObj2.getItem(4)->getI4());
        Debug(msgObj2.getItem(5)->getI8());
        Debug(msgObj2.getItem(6)->getBinary());
        Debug(msgObj2.getItem(7)->getU1());
        Debug(msgObj2.getItem(8)->getU2());
        Debug(msgObj2.getItem(9)->getU4());
        Debug(msgObj2.getItem(10)->getU8());
        Debug(msgObj2.getItem(11)->getF4());
        Debug(msgObj2.getItem(12)->getF8());
    }
    catch (const std::bad_variant_access&e)
    {
        Debug(e.what());
    }

    return a.exec();
}
