/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 * Date:
 *	Oct 2011
 * WWW:
 *	http://plindenbaum.blogspot.com
 * Motivation:
 *	key value datastore using 3 engines:
 *	* leveldb
 */
#ifndef KEY_VALUE_DS_H
#define KEY_VALUE_DS_H
#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <vector>
#include <fstream>
#include <list>
#include <cstring>
#include <sstream>
#include <memory>
#include <stdint.h>

class DataStore
    {
    public:
	typedef int32_t database_t;
        DataStore();
        ~DataStore();
        void open(const char* dir);
        void open();
        void close();
        void clear();
        void clear(database_t db);
        bool contains(database_t db,const char* key);
        bool put(database_t db,const char* key,const char* value);
        std::auto_ptr<std::string> get(database_t db,const char* key);
        bool rm(database_t db,const char* key);
    private:
        void* handler;
        std::string* db_home;
        bool temporary_db;
        void* comparator;
    };
#endif
