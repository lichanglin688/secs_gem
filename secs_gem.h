﻿#ifndef SECS_GEM_H
#define SECS_GEM_H

#include <QDebug>
#include <variant>
#include <vector>
#include <memory>
#include <QByteArray>

class QDataStream;
namespace Secs{

using std::variant;
using std::vector;
using std::string;

class Item;
using ItemPtr = std::shared_ptr<Item>;

namespace ValueType
{
using ASCII = char;
using Binary = int8_t;
using Bool = bool;
using I1 = int8_t;
using I2 = int16_t;
using I4 = int32_t;
using I8 = int64_t;
using U1 = uint8_t;
using U2 = uint16_t;
using U4 = uint32_t;
using U8 = uint64_t;
using Item = ItemPtr;
};

class Item;
#define ValueTypeList ValueType::ASCII, ValueType::Bool, ValueType::U1, ValueType::U2, ValueType::U4, ValueType::U8, ValueType::I1, ValueType::I2, ValueType::I4, ValueType::I8, ValueType::Item
using Value = variant<ValueTypeList>;

//LCS和JIS8未实现，不可使用
enum class Format
{
    List = 0,		        // 00000000
    Binary = 0x20,		    // 00100000
    Boolean = 0x24,		    // 00100100
    ASCII = 0x40,		    // 01000000
    JIS8 = 0x44,			// 01000100
    LCS = 0x48,			    // 01001000
    I8 = 0x60,				// 01100000
    I1 = 0x64,				// 01100100
    I2 = 0x68,				// 01101000
    I4 = 0x70,				// 01110000
    F8 = 0x80,				// 10000000
    F4 = 0x90,				// 10010000
    U8 = 0xA0,				// 10100000
    U1 = 0xA4,				// 10100100
    U2 = 0xA8,				// 10101000
    U4 = 0xB0,				// 10110000
    NONE = 0xFF
};

class Item
{
public:
    Item(Format format = Format::NONE);
    ~Item();

    static ItemPtr List();
    static ItemPtr Bool();
    static ItemPtr Binary();
    static ItemPtr I1();
    static ItemPtr I2();
    static ItemPtr I4();
    static ItemPtr I8();

    static ItemPtr U1();
    static ItemPtr U2();
    static ItemPtr U4();
    static ItemPtr U8();

    inline void append(const Value &value);

    inline size_t getCount();

    template<typename Type>
    Type getValue(size_t index)
    {
        static_assert(std::_Is_any_of_v<Type, ValueTypeList>, "Type is not Secs type");
        return std::get<Type>(values[index]);
    }

    ValueType::ASCII getASCII(size_t index = 0);
    string getString();
    ValueType::Binary getBinary(size_t index = 0);
    ValueType::Bool getBool(size_t index = 0);
    ValueType::I1 getI1(size_t index = 0);
    ValueType::I2 getI2(size_t index = 0);
    ValueType::I4 getI4(size_t index = 0);
    ValueType::I8 getI8(size_t index = 0);
    ValueType::U1 getU1(size_t index = 0);
    ValueType::U2 getU2(size_t index = 0);
    ValueType::U4 getU4(size_t index = 0);
    ValueType::U8 getU8(size_t index = 0);
    ValueType::Item getList(size_t index = 0);

    inline Format getFormat() const;

    int formatSize();

//private:
    std::vector<Value> values;
    Format format;
};

class Message
{
public:
    QByteArray encode();

    void decode(QByteArray data);

    ItemPtr getItem(size_t index);
    void appendItem(ItemPtr item);

 //private:
    uint16_t deviceID = 0;
    uint8_t s = 0;
    uint8_t f = 0;
    uint8_t pType = 0;
    uint8_t sType = 0;
    uint32_t systemBytes = 0;
    bool replyExpected = false;

    std::vector<ItemPtr> items;

private:
    void inputMessageHeader(QDataStream &in);
    uint8_t getStreamByte();
    void inputItem(QDataStream &in, const ItemPtr &item);
    void inputItemInfo(QDataStream &in, const ItemPtr &item);

    void outMessageHeader(QDataStream &out);
    ItemPtr outItem(QDataStream &out);
    void outItemInfo(QDataStream &out, Format &format, int &count);
    Value outItemValue(QDataStream &out, const Format &format);
};



class Visitor
{
public:
    void operator()(ValueType::ASCII value)
    {
        qDebug() << value;
    }
    void operator()(ValueType::Bool value)
    {
        qDebug() << value;
    }
    void operator()(ValueType::I1 value)
    {
        qDebug() << value;
    }
    void operator()(ValueType::I2 value)
    {
        qDebug() << value;
    }
    void operator()(ValueType::I4 value)
    {
        qDebug() << value;
    }
    void operator()(ValueType::I8 value)
    {
        qDebug() << value;
    }
    void operator()(ValueType::U1 value)
    {
        qDebug() << value;
    }
    void operator()(ValueType::U2 value)
    {
        qDebug() << value;
    }
    void operator()(ValueType::U4 value)
    {
        qDebug() << value;
    }
    void operator()(ValueType::U8 value)
    {
        qDebug() << value;
    }
    void operator()(ValueType::Item value)
    {
        qDebug() << "Item count => " << value->getCount();
    }
//    void operator()(ValueType::Binary value)
//    {
//        qDebug() << value;
//    }
};
}



#endif // SECS_GEM_H