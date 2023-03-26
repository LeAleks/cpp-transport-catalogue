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

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    // Класс Node должен уметь хранить типы:
    // int, double, std::string, bool, Array, Dict, std::nullptr_t
    // 
    class Node {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

        Node() = default;
        Node(std::nullptr_t ptr);
        Node(Array array);
        Node(Dict dict);
        Node(bool value);
        Node(int  value);
        Node(double  value);
        Node(std::string line_s);


        const Value& GetValue() const {
            return value_;
        }


        // Следующие методы Node сообщают, хранится ли внутри значение некоторого типа:

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

        // Ниже перечислены методы, которые возвращают хранящееся внутри Node значение
        // заданного типа. Если внутри содержится значение другого типа, должно
        // выбрасываться исключение std::logic_error.

        int AsInt() const;
        bool AsBool() const;
        // Возвращает значение типа double, если внутри хранится double либо int.
        // В последнем случае возвращается приведённое в double значение.
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        // Перегрузка операторов сравнения
        bool operator==(const Node& other) const {
            return value_ == other.value_;
        }

        bool operator!=(const Node& other) const {
            return value_ != other.value_;
        }

    private:
        Value value_;
    };


    // Класс для хранения указателя на информацию загруженную из json
    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& other) {
            return root_ == other.root_;
        }

        bool operator!=(const Document& other) {
            return root_ != other.root_;
        }

    private:
        Node root_;
    };

    Document Load(std::istream& input);


    // Печать документа
    void Print(const Document& doc, std::ostream& output);

}  // namespace json