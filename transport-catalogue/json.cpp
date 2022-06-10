#include "json.h"

using namespace std;

namespace json {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;
    // была ли считана закрывающая скобка
    bool array_is_closed = false;
    for (char c; input >> c;) {
        if(c == ']') {
            array_is_closed = true;
            break;
        }
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if(!array_is_closed) {
        throw ParsingError("No closing ']'."s);
    }
    return Node(move(result));
}

Node LoadInt(istream& input) {
    int result = 0;
    while (isdigit(input.peek())) {
        result *= 10;
        result += input.get() - '0';
    }
    return Node(result);
}

Number LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_sum;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_sum, &input] {
        parsed_sum += static_cast<char>(input.get());
        if(!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_sum из input
    auto read_digits = [&input, read_char] {
        if(!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected!"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if(input.peek() == '-') {
        read_char();
    }

    // парсим целую часть числа
    if(input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if(int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if(ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if(is_int) {
            // Сначала попробуем преобразовать строку в int
            try {
                return std::stoi(parsed_sum);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_sum);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_sum + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
std::string LoadString(std::istream& input) {
    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while(true) {
        if(it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if(ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if(it == end) {
                // Поток завершился сразу после символа обратной косой черты?
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *it;
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char)
            {
            case 'n':
                s.push_back('\n');
                break;
            case 't':
                s.push_back('\t');
                break;
            case 'r':
                s.push_back('\r');
                break;
            case '"':
                s.push_back('"');
                break;
            case '\\':
                s.push_back('\\');
                break;
            default:
                // Встретили неизвестную escape-последовательность
                throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }


        } else if(ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }
    return s;
}

Node LoadDict(istream& input) {
    Dict result;
    // была ли считана закрывающая скобка
    bool dict_is_closed = false;
    for (char c; input >> c;) {
        if(c == '}') {
            dict_is_closed = true;
            break;
        }
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input);
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    if(!dict_is_closed) {
        throw ParsingError("No closing '}'."s);
    }
    return Node(move(result));
}

Node LoadBool(bool value) {
    return Node(value);
}

Node LoadNull() {
    return Node();
}

Node LoadNode(istream& input) {
    using namespace std::literals;
    char c;
    while(input >> c) {
        if(c == ' ') {
            //пробелы игнорируются
            continue;
        } else if(c == '\\') {
            // встретили начало escape последовательности
            switch (input.peek())
            {
            case 'n':
                input >> c;
                break;
            case 't':
                input >> c;
                break;
            case 'r':
                input >> c;
                break;
            default:
                // Встретили неизвестную escape-последовательность
                throw ParsingError("Unrecognized escape sequence \\"s);
            }
        } else if (c == '{') {
            return LoadDict(input);
        } else if (c == '[') {
            return LoadArray(input);
        } else if( c == 'n') {
            if(input >> c && c == 'u') {
                if(input >> c && c == 'l') {
                    if(input >> c && c == 'l') {
                        return LoadNull();
                    } else { throw ParsingError("Bad JSON"); }
                } else { throw ParsingError("Bad JSON"); }
            } else { throw ParsingError("Bad JSON - not u!"); }
        } else if( c == '-' || std::isdigit(c)) {
            input.putback(c);
            Number number = LoadNumber(input);
            if(holds_alternative<int>(number)) {
                return Node(get<int>(number));
            } else if (holds_alternative<double>(number)) {
                return Node(get<double>(number));
            } else {
                throw ParsingError("Number parsing error");
            }
        } else if(c == '"') {
            return LoadString(input);
        } else if( c == 't') {
            if(input >> c; c == 'r') {
                if(input >> c; c == 'u') {
                    if(input >> c; c == 'e') {
                        return LoadBool(true);
                    } else { throw ParsingError("Bad JSON"); }
                } else { throw ParsingError("Bad JSON"); }
            } else { throw ParsingError("Bad JSON"); } 
        } else if (c == 'f') {
            if(input >> c; c == 'a') {
                if(input >> c; c == 'l') {
                    if(input >> c; c == 's') {
                        if(input >> c; c == 'e') {
                            return LoadBool(false);
                        } else { throw ParsingError("Bad JSON"); } 
                    } else { throw ParsingError("Bad JSON"); } 
                } else { throw ParsingError("Bad JSON"); } 
            } else { throw ParsingError("Bad JSON"); } 
        } else if(c == ']') {
            throw ParsingError("No open bracket for '[ ]' found"s);
        } else if(c == '}') {
            throw ParsingError("No open bracket for '{ }' found"s);
        }

        
        
        else { 
            return Node(); 
        } 
    }
    return Node(); 
}

Node::Node(nullptr_t) 
    : value_(nullptr) {
}

Node::Node() 
    : value_(nullptr) {
}

Node::Node(Array array)
    : value_(move(array)) {
}

Node::Node(Dict map)
    : value_(move(map)) {
}

Node::Node(double value)
    : value_(value) {
}

Node::Node(bool value) 
    : value_(value) {
}

Node::Node(int value)
    : value_(value) {
}

Node::Node(string value)
    : value_(move(value)) {
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

// Чтение документа из входного потока
Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), output);

}
const Node::Value& Node::GetValue() const {
    return value_;
}
void PrintNode(const Node& node, std::ostream& out) {
    std::visit(
        [&out](const auto& value){
            PrintValue(value, out);
        }, node.GetValue());
} 
void PrintValue(std::nullptr_t, std::ostream& out) {
    out << "null"s;
}
void PrintValue(bool value, std::ostream &out)
{
    if (value)
    {
        out << "true";
    }
    else
    {
        out << "false";
    }
}

std::string TransformPrintString(std::string text) {
    
    size_t pos = text.find_first_of("\"\\\r\n");
    while(pos != text.npos) {
        
        if(text[pos] == '"') {
            text.insert(pos, "\\"); // вставляет один слэш
            pos += 2;
        } else if (text[pos] == '\\') {
            text.insert(pos, "\\"); // вставляет один слэш
            pos += 2;
        } else if (text[pos] == '\r') {
            text.erase(pos, 1);
            text.insert(pos, "\\r");
            pos += 2;
        } else if (text[pos] == '\n') {
            text.erase(pos, 1);
            text.insert(pos, "\\n");
            pos += 2;
        } else {
            continue;
        }
        pos = text.find_first_of("\"\\\r\n", pos);
    }
    return text;
}

void PrintValue(const std::string& s, std::ostream &out) {
    out << "\"" << string(std::move(TransformPrintString(std::move(s)))) << "\"";
}
void PrintValue(const Array& array, std::ostream &out) {
    out << "[";
    bool is_first = true;
    for(const auto& node : array) {
        if(!is_first) {
            out << ", ";
        }
        PrintNode(node, out);
        is_first = false;
    }
    out << "]";
}
void PrintValue(const Dict& dict, std::ostream &out) {
    out << "{";
    bool is_first = true;
    for(const auto& p : dict) {
        if(!is_first) {
            out << ", ";
        }
        out << "\"" << p.first << "\": ";
        PrintNode(p.second, out);
        is_first = false;
    }
    out << "}";
}

bool Node::IsInt() const {
    if(holds_alternative<int>(value_)) { return true; } 
    else { return false; }
}
//Возвращает true, если в Node хранится int либо double.
bool Node::IsDouble() const {
    if(holds_alternative<double>(value_) || holds_alternative<int>(value_)) { return true; } 
    else { return false; }
} 
//Возвращает true, если в Node хранится double.
bool Node::IsPureDouble() const {
    if(holds_alternative<double>(value_)) { return true; } 
    else { return false; }
} 
bool Node::IsBool() const {
    if(holds_alternative<bool>(value_)) { return true; } 
    else { return false; }
}
bool Node::IsString() const {
    if(holds_alternative<std::string>(value_)) { return true; } 
    else { return false; }
}
bool Node::IsNull() const {
    if(holds_alternative<nullptr_t>(value_)) { return true; } 
    else { return false; }
}
bool Node::IsArray() const {
    if(holds_alternative<Array>(value_)) { return true; } 
    else { return false; }
}
bool Node::IsMap() const {
    if(holds_alternative<Dict>(value_)) { return true; } 
    else { return false; }
}

int Node::AsInt() const {
    try {
        return get<int>(value_);
    } catch (const bad_variant_access&) {
        throw std::logic_error("value is not 'int'");
    }
}
bool Node::AsBool() const {
    try {
        return get<bool>(value_);
    } catch (const bad_variant_access&) {
        throw std::logic_error("value is not 'bool'");
    }
}
// Возвращает значение типа double, если внутри хранится double либо int. 
// В последнем случае возвращается приведённое в double значение.
double Node::AsDouble() const {
    try {
        if(IsInt()) {
            double value = static_cast<double>(get<int>(value_));
            return value;
        }
        return get<double>(value_);
    } catch (const bad_variant_access&) {
        throw std::logic_error("value is not 'double' castable");
    }
}
const std::string& Node::AsString() const {
    try {
        return get<std::string>(value_);
    } catch (const bad_variant_access&) {
        throw std::logic_error("value is not 'std::string'");
    }
}
const Array& Node::AsArray() const {
    try {
        return get<Array>(value_);
    } catch (const bad_variant_access&) {
        throw std::logic_error("value is not 'Array (a.k.a std::vector<Node>)'");
    }
}
const Dict& Node::AsMap() const {
    try {
        return get<Dict>(value_);
    } catch (const bad_variant_access&) {
        throw std::logic_error("value is not 'Dict (a.k.a std::map)'");
    }
}

bool operator==(const Node& lhs, const Node& rhs) {
    return lhs.GetValue() == rhs.GetValue();
}
bool operator!=(const Node& lhs, const Node& rhs) {
    return lhs.GetValue() != rhs.GetValue();
}

bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}
bool operator!=(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() != rhs.GetRoot();
}

json::Document LoadJSON(const std::string& s) {
    std::istringstream strm(s);
    return json::Load(strm);
}

std::string Print(const Node& node) {
    std::ostringstream out;
    Print(Document{node}, out);
    //std::cout << "Debug: " << out.str() << std::endl;
    return out.str();
}


}  // namespace json