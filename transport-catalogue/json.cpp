#include "json.h"

using namespace std;

namespace json {

void PrintContext::PrintIndent() const {
    for (int i = 0; i < indent; ++i) {
        out.put(' ');
    }
}

// Возвращает новый контекст вывода с увеличенным смещением
PrintContext PrintContext::Indented() const {
    return {out, indent_step, indent_step + indent};
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(*this);
}
// Возвращает true, если в Node хранится int либо double.
bool Node::IsDouble() const {
    return std::holds_alternative<double>(*this) || IsInt();
} 

// Возвращает true, если в Node хранится double.
bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(*this);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(*this);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(*this);
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(*this);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(*this);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(*this);
}

int Node::AsInt() const {
    if(IsInt()) {
        return std::get<int>(*this);
    } else {
        throw std::logic_error("bad variant access");
    }
}

bool Node::AsBool() const {
    if(IsBool()) {
        return std::get<bool>(*this);
    } else {
        throw std::logic_error("bad variant access");
    }
}

// Возвращает значение типа double, если внутри хранится double либо int. В последнем случае возвращается приведённое в double значение.
double Node::AsDouble() const {
    if(IsInt()) {
            double value = static_cast<double>(get<int>(*this));
            return value;
    } else if(IsDouble()) {
        return std::get<double>(*this);
    } else {
        throw std::logic_error("bad variant access");
    }
}

const std::string& Node::AsString() const {
    if(IsString()) {
        return std::get<std::string>(*this);
    } else {
        throw std::logic_error("bad variant access");
    }
}

const Array& Node::AsArray() const {
    if(IsArray()) {
        return std::get<Array>(*this);
    } else {
        throw std::logic_error("bad variant access");
    }
}

const Dict& Node::AsMap() const {
    if(IsMap()) {
        return std::get<Dict>(*this);
    } else {
        throw std::logic_error("bad variant access");
    }
}

Value Node::GetValue() const {
    if(IsArray()) {
        return this->AsArray();
    } if(IsBool()) {
        return this->AsBool();
    } if(IsPureDouble()) {
        return this->AsDouble();
    } if(IsInt()) {
        return this->AsInt();
    } if(IsMap()) {
        return this->AsMap();
    } if(IsNull()) {
        return nullptr;
    } if(IsString()) {
        return this->AsString();
    } else {
        throw std::logic_error("bad variant access");
    }
}

bool operator==(const Node& lhs, const Node& rhs) {
    return lhs.GetValue() == rhs.GetValue();
}

bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Node LoadArray(std::istream& input) {
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
    return Node{move(result)};
}

Node LoadInt(istream& input) {
    int result = 0;
    while (isdigit(input.peek())) {
        result *= 10;
        result += input.get() - '0';
    }
    return Node{result};
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

Node LoadDict(std::istream& input) {
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
    return Node{move(result)};
}

Node LoadBool(bool value) {
    return Node{value};
}

Node LoadNull() {
    return Node();
}

Node LoadNode(istream &input)
{
    using namespace std::literals;
    char c;
    while (input >> c)
    {
        if (c == ' ')
        {
            //пробелы игнорируются
            continue;
        }
        else if (c == '\\')
        {
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
        }
        else if (c == '{')
        {
            return LoadDict(input);
        }
        else if (c == '[')
        {
            return LoadArray(input);
        }
        else if (c == 'n')
        {
            if (input >> c && c == 'u')
            {
                if (input >> c && c == 'l')
                {
                    if (input >> c && c == 'l')
                    {
                        return LoadNull();
                    }
                    else
                    {
                        throw ParsingError("Bad JSON");
                    }
                }
                else
                {
                    throw ParsingError("Bad JSON");
                }
            }
            else
            {
                throw ParsingError("Bad JSON - not u!");
            }
        }
        else if (c == '-' || std::isdigit(c))
        {
            input.putback(c);
            Number number = LoadNumber(input);
            if (holds_alternative<int>(number))
            {
                return Node{get<int>(number)};
            }
            else if (holds_alternative<double>(number))
            {
                return Node{get<double>(number)};
            }
            else
            {
                throw ParsingError("Number parsing error");
            }
        }
        else if (c == '"')
        {
            return Node{LoadString(input)};
        }
        else if (c == 't')
        {
            if (input >> c; c == 'r')
            {
                if (input >> c; c == 'u')
                {
                    if (input >> c; c == 'e')
                    {
                        return LoadBool(true);
                    }
                    else
                    {
                        throw ParsingError("Bad JSON");
                    }
                }
                else
                {
                    throw ParsingError("Bad JSON");
                }
            }
            else
            {
                throw ParsingError("Bad JSON");
            }
        }
        else if (c == 'f')
        {
            if (input >> c; c == 'a')
            {
                if (input >> c; c == 'l')
                {
                    if (input >> c; c == 's')
                    {
                        if (input >> c; c == 'e')
                        {
                            return LoadBool(false);
                        }
                        else
                        {
                            throw ParsingError("Bad JSON");
                        }
                    }
                    else
                    {
                        throw ParsingError("Bad JSON");
                    }
                }
                else
                {
                    throw ParsingError("Bad JSON");
                }
            }
            else
            {
                throw ParsingError("Bad JSON");
            }
        }
        else if (c == ']')
        {
            throw ParsingError("No open bracket for '[ ]' found"s);
        }
        else if (c == '}')
        {
            throw ParsingError("No open bracket for '{ }' found"s);
        }
        else
        {
            return Node();
        }
    }
    return Node{};
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), output);
}

void Print(const Document& doc, const PrintContext& ctx) {
    PrintNode(doc.GetRoot(), ctx);
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

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, const PrintContext& ctx) {
    ctx.out << "null"sv;
}

void PrintValue(Array array, const PrintContext& ctx) {
    ctx.out << "[" << '\n';
    bool is_first = true;

    auto indented = ctx.Indented();
    for(const auto& node : array) {
        if(!is_first) {
            ctx.out << "," << '\n';
        }
        indented.PrintIndent();
        PrintNode(node, indented);
        is_first = false;
    }
    ctx.out << '\n';
    ctx.PrintIndent();
    ctx.out << "]";
}

void PrintValue(Dict dict, const PrintContext& ctx) {
    ctx.out << "{" << '\n';

    bool is_first = true;

    auto indented = ctx.Indented();
    for(const auto& p : dict) {
        if(!is_first) {
            ctx.out << ',' << '\n';
        }
        indented.PrintIndent();
        ctx.out << "\"" << p.first << "\": ";
        PrintNode(p.second, indented);
        is_first = false;
    }
    ctx.out << '\n';
    ctx.PrintIndent();
    ctx.out << "}";
}

void PrintValue(bool value, const PrintContext& ctx) {
    if (value) {
        ctx.out << "true";
    }
    else {
        ctx.out << "false";
    }
}

void PrintValue(const std::string& s, const PrintContext& ctx) {
    ctx.out << "\"" << string(std::move(TransformPrintString(std::move(s)))) << "\"";
}
//Другие перегрузки функции PrintValue пишутся аналогично
void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit(
        [&ctx](const auto& value) { PrintValue(value, ctx); },
        node.GetValue());
}
void PrintNode(const Node& node, std::ostream& out) {
    std::visit(
        [&out](const auto& value) { PrintValue(value, PrintContext{out, 1, 0}); },
        node.GetValue());
}
bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}
bool operator!=(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() != rhs.GetRoot();
}
}  // namespace json