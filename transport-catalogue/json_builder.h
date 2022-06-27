#pragma once
#include "json.h"
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <stdexcept>
#include <utility>

namespace json {

using json::Node;
/*
все классы не используются отдельно от билдера, можно было бы спрятать их внутрь класса билдера, 
организовать еще один класс, который бы содержал все возможные методы и был бы классом родителем для всех остальных методов
// */
// class ValueItemContext; // 1
// class KeyValueItemContext; // 2
// class DictItemContext; // 3
// class ArrayItemContext; // 4
// class ArrayValueItemContext; // 5


// class ValueItemContext2; // 1
// class KeyValueItemContext2; // 2
// class DictItemContext2; // 3
// class ArrayItemContext2; // 4
// class ArrayValueItemContext2; // 5
// class Builder;

//class ValueItemContext;



class Builder {
// friend class CommonContext;
public:
// declaration
// class ValueItemContext; // 1
// class KeyValueItemContext; // 2
// class DictItemContext; // 3
// class ArrayItemContext; // 4
// class ArrayValueItemContext; // 5

class ChildValueItemContext; // 1
class ChildKeyValueItemContext; // 2
class ChildDictItemContext; // 3
class ChildArrayItemContext; // 4
class ChildArrayItemValueContext; // 5
  
    ChildValueItemContext Key(std::string key);
    Builder& Value(Node::Value value);

    ChildDictItemContext StartDict();
    Builder& EndDict();

    ChildArrayItemContext StartArray();
    Builder& EndArray();
    Node Build();

protected:


private:
    Node root_;
    std::vector<Node*> nodes_stack_;
    bool no_content_ = true;

public:

    class CommonContext {
    public:
        CommonContext(Builder& builder)
            : builder_(builder) {}

        Builder::ChildValueItemContext Key(std::string key);
        Builder& Value(Node::Value value);
        Builder::ChildDictItemContext StartDict();
        Builder& EndDict();
        Builder::ChildArrayItemContext StartArray();
        Builder& EndArray();

    protected:
        Builder& builder_;
    };

// 2
class ChildKeyValueItemContext final : public CommonContext {
public:
    ChildKeyValueItemContext(Builder& builder)
        : CommonContext(builder) {}

    Builder& Value(Node::Value value) = delete;
    Builder::ChildDictItemContext StartDict() = delete;
    Builder::ChildArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;

};

// 1
class ChildValueItemContext final : public CommonContext {
public:
    ChildValueItemContext(Builder& builder)
        : CommonContext(builder) {}

    // Вызов Value после Key - попадаешь в контекст №2
    // Перекрытие имени базового класса
    ChildKeyValueItemContext Value(Node::Value value);
    Builder::ChildValueItemContext Key(std::string key) = delete;
    Builder& EndDict() = delete;
    Builder& EndArray() = delete;

};

// 3
class ChildDictItemContext : public CommonContext {
public:
    ChildDictItemContext(Builder& builder)
        : CommonContext(builder) {}

    Builder& Value(Node::Value value) = delete;
    Builder::ChildDictItemContext StartDict() = delete;
    Builder::ChildArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;

};

// 4
class ChildArrayItemContext : public CommonContext {
public:
    ChildArrayItemContext(Builder& builder)
        : CommonContext(builder) {}
    ChildArrayItemValueContext Value(Node::Value value);
    Builder::ChildValueItemContext Key(std::string key) = delete;
    Builder& EndDict() = delete;

};

// 5
class ChildArrayItemValueContext : public CommonContext {
public:
    ChildArrayItemValueContext(Builder& builder)
        : CommonContext(builder) {}

    ChildValueItemContext Key(std::string key) = delete;
    ChildArrayItemValueContext Value(Node::Value value);
    Builder& EndDict() = delete;

};
};
} // namespace json