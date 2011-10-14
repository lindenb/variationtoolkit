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
 *
 */
#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <vector>
#include <fstream>
#include <list>
#include <cstring>
#include <sstream>
#include <memory>
#ifndef NOLEVELDB
#include <leveldb/db.h>
#endif
#include "throw.h"
#include "where.h"

#include "keyvaluedatastore.h"


using namespace std;

#define strequals(a,b) (std::strcmp(a,b)==0)

#ifndef NOLEVELDB
#define HANDLER ((leveldb::DB*)handler)
#endif

class Wrapper
	    {
	    public:
		size_t len;
		char* ptr;
		Wrapper(DataStore::database_t d,const char* s)
		    {
		    size_t L=strlen(s);
		    len=L+1+sizeof(DataStore::database_t);
		    ptr=new char[len];
		    memcpy(ptr,&d,sizeof(DataStore::database_t));
		    memcpy(&ptr[sizeof(DataStore::database_t)],s,L+1);
		    }
		~Wrapper()
		    {
		    delete[] ptr;
		    }
	    };


DataStore::DataStore():handler(NULL),db_home(NULL)
    {

    }

DataStore::~DataStore()
    {
    close();
    }

void DataStore::open()
    {
    open(NULL);
    }

void DataStore::open(const char* dir)
    {
#ifndef NOLEVELDB
    if(dir==NULL)
        {
        THROW("DB_HOME undefined.");
        }
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::DB* D=NULL;
    leveldb::Status status = leveldb::DB::Open(options,dir,&D);
    if(!status.ok())
	{
	handler=NULL;
	THROW("Cannot open leveldb file "<< dir <<".\n");
	}
    handler=D;
#else
    THROW("Compiled without leveldb");
#endif
    }

void DataStore::close()
    {
#ifndef NOLEVELDB
    if(db_home!=NULL)
	{
	delete db_home;
	db_home=NULL;
	}
    if(handler!=NULL)
	{
	delete HANDLER;
	handler=NULL;
	}
#endif
    }

bool DataStore::rm(DataStore::database_t db,const char* s)
    {
#ifndef NOLEVELDB
    Wrapper w(db,s);
    leveldb::Slice key1(w.ptr,w.len);
    leveldb::WriteOptions options;
    leveldb::Status status = HANDLER->Delete(options, key1);
    if(!status.ok())
	{
	return false;
	}
    return true;
#else
    THROW("Compiled without leveldb");
#endif

    }




bool DataStore::put(DataStore::database_t db,const char* key,const char* value)
    {
#ifndef NOLEVELDB
    Wrapper w1(db,key);
    leveldb::Slice key1(w1.ptr,w1.len);
    leveldb::Slice value1(value,strlen(value)+1);
    leveldb::WriteOptions opt;
    leveldb::Status status = HANDLER->Put(opt, key1, value1);
    if(!status.ok())
	{
	THROW("Could not insert:"  << status.ToString());
	}
    return true;
#else
    THROW("Compiled without leveldb");
#endif
    }

auto_ptr<string> DataStore::get(DataStore::database_t db,const char* key)
    {
#ifndef NOLEVELDB
    Wrapper w1(db,key);
    leveldb::Slice key1(w1.ptr,w1.len);
    std::string* value=new string;
    leveldb::Status status = HANDLER->Get(leveldb::ReadOptions(), key1, value);
    if(!status.ok())
	{
	delete value;
	value=NULL;
	}
    return auto_ptr<string>(value);
#else
    THROW("Compiled without leveldb");
#endif
    }






