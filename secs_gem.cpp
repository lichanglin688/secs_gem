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
        else if (format == Format::ASCII)
        {
            in << (qint8)item->getValue<ValueType::ASCII>(i);
        }
        else if(format == Format::Binary)
        {
            in << item->getValue<ValueType::Binary>(i);
        }
        else if(format == Format::Bool)
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
    if(format == Format::List && dataLength == 0)
    {
        in << (ValueType::U1)((int)format | 1);
        in << (ValueType::U1)0;
        return;
    }

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
    else if(format == Format::Bool)
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

inline void Item::append(const Value &value)
{
    assert(checkValueFormat(value));
    values.emplace_back(value);
}

inline size_t Item::getCount()
{
    return values.size();
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
    if (std::holds_alternative<ValueType::ASCII>(value) && format == Format::ASCII)
        return true;
    else if (std::holds_alternative<ValueType::Bool>(value) && format == Format::Bool)
        return true;
    else if (std::holds_alternative<ValueType::U1>(value) && format == Format::U1)
        return true;
    else if (std::holds_alternative<ValueType::U2>(value) && format == Format::U2)
        return true;
    else if (std::holds_alternative<ValueType::U4>(value) && format == Format::U4)
        return true;
    else if (std::holds_alternative<ValueType::U8>(value) && format == Format::U8)
        return true;
    else if (std::holds_alternative<ValueType::I1>(value) && (format == Format::I1 || format == Format::Binary))
        return true;
    else if (std::holds_alternative<ValueType::I2>(value) && format == Format::I2)
        return true;
    else if (std::holds_alternative<ValueType::I4>(value) && format == Format::I4)
        return true;
    else if (std::holds_alternative<ValueType::I8>(value) && format == Format::I8)
        return true;
    else if (std::holds_alternative<ValueType::F4>(value)  && format == Format::F4)
        return true;
    else if (std::holds_alternative<ValueType::F8>(value)  && format == Format::F8)
        return true;
    else if (std::holds_alternative<ValueType::Item>(value) && format == Format::List)
        return true;
    return false;
}

ItemPtr Item::String(const string& str)
{
    ItemPtr item = ASCII();
    for (auto&& s : str)
    {
        item->append(s);
    }
    return item;
}

}

