#include "numeric_cast.h"
#include "throw.h"
#include "json.h"

static void quote(std::ostream& out,const std::string& s )
    {
    out << "\"";
    out << "\"";
    }

Node::Node()
    {
    }

Node::~Node()
    {
    }

bool Node::isA(Node::Type t) const
    {
    return type()==t;
    }

bool Node::isNill() const { return isA(Node::NIL);}
bool Node::isBool() const { return isA(Node::BOOLEAN);}
bool Node::isInt() const { return isA(Node::INTEGER);}
bool Node::isDouble() const { return isA(Node::DOUBLE);}
bool Node::isArray() const { return isA(Node::ARRAY);}
bool Node::isObject() const { return isA(Node::OBJECT);}
void Node::_check(Node::Type t) { if(!isA(t)) {THROW("Bad json cast");} }
NilNode* Node::asNil() { _check(Node::NIL); return (NilNode*)this;}
BoolNode* Node::asBool() { _check(Node::BOOLEAN); return (BoolNode*)this;}
StringNode* Node::asString() { _check(Node::STRING); return (StringNode*)this;}
IntNode* Node::asInt() { _check(Node::INTEGER); return (IntNode*)this;}
DoubleNode* Node::asDouble() { _check(Node::DOUBLE); return (DoubleNode*)this;}
ArrayNode* Node::asArray() { _check(Node::ARRAY); return (ArrayNode*)this;}
ObjectNode* Node::asObject() { _check(Node::OBJECT); return (ObjectNode*)this;}


NilNode::NilNode()
    {
    }
NilNode::~NilNode()
    {
    }
void NilNode::print(std::ostream& out) const
    {
    out << "null";
    }

Node::Type NilNode::type() const
    {
    return Node::NIL;
    }

Node* NilNode::clone() const
    {
    return new NilNode;
    }


BoolNode::BoolNode(bool b):value(b)
    {
    }

BoolNode::~BoolNode()
    {
    }

Node::Type BoolNode::type() const
    {
    return Node::BOOLEAN;
    }

Node* BoolNode::clone() const
    {
    return new BoolNode(this->value);
    }

void BoolNode::print(std::ostream& out) const
    {
    out << (value?"true":"false");
    }


IntNode::IntNode(int64_t v):value(v)
    {

    }
IntNode::~IntNode()
    {

    }
Node::Type IntNode::type() const
    {
    return Node::INTEGER;
    }

Node* IntNode::clone() const
    {
    return new IntNode(value);
    }

void IntNode::print(std::ostream& out) const
    {
    out << value;
    }



DoubleNode::DoubleNode(double v):value(v)
    {
    }

DoubleNode::~DoubleNode()
    {

    }
Node::Type DoubleNode::type() const
    {
    return Node::DOUBLE;
    }

Node* DoubleNode::clone() const
    {
    return new DoubleNode(value);
    }

void DoubleNode::print(std::ostream& out) const
    {
    out << value;
    }


StringNode::StringNode(const char* s):value(s)
    {
    }

StringNode::StringNode(const std::string& s):value(s)
    {
    }

StringNode::StringNode()
    {
    }

StringNode::~StringNode()
    {
    }

Node::Type StringNode::type() const
    {
    return Node::STRING;
    }

Node* StringNode::clone() const
    {
    return new StringNode(this->value);
    }

void StringNode::print(std::ostream& out) const
    {
    quote(out,value);
    }



ArrayNode::ArrayNode()
    {
    }
ArrayNode::~ArrayNode()
    {
    while(!array.empty())
	{
	delete array.back();
	array.pop_back();
	}
    }

Node::Type ArrayNode::type() const
    {
    return Node::ARRAY;
    }

Node* ArrayNode::clone() const
    {
    ArrayNode* cp=new ArrayNode;
    cp->array.reserve(this->array.size());
    for(std::vector<Node*>::size_type i=0;i<array.size();++i)
   	{
   	cp->array.push_back(this->array[i]->clone());
   	}
    return cp;
    }

void ArrayNode::print(std::ostream& out) const
    {
    out << "[";
    for(std::vector<Node*>::size_type i=0;i<array.size();++i)
	{
	if(i>0) out << ",";
	array[i]->print(out);
	}
    out << "]";
    }


ObjectNode::ObjectNode()
    {
    }
ObjectNode::~ObjectNode()
    {
    for(std::vector<void*>::size_type i=0;i+1<array.size();i+=2)
	{
	delete (std::string*)array[i+0];
	delete (Node*)array[i+1];
	}
    }
Node::Type ObjectNode::type() const
    {
    return Node::OBJECT;
    }

Node* ObjectNode::clone() const
    {
    ObjectNode* cp=new ObjectNode;
    cp->array.reserve(this->array.size());
    for(std::vector<void*>::size_type i=0;i+1<array.size();i+=2)
    	{
    	cp->array.push_back(new std::string(*((std::string*)array[i+0])));
    	cp->array.push_back(((Node*)array[i+1])->clone());
    	}
    return cp;
    }

void ObjectNode::print(std::ostream& out) const
    {
    out << "{";
    for(std::vector<void*>::size_type i=0;i+1<array.size();i+=2)
	{
	const std::string* k= (std::string*)array[i+0];
	const Node* v= (Node*)array[i+1];
	if(i>0) out << ",";
	quote(out,*k);
	out << ":";
	v->print(out);
	}
    out << "}";
    }

JSONParser::JSONParser(std::istream& in):in(in)
    {
    }

JSONParser::~JSONParser()
    {
    }

int JSONParser::get(int32_t n)
    {
    while(n<=buffer.size())
	{
	int c = in.get();
	if(c==-1) return -1;
	buffer.push_back(c);
	}
    return buffer[n];
    }

void JSONParser::consumme(int32_t n)
    {
    for(int32_t i=0;i< n;++i)
	{
	int c=get(0);
	if(c==-1) break;
	buffer.pop_front();
	}
    }

int JSONParser::skip_spaces()
    {
    while(std::isspace(get(0)))
	{
	consumme(1);
	}
    return get(0);
    }

bool JSONParser::in_avail(const char* s)
    {
    int32_t i=0;
    while(s[i]!=0)
	{
	if(s[i]!=get(i)) return false;
	}
    return true;
    }

StringNode* JSONParser::nextString()
    {
    std::ostringstream os;
    int c=skip_spaces();
    if(c!='\"')
	{
	return 0;
	}
    consumme(1);
    for(;;)
	{
	c=get(0);
	if(c=='\"')
	    {
	    consumme(1);
	    break;
	    }
	if(c=='\\')
	    {
	    consumme(1);
	    switch(get(0))
		{
		case 'n':os << "\n"; break;
		case 'r':os << "\r"; break;
		case 't':os << "\t"; break;
		case 'v':os << "\v"; break;
		case '\'':os << "\'"; break;
		case '\"':os << "\""; break;
		default: os << (char)c;break;
		}
	    consumme(1);
	    }
	else if(c==-1)
	    {
	    THROW("ok");
	    }
	else
	    {
	    consumme(1);
	    os << (char)c;
	    }
	}
    return 0;
    }

BoolNode* JSONParser::nextBool()
    {
    int c=skip_spaces();
    if(in_avail("true"))
	{
	consumme(4);
	return new BoolNode(true);
	}
    if(in_avail("false"))
    	{
    	consumme(5);
    	return new BoolNode(false);
    	}
    return 0;
    }

NilNode* JSONParser::nextNil()
    {
    skip_spaces();
    if(in_avail("null"))
	{
	consumme(4);
	return new NilNode;
	}
    return 0;
    }

ArrayNode* JSONParser::nextArray()
    {
    skip_spaces();
    if(!get('[')) return 0;
    ArrayNode* node=new ArrayNode;
    consumme(1);
    for(;;)
	{
	//TODO add item
	int c=skip_spaces();
	if(c==']')
	    {
	    consumme(1);
	    break;
	    }
	//TODO add item
	Node* item = next();

	c=skip_spaces();
	if(c==',')
	    {
	    consumme(1);
	    continue;
	    }
	THROW("syntax");
	}
    return node;
    }


ObjectNode* JSONParser::nextObject()
    {
    skip_spaces();
    if(!get('{')) return 0;
    ObjectNode* node=new ObjectNode;
    consumme(1);
    for(;;)
	{
	int c=skip_spaces();
	if(c=='}')
	    {
	    consumme(1);
	    break;
	    }
	else if(c!='\"')
	    {
	    THROW("syntax");
	    }
	std::string* key=parse_string();
	c=skip_spaces();
	if(c!=':')
	    {
	    THROW("syntax");
	    }
	skip_spaces();
	Node* item=next();
	c=skip_spaces();
	if(c==',')
	    {
	    consumme(1);
	    continue;
	    }
	if(c=='}')
	    {
	    consumme(1);
	    break;
	    }
	}
    return node;
    }

Node* JSONParser::nextNumber()
    {
    int c;
    std::ostringstream os;
    skip_spaces();
    while((c=get(0))!=-1)
	{
	if(!(std::isdigit(c) || c=='-' || c=='+' || c=='.' || c=='e' || c=='E'))
	    {
	    break;
	    }
	consumme(1);
	os << (char)c;
	}
    std::string s(os.str());
    int32_t v1;
    if(numeric_cast<int32_t>(s.c_str(),&v1))
	{
	return new IntNode(v1);
	}
    double v2;
    if(numeric_cast<double>(s.c_str(),&v2))
    	{
    	return new DoubleNode(v2);
    	}
    return 0;
    }

Node* JSONParser::next()
    {
    int c=skip_spaces();
    switch(c)
	{
	case -1: return 0;
	case 'n': return nextNil();
	case 't': return nextBool();
	case 'f': return nextBool();
	case '\"': return nextString();
	case '[': return nextArray();
	case '{': return nextObject();
	case '-':
	case '+':
	case '0': case '1': case '2':
	case '3': case '4': case '5':
	case '6': case '7': case '8':
	case '9': return nextNumber();
	default: THROW("error");
	}
    return 0;
    }
