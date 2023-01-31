#include "secs_gem.h"
#include <QDataStream>
#include <QDebug>
#include <QtEndian>

namespace Secs{
QByteArray Message::encode()
{
    QByteArray data;
    QDataStream in(&data, QIODevice::WriteOnly);
    in.setFloatingPointPrecision(QDataStream::SinglePrecision);

    in << quint32(0);
    inputMessageHeader(in);
    for(auto && item : items)
    {
        inputItem(in, item);
    }
    int size = qToBigEndian(data.size()) - 4;
    data.replace(0, 4, (char*)&size, 4);
    return data;
}

void Message::decode(QByteArray data)
{
    QDataStream out(&data, QIODevice::ReadOnly);
    out.setFloatingPointPrecision(QDataStream::SinglePrecision);

    int length = 0;
    out >> length;
    outMessageHeader(out);
    while (!out.atEnd())
    {
        items.emplace_back(outItem(out));
    }
}

ItemPtr Message::getItem(size_t index)
{
    return items.at(index);
}

void Message::appendItem(ItemPtr item)
{
    items.emplace_back(item);
}

void Message::inputMessageHeader(QDataStream &in)
{
    in << deviceID;
    in << getStreamByte();
    in << f;
    in << pType;
    in << sType;
    in << systemBytes;
}

uint8_t Message::getStreamByte()
{
    return replyExpected ? s | 0x80 : s;
}

void Message::inputItem(QDataStream &in, const ItemPtr &item)
{
    Format format = item->getFormat();
    if(format == Format::NONE)
        return;

    inputItemInfo(in, item);

    for(size_t i = 0; i < item->getCount(); ++i)
    {
        if(format == Format::List)
        {
            inputItem(in, item->getValue<ValueType::Item>(i));
        }
        else if(format == Format::Binary)
        {
            in << item->getValue<ValueType::Binary>(i);
        }
        else if(format == Format::Boolean)
        {
            in << item->getValue<ValueType::Bool>(i);
        }
        else if(format == Format::I1)
        {
            in << item->getValue<ValueType::I1>(i);
        }
        else if(format == Format::I2)
        {
            in << item->getValue<ValueType::I2>(i);
        }
        else if(format == Format::I4)
        {
            in << item->getValue<ValueType::I4>(i);
        }
        else if(format == Format::I8)
        {
            in << item->getValue<ValueType::I8>(i);
        }
        else if(format == Format::U1)
        {
            in << item->getValue<ValueType::U1>(i);
        }
        else if(format == Format::U2)
        {
            in << item->getValue<ValueType::U2>(i);
        }
        else if(format == Format::U4)
        {
            in << item->getValue<ValueType::U4>(i);
        }
        else if(format == Format::U8)
        {
            in << item->getValue<ValueType::U8>(i);
        }
        else if(format == Format::F4)
        {
            in << item->getValue<ValueType::F4>(i);
        }
        else if(format == Format::F8)
        {
            in << item->getValue<ValueType::F8>(i);
        }
    }
}

void Message::inputItemInfo(QDataStream &in, const ItemPtr &item)
{
    Format format = item->getFormat();
    int count = item->getCount();
    if(count <= 0)return;

    if(count <= 0xff)
    {
        in << (ValueType::U1)((int)format | 1);
        in << (ValueType::U1)count;
    }
    else if(count <= 0xffff)
    {
        in << (ValueType::U1)((int)format | 2);
        in << (ValueType::U2)count;
    }
    else if(count <= 0xffffff)
    {
        in << (ValueType::U1)((int)format | 3);
        quint8 temp = count >> 16;
        in << temp;

        temp = count >> 8;
        in << temp;

        temp = count;
        in << temp;
    }
}

void Message::outMessageHeader(QDataStream &out)
{
    out >> deviceID;

    ValueType::U1 byte = 0;
    out >> byte;
    if(byte >= 0x80)
    {
        s = byte - 0x80;
        replyExpected = true;
    }
    else
    {
        s = byte;
        replyExpected = false;
    }
    out >> f;
    out >> pType;
    out >> sType;
    out >> systemBytes;
}

ItemPtr Message::outItem(QDataStream &out)
{
    Format format = Format::NONE;
    int size = 0;
    outItemInfo(out, format, size);
    ItemPtr item = std::make_shared<Item>(format);
    int formatSize = item->formatSize();
    int pos = 0;
    while (pos < size) {
        item->append(outItemValue(out, format));
        pos += formatSize;
    }
    return item;
}

void Message::outItemInfo(QDataStream &out, Format &format, int &size)
{
    ValueType::U1 byte = 0;
    out >> byte;
    format = static_cast<Format>(byte & 0xfc);
    int valueLength = byte & 0x03;
    if(valueLength == 1)
    {
        ValueType::U1 temp = 0;
        out >> temp;
        size = temp;
    }
    else if(valueLength == 2)
    {
        ValueType::U2 temp = 0;
        out >> temp;
        size = temp;
    }
    else if(valueLength == 3)
    {
        int temp = 0;
        quint8 byte = 0;
        out >> byte;

        temp = byte << 16;

        byte = 0;
        out >> byte;
        temp += byte << 8;

        byte = 0;
        out >> byte;
        temp += byte;

        size = temp;
    }
}

Value Message::outItemValue(QDataStream &out, const Format &format)
{
    if(format == Format::List)
    {
        return outItem(out);
    }
    else if(format == Format::Binary)
    {
        ValueType::Binary value = 0;
        out >> value;
        return value;
    }
    else if(format == Format::Boolean)
    {
        ValueType::Bool value = false;
        out >> value;
        return value;
    }
    else if(format == Format::ASCII)
    {
        ValueType::ASCII value = 0;
        out.readRawData(&value, 1);
        return value;
    }
    else if(format == Format::I1)
    {
        ValueType::I1 value = 0;
        out >> value;
        return value;
    }
    else if(format == Format::I2)
    {
        ValueType::I1 value = 0;
        out >> value;
        return value;
    }
    else if(format == Format::I4)
    {
        ValueType::I1 value = 0;
        out >> value;
        return value;
    }
    else if(format == Format::I8)
    {
        ValueType::I1 value = 0;
        out >> value;
        return value;
    }
    else if(format == Format::U1)
    {
        ValueType::I1 value = 0;
        out >> value;
        return value;
    }
    else if(format == Format::U2)
    {
        ValueType::I1 value = 0;
        out >> value;
        return value;
    }
    else if(format == Format::U4)
    {
        ValueType::I1 value = 0;
        out >> value;
        return value;
    }
    else if(format == Format::U8)
    {
        ValueType::I1 value = 0;
        out >> value;
        return value;
    }
    else if(format == Format::F4)
    {
        ValueType::F4 value = 0;
        out >> value;
        return value;
    }
    else if(format == Format::F8)
    {
        ValueType::F8 value = 0;
        out >> value;
        return value;
    }
    return 0;
}


Item::Item(Format format)
    :format(format)
{

}

Item::~Item()
{

}

ItemPtr Item::List()
{
    return std::make_shared<Item>(Format::List);
}

ItemPtr Item::Bool()
{
    return std::make_shared<Item>(Format::Boolean);
}

ItemPtr Item::Binary()
{
    return std::make_shared<Item>(Format::Binary);
}

ItemPtr Item::I1()
{
    return std::make_shared<Item>(Format::I1);
}

ItemPtr Item::I2()
{
    return std::make_shared<Item>(Format::I2);
}

ItemPtr Item::I4()
{
    return std::make_shared<Item>(Format::I4);
}

ItemPtr Item::I8()
{
    return std::make_shared<Item>(Format::I8);
}

ItemPtr Item::U1()
{
    return std::make_shared<Item>(Format::U1);
}

ItemPtr Item::U2()
{
    return std::make_shared<Item>(Format::U2);
}

ItemPtr Item::U4()
{
    return std::make_shared<Item>(Format::U4);
}

ItemPtr Item::U8()
{
    return std::make_shared<Item>(Format::U8);
}

void Item::append(const Value &value)
{
    values.emplace_back(value);
}

size_t Item::getCount()
{
    return values.size();
}

ValueType::ASCII Item::getASCII(size_t index)
{
    return getValue<ValueType::ASCII>(index);
}

string Item::getString()
{
    string str;
    for (auto&& value : values)
    {
        str.push_back(std::get<ValueType::ASCII>(value));
    }
    return str;
}

ValueType::Binary Item::getBinary(size_t index)
{
    return getValue<ValueType::Binary>(index);
}

ValueType::Bool Item::getBool(size_t index)
{
    return getValue<ValueType::Bool>(index);
}

ValueType::I1 Item::getI1(size_t index)
{
    return getValue<ValueType::I1>(index);
}

ValueType::I2 Item::getI2(size_t index)
{
    return getValue<ValueType::I2>(index);
}

ValueType::I4 Item::getI4(size_t index)
{
    return getValue<ValueType::I4>(index);
}

ValueType::I8 Item::getI8(size_t index)
{
    return getValue<ValueType::I8>(index);
}

ValueType::U1 Item::getU1(size_t index)
{
    return getValue<ValueType::U1>(index);
}

ValueType::U2 Item::getU2(size_t index)
{
    return getValue<ValueType::U2>(index);
}

ValueType::U4 Item::getU4(size_t index)
{
    return getValue<ValueType::U4>(index);
}

ValueType::U8 Item::getU8(size_t index)
{
    return getValue<ValueType::U8>(index);
}

ValueType::Item Item::getList(size_t index)
{
    return getValue<ValueType::Item>(index);
}

Format Item::getFormat() const
{
    return format;
}

int Item::formatSize()
{
    int size = 1;
    if(format == Format::I2 || format == Format::U2)
        size = 2;
    else if(format == Format::I4 || format == Format::U4)
        size = 4;
    else if(format == Format::I8 || format == Format::U8)
        size = 8;
    return size;
}

}

