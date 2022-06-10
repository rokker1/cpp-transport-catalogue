#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <stdexcept>
#include <sstream>

namespace json {

struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for(int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Number = std::variant<int, double>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

Number LoadNumber(std::istream& input);
std::string LoadString(std::istream& input);
Node LoadNull();
Node LoadBool(bool value);

class Node {
public:
   /* Реализуйте Node, используя std::variant */
   using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    Node();
    Node(std::nullptr_t);
    Node(Array array);
    Node(Dict map);
    Node(bool value);
    Node(int value);
    Node(double value);
    Node(std::string value);

    const Value& GetValue() const;

    bool IsInt() const;
    //Возвращает true, если в Node хранится int либо double.
    bool IsDouble() const; 
    //Возвращает true, если в Node хранится double.
    bool IsPureDouble() const; 
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    int AsInt() const;
    bool AsBool() const;
    // Возвращает значение типа double, если внутри хранится double либо int. 
    // В последнем случае возвращается приведённое в double значение.
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

private:
    Value value_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

bool operator==(const Node& lhs, const Node& rhs);
bool operator!=(const Node& lhs, const Node& rhs);
bool operator==(const Document& lhs, const Document& rhs);
bool operator!=(const Document& lhs, const Document& rhs);

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

void PrintNode(const Node& node, std::ostream& out);

// int & double
template <typename Value>
void PrintValue(const Value& value, std::ostream& out) {
    out << value;
}

void PrintValue(std::nullptr_t, std::ostream& out);
void PrintValue(bool value, std::ostream &out);
std::string TransformPrintString(std::string text);
void PrintValue(const std::string& s, std::ostream &out);
void PrintValue(const Array& array, std::ostream &out);
void PrintValue(const Dict& array, std::ostream &out);

json::Document LoadJSON(const std::string& s);
std::string Print(const Node& node);
}