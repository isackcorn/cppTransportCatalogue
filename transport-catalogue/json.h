#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace Json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>{
    public:
        using variant::variant;
        using Value = variant;

        Node() = default;

        Node(std::nullptr_t);

        Node(std::string value);

        Node(int value);

        Node(double value);

        Node(bool value);

        Node(Array array);

        Node(Dict map);

        bool IsNull() const;

        bool IsBool() const;

        bool IsInt() const;

        bool IsDouble() const;

        bool IsNativeDouble() const;

        bool IsDict() const;

        bool IsString() const;

        bool IsArray() const;

        bool IsMap() const;

        int AsInt() const;

        bool AsBool() const;

        double AsDouble() const;

        const std::string &AsString() const;

        const Array &AsArray() const;

        const Dict &AsMap() const;

        [[nodiscard]] const Value & GetValue() const;

        Value& GetValue();

        const Dict& AsDict() const;

        bool operator==(const Node &rhs) const;

        bool operator!=(const Node &rhs) const;

    };

    class Document {
    public:
        explicit Document(Node root);

        [[nodiscard]] const Node &GetRoot() const;

        bool operator==(const Document &rhs) const;

        bool operator!=(const Document &rhs) const;

    private:
        Node root_;
    };

    Document Load(std::istream &input);

    void Print(const Document &document, std::ostream &output);

    struct ContainerPrinter {
        std::ostream &out;

        void operator()(std::nullptr_t);

        void operator()(const std::string &value);

        void operator()(int value);

        void operator()(double value);

        void operator()(bool value);

        void operator()(Array array);

        void operator()(const Dict& dict);
    };

}