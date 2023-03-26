#include "json.h"
#include <sstream>

using namespace std;

namespace json {

namespace {

// ---------- Функции для чтения json и создания Node----------

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    return Node(move(result));
}

// Новый вариант LoadInt, обрабатывающий и int и double
// Новый алгоритм считывания чисел
Node LoadNumber(std::istream& input) {
    string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    }
    else {
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
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(stoi(parsed_num));
            }
            catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(stod(parsed_num));
    }
    catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Новый вариант LoadString, возвращающий Node
Node LoadString(istream& input) {
    auto it = istreambuf_iterator<char>(input);
    auto end = istreambuf_iterator<char>();
    string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        }
        else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
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
        }
        else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        }
        else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }
    return Node(move(s));
}

Node LoadDict(istream& input) {
    Dict result;

    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({ move(key), LoadNode(input) });
    }

    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    // Проверка, что не начинается с неправильных скобок
    string first_letters = "]}"s;
    if (first_letters.find(c) != string::npos) {
        throw ParsingError("Failed to read number from stream"s);
    }

    first_letters = "ntf"s;

    if (c == '[') {
        // Проверка, что после что-то есть
        if (input >> c) {
            input.putback(c);
            return LoadArray(input);
        }
        else {
            throw ParsingError("Array parsing error");
        }
    }
    else if (c == '{') {
        // Проверка, что после что-то есть
        if (input >> c) {
            input.putback(c);
            return LoadDict(input);
        }
        else {
            throw ParsingError("Dict parsing error");
        }
    }
    else if (c == '"') {
        // Если знак разделения или буква, то string
        return LoadString(input);
    }
    else if (first_letters.find(c) != string::npos) {
        stringstream line;
        while (input) {
            if (isalpha(c)) {
                line << c;
            }
            input >> c;
            if (!isalpha(c)) {
                input.putback(c);
                break;
            }
        }

        if (line.str() == "null"s) {
            return Node(nullptr);
        }
        else if (line.str() == "true"s) {
            return Node(true);
        }
        else if (line.str() == "false"s) {
            return Node(false);
        }
        else {
            // Порядок букв не совпадает с null, false, true
            throw ParsingError("Bool parsing error");
        }
    }
    else {
        input.putback(c);
        // Функция LoadInt заменена на LoadNumber
        return LoadNumber(input);
    }
}

}  // namespace


// ----------- Новые алгоритмы парсинга строк и чисел ---------

using Number = std::variant<int, double>;
// Новый алгоритм считывания чисел
Number LoadNumberNew(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    }
    else {
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
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            }
            catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    }
    catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Новый алгоритм считывания строк
// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
std::string LoadStringNew(std::istream& input) {
    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        }
        else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
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
        }
        else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        }
        else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}

// ---------- Конструкторы новой версии Node ----------

Node::Node(nullptr_t ptr)
    : value_(ptr) {

}

Node::Node(Array array)
    : value_(move(array)) {
}

Node::Node(Dict dict)
    : value_(move(dict)) {

}

Node::Node(bool value)
    : value_(value) {

}

Node::Node(int  value)
    : value_(value) {

}

Node::Node(double  value)
    : value_(value) {

}

Node::Node(string line_s)
    : value_(move(line_s)) {

}

// ---------- Методы новой версии Node  ----------

// Следующие методы Node сообщают, хранится ли внутри значение некоторого типа:

bool Node::IsInt() const {
    if (holds_alternative<int>(value_)) {
        return true;
    }
    else {
        return false;
    }
}

// Возвращает true, если в Node хранится int либо double.
bool Node::IsDouble() const {
    if (holds_alternative<double>(value_) || holds_alternative<int>(value_)) {
        return true;
    }
    else {
        return false;
    }
}

// Возвращает true, если в Node хранится double.
bool Node::IsPureDouble() const {
    if (holds_alternative<double>(value_)) {
        return true;
    }
    else {
        return false;
    }
}

bool Node::IsBool() const {
    if (holds_alternative<bool>(value_)) {
        return true;
    }
    else {
        return false;
    }
}

bool Node::IsString() const {
    if (holds_alternative<string>(value_)) {
        return true;
    }
    else {
        return false;
    }
}

bool Node::IsNull() const {
    if (holds_alternative<nullptr_t>(value_)) {
        return true;
    }
    else {
        return false;
    }
}

bool Node::IsArray() const {
    if (holds_alternative<Array>(value_)) {
        return true;
    }
    else {
        return false;
    }
}

bool Node::IsMap() const {
    if (holds_alternative<Dict>(value_)) {
        return true;
    }
    else {
        return false;
    }
}

// Ниже перечислены методы, которые возвращают хранящееся внутри Node значение
// заданного типа. Если внутри содержится значение другого типа, должно
// выбрасываться исключение std::logic_error.

int Node::AsInt() const {
    // std::get_if вернёт указатель на значение нужного типа 
    // либо nullptr, если variant содержит значение другого типа.
    if (const auto* value_ptr = get_if<int>(&value_)) {
        return *value_ptr;
    }
    else {
        throw logic_error("AsInt: isn't 'int' type"s);
    }
}

bool Node::AsBool() const {
    if (const auto* value_ptr = get_if<bool>(&value_)) {
        return *value_ptr;
    }
    else {
        throw logic_error("AsBool: isn't 'bool' type"s);
    }
}

// Возвращает значение типа double, если внутри хранится double либо int.
// В последнем случае возвращается приведённое в double значение.
double Node::AsDouble() const {
    if (const auto* value_ptr = get_if<double>(&value_)) {
        return *value_ptr;
    }
    else {
        if (const auto* value_ptr = get_if<int>(&value_)) {
            return double(*value_ptr);
        }
        else {
            throw logic_error("AsDouble: isn't 'double' or 'int' type"s);
        }
    }
}

const string& Node::AsString() const {
    if (const auto* value_ptr = get_if<string>(&value_)) {
        return *value_ptr;
    }
    else {
        throw logic_error("AsString: isn't 'string' type"s);
    }
}

const Array& Node::AsArray() const {
    if (const auto* value_ptr = get_if<Array>(&value_)) {
        return *value_ptr;
    }
    else {
        throw logic_error("AsArray: isn't 'Array' type"s);
    }
}

const Dict& Node::AsMap() const {
    if (const auto* value_ptr = get_if<Dict>(&value_)) {
        return *value_ptr;
    }
    else {
        throw logic_error("AsMap: isn't 'Dict' type"s);
    }
}


// ---------- Методы класса Document ----------

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{ LoadNode(input) };
}


// --------- Функции вывода информации ----------

// Чтобы вывести значение Node в поток вывода, воспользуйтесь одним
// из способов, описанных в теории. Например, std::visit:

void PrintNode(const Node& node, std::ostream& out);

void PrintValue(const int value, std::ostream& out) {
    out << value;
}

void PrintValue(const double value, std::ostream& out) {
    out << value;
}

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, std::ostream& out) {
    out << "null"sv;
}

// Другие перегрузки функции PrintValue пишутся аналогично
// int, double, std::string, bool, Array, Dict, std::nullptr_t

// Перегрузка функции для вывода string
void PrintValue(const std::string& line, std::ostream& out) {
    out << "\""sv;
    for (char i : line) {
        switch (i)
        {
        case '\"':
            out << "\\\""sv;
            break;
        case '\n':
            out << "\\n"sv;
            break;
        case '\r':
            out << "\\r"sv;
            break;
        case '\\':
            out << "\\\\"sv;
            break;
        default:
            out << i;
            break;
        }
    }
    out << "\""sv;
}

// Перегрузка функции для вывода bool в виде true-false
void PrintValue(const bool value, std::ostream& out) {
    out << boolalpha << value;
}

// Перегрузка функции для вывода Array
void PrintValue(const Array& value, std::ostream& out) {
    out << "[\n"sv;

    bool is_first = true;

    for (auto& node : value) {
        if (is_first) {
            PrintNode(node, out);
            is_first = false;
        }
        else {
            out << ",\n"sv;
            PrintNode(node, out);
        }
    }

    out << "\n]"sv;
}

// Перегрузка функции для вывода Dict
void PrintValue(const Dict& value, std::ostream& out) {
    out << "{\n"sv;

    bool is_first = true;
    for (const auto& [key, node] : value) {
        if (is_first) {
            out << "\"" << key << "\": ";
            PrintNode(node, out);
            is_first = false;
        }
        else {
            out << ",\n\"" << key << "\": ";
            PrintNode(node, out);
        }
    }

    out << "\n}"sv;
}

void PrintNode(const Node& node, std::ostream& out) {
    std::visit(
        [&out](const auto& value) { PrintValue(value, out); },
        node.GetValue());
}


// Печать документа
void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), output);
}

}  // namespace json