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

class Connection
    {
    private:
	void* _ptr;
    public:
	static const int READ_ONLY;
	static const int READ_WRITE;
	static const int CREATE;
	Connection();
	~Connection();
	/** open tmp db */
	void open();
	/** open read-only */
	void open(const char* filename);
	/** open database */
	void open(const char* filename,int flags);
	/** close db */
	void close();
	/** execute */
	int execute(const char* sql);

	std::auto_ptr<Statement> prepare(const char* s);
	std::auto_ptr<Statement> prepare(const char* s,std::size_t len);
    };

class Statement
    {
    private:
	Connection* owner;
	void*_ptr;
	Statement(Connection* owner,void* ptr);
    public:
	~Statement();
	/** step */
	int step();
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
