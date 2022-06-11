#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
using Number = std::variant<int, double>;

// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const;

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const;
};


// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node : public Value {
public:
    using variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>::variant;

    bool IsInt() const;
    // Возвращает true, если в Node хранится int либо double.
    bool IsDouble() const; 
    // Возвращает true, если в Node хранится double.
    bool IsPureDouble() const; 
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    int AsInt() const;
    bool AsBool() const;
    // Возвращает значение типа double, если внутри хранится double либо int. В последнем случае возвращается приведённое в double значение.
    double AsDouble() const; 
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

    Value GetValue() const;
};

bool operator==(const Node& lhs, const Node& rhs);
bool operator!=(const Node& lhs, const Node& rhs);

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

bool operator==(const Document& lhs, const Document& rhs);
bool operator!=(const Document& lhs, const Document& rhs);

Document Load(std::istream& input);
Node LoadNode(std::istream &input);
Node LoadArray(std::istream& input);
Number LoadNumber(std::istream& input);
// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
std::string LoadString(std::istream& input);
Node LoadDict(std::istream& input);
Node LoadBool(bool value);
Node LoadNull();

void Print(const Document& doc, std::ostream& output);
void Print(const Document& doc, const PrintContext& ctx);

std::string TransformPrintString(std::string text);

// Шаблон, подходящий для вывода double и int
template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}
void PrintValue(std::nullptr_t, const PrintContext& ctx);
void PrintValue(Array array, const PrintContext& ctx);
void PrintValue(Dict dicr, const PrintContext& ctx);
void PrintValue(bool value, const PrintContext& ctx);
void PrintValue(const std::string& value, const PrintContext& ctx);
void PrintNode(const Node& node, const PrintContext& ctx);
void PrintNode(const Node& node, std::ostream& out);
}  // namespace json