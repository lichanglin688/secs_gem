#include "secs_gem.h"
#include <QDataStream>
#include <QDebug>
#include <QtEndian>

namespace Secs{
QByteArray Message::encode()
{
    QByteArray data;
    QDataStream in(&data, QIODevice::WriteOnly);

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
        else if (format == Format::F4)
        {
            in << item->getValue<ValueType::F4>(i);
        }
        else if (format == Format::F8)
        {
            in << item->getValue<ValueType::F8>(i);
        }
    }
}

void Message::inputItemInfo(QDataStream &in, const ItemPtr &item)
{
    Format format = item->getFormat();
    int dataLength = item->dataLength();
    if(dataLength <= 0)return;

    if(dataLength <= 0xff)
    {
        in << (ValueType::U1)((int)format | 1);
        in << (ValueType::U1)dataLength;
    }
    else if(dataLength <= 0xffff)
    {
        in << (ValueType::U1)((int)format | 2);
        in << (ValueType::U2)dataLength;
    }
    else if(dataLength <= 0xffffff)
    {
        in << (ValueType::U1)((int)format | 3);
        quint8 temp = dataLength >> 16;
        in << temp;

        temp = dataLength >> 8;
        in << temp;

        temp = dataLength;
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
        ValueType::I2 value = 0;
        out >> value;
        return value;
    }
    else if(format == Format::I4)
    {
        ValueType::I4 value = 0;
        out >> value;
        return value;
    }
    else if(format == Format::I8)
    {
        ValueType::I8 value = 0;
        out >> value;
        return value;
    }
    else if(format == Format::U1)
    {
        ValueType::U1 value = 0;
        out >> value;
        return value;
    }
    else if(format == Format::U2)
    {
        ValueType::U2 value = 0;
        out >> value;
        return value;
    }
    else if(format == Format::U4)
    {
        ValueType::U4 value = 0;
        out >> value;
        return value;
    }
    else if(format == Format::U8)
    {
        ValueType::U8 value = 0;
        out >> value;
        return value;
    }
    else if (format == Format::F4)
    {
        ValueType::F4 value = 0;
        out >> value;
        return value;
    }
    else if (format == Format::F8)
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
ItemPtr Item::List(initializer_list<ValueType::Item> values)
{
    ItemPtr item = std::make_shared<Item>(Format::List);
    for (auto&& value : values)
    {
        item->append(value);
    }
    return item;
}

ItemPtr Item::Bool(initializer_list<ValueType::Bool> values)
{
    ItemPtr item = std::make_shared<Item>(Format::Boolean);
    for (auto&& value : values)
    {
        item->append(value);
    }
    return item;
}

ItemPtr Item::Binary(initializer_list<ValueType::Binary> values)
{
    ItemPtr item = std::make_shared<Item>(Format::Binary);
    for (auto&& value : values)
    {
        item->append(value);
    }
    return item;
}

ItemPtr Item::I1(initializer_list<ValueType::I1> values)
{
    ItemPtr item = std::make_shared<Item>(Format::I1);
    for (auto&& value : values)
    {
        item->append(value);
    }
    return item;
}

ItemPtr Item::I2(initializer_list<ValueType::I2> values)
{
    ItemPtr item = std::make_shared<Item>(Format::I2);
    for (auto&& value : values)
    {
        item->append(value);
    }
    return item;
}

ItemPtr Item::I4(initializer_list<ValueType::I4> values)
{
    ItemPtr item = std::make_shared<Item>(Format::I4);
    for (auto&& value : values)
    {
        item->append(value);
    }
    return item;
}

ItemPtr Item::I8(initializer_list<ValueType::I8> values)
{
    ItemPtr item = std::make_shared<Item>(Format::I8);
    for (auto&& value : values)
    {
        item->append(value);
    }
    return item;
}

ItemPtr Item::U1(initializer_list<ValueType::U1> values)
{
    ItemPtr item = std::make_shared<Item>(Format::U1);
    for (auto&& value : values)
    {
        item->append(value);
    }
    return item;
}

ItemPtr Item::U2(initializer_list<ValueType::U2> values)
{
    ItemPtr item = std::make_shared<Item>(Format::U2);
    for (auto&& value : values)
    {
        item->append(value);
    }
    return item;
}

ItemPtr Item::U4(initializer_list<ValueType::U4> values)
{
    ItemPtr item = std::make_shared<Item>(Format::U4);
    for (auto&& value : values)
    {
        item->append(value);
    }
    return item;
}

ItemPtr Item::U8(initializer_list<ValueType::U8> values)
{
    ItemPtr item = std::make_shared<Item>(Format::U8);
    for (auto&& value : values)
    {
        item->append(value);
    }
    return item;
}

ItemPtr Item::F8(initializer_list<ValueType::F8> values)
{
    ItemPtr item = std::make_shared<Item>(Format::F8);
    for (auto&& value : values)
    {
        item->append(value);
    }
    return item;
}

ItemPtr Item::F4(initializer_list<ValueType::F4> values)
{
    ItemPtr item = std::make_shared<Item>(Format::F4);
    for (auto&& value : values)
    {
        item->append(value);
    }
    return item;
}

inline void Item::append(const Value &value)
{
    assert(checkValueFormat(value));
    values.emplace_back(value);
}

inline size_t Item::getCount()
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

ValueType::F4 Item::getF4(size_t index)
{
    return getValue<ValueType::F4>(index);
}

ValueType::F8 Item::getF8(size_t index)
{
    return getValue<ValueType::F8>(index);
}

ValueType::Item Item::getList(size_t index)
{
    return getValue<ValueType::Item>(index);
}

inline Format Item::getFormat() const
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

size_t Item::dataLength()
{
    return getCount() * formatSize();
}

bool Item::checkValueFormat(const Value& value)
{
    if (value.index() == 0 && format == Format::ASCII)
        return true;
    else if (value.index() == 1 && format == Format::Boolean)
        return true;
    else if (value.index() == 2 && format == Format::U1)
        return true;
    else if (value.index() == 3 && format == Format::U2)
        return true;
    else if (value.index() == 4 && format == Format::U4)
        return true;
    else if (value.index() == 5 && format == Format::U8)
        return true;
    else if (value.index() == 6 && (format == Format::I1 || format == Format::Binary))
        return true;
    else if (value.index() == 7 && format == Format::I2)
        return true;
    else if (value.index() == 8 && format == Format::I4)
        return true;
    else if (value.index() == 9 && format == Format::I8)
        return true;
    else if (value.index() == 10 && format == Format::F4)
        return true;
    else if (value.index() == 11 && format == Format::F8)
        return true;
    else if (value.index() == 12 && format == Format::List)
        return true;
    return false;
}

}

