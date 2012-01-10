#ifndef JSON_H
#define JSON_H
#include<iostream>
#include<string>
#include<vector>
#include<deque>
#include<stdint.h>

class NilNode;
class BoolNode;
class StringNode;
class IntNode;
class DoubleNode;
class ArrayNode;
class ObjectNode;

class Node
    {
    protected:
	Node();
    public:
	enum Type {NIL,BOOLEAN,INTEGER,DOUBLE,STRING,ARRAY,OBJECT};
	virtual ~Node();
	virtual Type type() const=0;
	virtual Node* clone() const=0;
	virtual bool isA(Node::Type t) const;
	virtual bool isNill() const;
	virtual bool isBool() const;
	virtual bool isInt() const;
	virtual bool isDouble() const;
	virtual bool isArray() const;
	virtual bool isObject() const;
	virtual NilNode* asNil();
	virtual BoolNode* asBool();
	virtual StringNode* asString();
	virtual IntNode* asInt();
	virtual DoubleNode* asDouble();
	virtual ArrayNode* asArray();
	virtual ObjectNode* asObject();
	virtual void print(std::ostream& out) const=0;
    private:
	void _check(Type t);
    };

class NilNode:public Node
    {
    public:
	NilNode();
	virtual ~NilNode();
	virtual Type type() const;
	virtual Node* clone() const;
	virtual void print(std::ostream& out) const;
    };

class BoolNode: public Node
    {
    private:
	bool value;
    public:
	BoolNode(bool b);
	virtual ~BoolNode();
	virtual Type type() const;
	virtual Node* clone() const;
	virtual void print(std::ostream& out) const;
    };


class IntNode: public Node
    {
    private:
	int64_t value;
    public:
	IntNode(int64_t b);
	virtual ~IntNode();
	virtual Type type() const;
	virtual Node* clone() const;
	virtual void print(std::ostream& out) const;
    };

class DoubleNode: public Node
    {
    private:
	double value;
    public:
	DoubleNode(double b);
	virtual ~DoubleNode();
	virtual Type type() const;
	virtual Node* clone() const;
	virtual void print(std::ostream& out) const;
    };

class StringNode: public Node
    {
    private:
	std::string value;
    public:
	StringNode(const char* s);
	StringNode(const std::string& s);
	StringNode();
	virtual ~StringNode();
	virtual Type type() const;
	virtual Node* clone() const;
	virtual void print(std::ostream& out) const;
    };

class ArrayNode: public Node
    {
    private:
	std::vector<Node*> array;
    public:
	ArrayNode();
	virtual ~ArrayNode();
	virtual Type type() const;
	virtual Node* clone() const;
	virtual void print(std::ostream& out) const;
    };


class ObjectNode: public Node
    {
    private:
	std::vector<void*> array;
    public:
	ObjectNode();
	virtual ~ObjectNode();
	virtual Type type() const;
	virtual Node* clone() const;
	virtual void print(std::ostream& out) const;
    };

class JSONParser
    {
    private:
	std::istream& in;
	std::deque<int> buffer;
	int get(int32_t);
	void consumme(int32_t);
	int skip_spaces();
	bool in_avail(const char* s);
	std::string* parse_string();
    public:
	JSONParser(std::istream& in);
	~JSONParser();
	Node* next();
	Node* nextNumber();
	BoolNode* nextBool();
	NilNode* nextNil();
	StringNode* nextString();
	ArrayNode* nextArray();
	ObjectNode* nextObject();
    };

#endif
