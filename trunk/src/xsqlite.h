/*
 * xsqlite.h
 *
 *  Created on: Dec 24, 2011
 *      Author: lindenb
 */

#ifndef XSQLITE_H_
#define XSQLITE_H_
#include <memory>
#include <cstddef>
#include <string>

class Statement;
class Connection;

class ConnectionFactory
    {
    private:
	bool allow_create;
	bool read_only;
	std::auto_ptr<std::string> filename;
    public:
	ConnectionFactory();
	~ConnectionFactory();
	void set_read_only(bool b);
	void set_allow_create(bool b);
	void set_filename(const char* filename);
	std::auto_ptr<Connection> create();
    };


class Connection
    {
    private:
	void* _ptr;
	Connection(void* _ptr);
    public:
	~Connection();
	void begin();
	void commit();

	/** close db */
	void close();
	/** execute */
	int execute(const char* sql);
	/*http://www.sqlite.org/lang_vacuum.html */
	int compact();
	int64_t last_insert_id();
	std::auto_ptr<Statement> prepare(const char* s);
	std::auto_ptr<Statement> prepare(const char* s,std::size_t len);
    friend class ConnectionFactory;
    };

class Statement
    {
    private:
	Connection* owner;
	void*_ptr;
	Statement(Connection* owner,void* ptr);
    public:
	static const int DONE;
	~Statement();
	/** step */
	int step();
	void execute();
	int reset();
	void close();
	int column_count();
	int bind_double(int index1,double v);
	int bind_int(int index1,int32_t v);
	int bind_int(int index1,int64_t v);
	int bind_null(int index1);
	int bind_string(int index1,const char* s);
	int bind_string(int index1,const char* s,std::size_t len);
	double get_double(int index1);
	int32_t get_int32(int index1);
	int64_t get_int64(int index1);
	const char* get_string(int index1);
    friend class Connection;
    };



#endif /* XSQLITE_H_ */
