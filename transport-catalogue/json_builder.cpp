#include "json_builder.h"

#include <utility>

namespace Json {

    Builder::Builder() {
        Node *root_ptr = &root_;
        nodes_.emplace_back(root_ptr);
    }

    Builder::DictKeyContext Builder::Key(std::string key) {
        auto *top_node = nodes_.back();

        if (top_node->IsDict() && !key_) key_ = std::move(key);
        else {
            throw std::logic_error("wrong key");
        }

        return *this;
    }

    Builder &Builder::Value(Node::Value value) {
        auto *top_node = nodes_.back();

        if (top_node->IsDict()) {
            if (!key_) {
                throw std::logic_error("key error");
            }
            auto &dict = std::get<Dict>(top_node->GetValue());
            auto [pos, _] = dict.emplace(std::move(key_.value()), Node{});
            key_ = std::nullopt;
            top_node = &pos->second;
            top_node->GetValue() = std::move(value);
        } else if (top_node->IsArray()) {
            auto &array = std::get<Array>(top_node->GetValue());
            array.emplace_back(GetNode(value));
            top_node = &array.back();
        } else if (root_.IsNull()) {
            root_.GetValue() = std::move(value);
        } else {
            throw std::logic_error("unknown container");
        }

        return *this;
    }

    Builder::DictItemContext Builder::StartDict() {
        auto *top_node = nodes_.back();

        if (top_node->IsDict()) {
            if (!key_) {
                throw std::logic_error("key not found");
            }
            auto &dict = std::get<Dict>(top_node->GetValue());
            auto [pos, _] = dict.emplace(std::move(key_.value()), Dict());
            key_ = std::nullopt;
            nodes_.emplace_back(&pos->second);
        } else if (top_node->IsArray()) {
            auto &array = std::get<Array>(top_node->GetValue());
            array.emplace_back(Dict());
            nodes_.emplace_back(&array.back());
        } else if (top_node->IsNull()) {
            top_node->GetValue() = Dict();
        } else {
            throw std::logic_error("wrong note");
        }

        return *this;
    }

    Builder &Builder::EndDict() {
        auto *top_node = nodes_.back();

        if (!top_node->IsDict()) {
            throw std::logic_error("wrong note");
        }
        nodes_.pop_back();

        return *this;
    }

    Builder::ArrayItemContext Builder::StartArray() {
        auto *top_node = nodes_.back();

        if (top_node->IsDict()) {
            if (!key_) {
                throw std::logic_error("key not found");
            }
            auto &dict = std::get<Dict>(top_node->GetValue());
            auto [pos, _] = dict.emplace(std::move(key_.value()), Array());
            key_ = std::nullopt;
            nodes_.emplace_back(&pos->second);
        } else if (top_node->IsArray()) {
            auto &array = std::get<Array>(top_node->GetValue());
            array.emplace_back(Array());
            nodes_.emplace_back(&array.back());
        } else if (top_node->IsNull()) {
            top_node->GetValue() = Array();
        } else {
            throw std::logic_error("Wrong prev node");
        }

        return *this;
    }

    Builder &Builder::EndArray() {
        auto *top_node = nodes_.back();

        if (!top_node->IsArray()) {
            throw std::logic_error("array error");
        }
        nodes_.pop_back();

        return *this;
    }

    Node Builder::Build() {
        if (root_.IsNull() || nodes_.size() > 1) {
            throw std::logic_error("build error");
        }
        return root_;
    }

    Node Builder::GetNode(Node::Value value) {
        if (std::holds_alternative<bool>(value)) {
            return {std::get<bool>(value)};
        }
        if (std::holds_alternative<Dict>(value)) {
            return {std::get<Dict>(value)};
        }
        if (std::holds_alternative<Array>(value)) {
            return {std::get<Array>(value)};
        }
        if (std::holds_alternative<int>(value)) {
            return {std::get<int>(value)};
        }
        if (std::holds_alternative<double>(value)) {
            return {std::get<double>(value)};
        }
        if (std::holds_alternative<std::string>(value)) {
            return {std::get<std::string>(value)};
        }
        if (std::holds_alternative<std::nullptr_t>(value)) {
            return {std::get<std::nullptr_t>(value)};
        }
        return {};
    }

    Builder::DictItemContext::DictItemContext(Builder &builder)
            : builder_(builder) {}

    Builder::DictKeyContext Builder::DictItemContext::Key(std::string key) {
        return builder_.Key(std::move(key));
    }

    Builder &Builder::DictItemContext::EndDict() {
        return builder_.EndDict();
    }

    Builder::ArrayItemContext::ArrayItemContext(Builder &builder)
            : builder_(builder) {}

    Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
        return ArrayItemContext(builder_.Value(std::move(value)));
    }

    Builder::DictItemContext Builder::ArrayItemContext::StartDict() {
        return builder_.StartDict();
    }

    Builder::ArrayItemContext Builder::ArrayItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder &Builder::ArrayItemContext::EndArray() {
        return builder_.EndArray();
    }

    Builder::DictKeyContext::DictKeyContext(Builder &builder)
            : builder_(builder) {}

    Builder::DictItemContext Builder::DictKeyContext::Value(Node::Value value) {
        return DictItemContext(builder_.Value(std::move(value)));
    }

    Builder::ArrayItemContext Builder::DictKeyContext::StartArray() {
        return builder_.StartArray();
    }

    Builder::DictItemContext Builder::DictKeyContext::StartDict() {
        return builder_.StartDict();
    }

}