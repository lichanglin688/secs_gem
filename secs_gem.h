#ifndef SECS_GEM_H
#define SECS_GEM_H

#include <variant>
#include <vector>
#include <memory>
#include <QByteArray>
#include <initializer_list>
#include <QDebug>

class QDataStream;
namespace Secs{

using std::variant;
using std::vector;
using std::string;
using std::initializer_list;

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
using F4 = float;
using F8 = double;
using Item = ItemPtr;
};

class Item;
#define ValueTypeList ValueType::ASCII, ValueType::Bool, ValueType::U1, ValueType::U2, ValueType::U4, ValueType::U8, ValueType::I1, ValueType::I2, ValueType::I4, ValueType::I8, ValueType::F4, ValueType::F8, ValueType::Item
using Value = variant<ValueTypeList>;

//LCS和JIS8未实现，不可使用
enum class Format
{
    List = 0,		        // 00000000
    Binary = 0x20,		    // 00100000
    Bool = 0x24,		    // 00100100
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

#define SECS_CREATE_ITEM(type) \
        static ItemPtr type(initializer_list<ValueType::type> values = {}) \
        {\
            ItemPtr item = std::make_shared<Item>(Format::type); \
            for (auto&& value : values)\
            {\
                item->append(value);\
            }\
            return item;\
        }

#define SECS_GET_VALUE(type) \
    ValueType::type get##type(size_t index = 0)\
    {\
        return getValue<ValueType::type>(index);\
    }

class Item
{
public:
    Item(Format format = Format::NONE);
    ~Item();

    SECS_CREATE_ITEM(ASCII)
    SECS_CREATE_ITEM(Bool)
    SECS_CREATE_ITEM(Binary)
    SECS_CREATE_ITEM(I1)
    SECS_CREATE_ITEM(I2)
    SECS_CREATE_ITEM(I4)
    SECS_CREATE_ITEM(I8)
    SECS_CREATE_ITEM(U1)
    SECS_CREATE_ITEM(U2)
    SECS_CREATE_ITEM(U4)
    SECS_CREATE_ITEM(U8)
    SECS_CREATE_ITEM(F4)
    SECS_CREATE_ITEM(F8)
    static ItemPtr List(initializer_list<ValueType::Item> values = {});
    static ItemPtr String(const string &str);

    SECS_GET_VALUE(ASCII)
    SECS_GET_VALUE(Binary)
    SECS_GET_VALUE(Bool)
    SECS_GET_VALUE(I1)
    SECS_GET_VALUE(I2)
    SECS_GET_VALUE(I4)
    SECS_GET_VALUE(I8)
    SECS_GET_VALUE(U1)
    SECS_GET_VALUE(U2)
    SECS_GET_VALUE(U4)
    SECS_GET_VALUE(U8)
    SECS_GET_VALUE(F4)
    SECS_GET_VALUE(F8)
    SECS_GET_VALUE(Item)
    string getString();

    void append(const Value &value);

    size_t getCount();

    template<typename Type>
    Type getValue(size_t index)
    {
        static_assert(std::_Is_any_of_v<Type, ValueTypeList>, "Type is not Secs type");
        return std::get<Type>(values[index]);
    }

    Format getFormat() const;

    int formatSize();

    size_t dataLength();

private:
    bool checkValueFormat(const Value &value);

private:
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

    uint16_t deviceID = 0;
    uint8_t s = 0;
    uint8_t f = 0;
    uint8_t pType = 0;
    uint8_t sType = 0;
    uint32_t systemBytes = 0;
    bool replyExpected = false;

private:
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
    void operator()(ValueType::F4 value)
    {
        qDebug() << value;
    }
    void operator()(ValueType::F8 value)
    {
        qDebug() << value;
    }
};

class CheckVisitor
{
public:
    CheckVisitor()
    {

    }
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
    void operator()(ValueType::F4 value)
    {
        qDebug() << value;
    }
    void operator()(ValueType::F8 value)
    {
        qDebug() << value;
    }

private:
    Format format;
};
}



#endif // SECS_GEM_H
