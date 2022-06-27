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
*/
// class ValueItemContext; // 1
// class KeyValueItemContext; // 2
// class DictItemContext; // 3
// class ArrayItemContext; // 4
// class ArrayValueItemContext; // 5


class ValueItemContext2; // 1
class KeyValueItemContext2; // 2
class DictItemContext2; // 3
class ArrayItemContext2; // 4
class ArrayValueItemContext2; // 5
class Builder;

class CommonContext {
public:
    virtual ValueItemContext2 Key(std::string key) = 0;
    virtual Builder& Value(Node::Value value) = 0;
    virtual DictItemContext2 StartDict() = 0;
    virtual Builder& EndDict() = 0;
    virtual ArrayItemContext2 StartArray() = 0;
    virtual Builder& EndArray() = 0;
    virtual Node Build() = 0;
private:
};

class Builder {

// declaration
class ValueItemContext; // 1
class KeyValueItemContext; // 2
class DictItemContext; // 3
class ArrayItemContext; // 4
class ArrayValueItemContext; // 5

public:
    
    ValueItemContext Key(std::string key);
    Builder& Value(Node::Value value);

    DictItemContext StartDict();
    Builder& EndDict();

    ArrayItemContext StartArray();
    Builder& EndArray();
    Node Build();

protected:


private:
    Node root_;
    std::vector<Node*> nodes_stack_;
    bool no_content_ = true;


// Context 1
class ValueItemContext {
public:
    ValueItemContext(Builder& b)
        : builder_(b) {}
    KeyValueItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
   
private:
   Builder& builder_;
};

// констекст для случая 3
class DictItemContext {
public:
    DictItemContext(Builder& builder);

    ValueItemContext Key(std::string key);
    Builder& EndDict();

private:

    Builder& builder_;
};

class ArrayItemContext {
public:
    ArrayItemContext(Builder& b)
        : builder_(b) {}

    // тут
    ArrayValueItemContext Value(Node::Value value);
    DictItemContext StartDict() ;
    ArrayItemContext StartArray() ;
    Builder& EndArray() ;

private:
    Builder& builder_;
};

class KeyValueItemContext {
public:
    KeyValueItemContext(Builder& b) 
        : builder_(b) {}
    
    ValueItemContext Key(std::string key);
    Builder& EndDict();

private:
    Builder& builder_;
};

class ArrayValueItemContext {
public:
    ArrayValueItemContext(Builder& b) 
        : builder_(b) {}
    ArrayValueItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();

private:
    Builder& builder_;
};


};



} // namespace json