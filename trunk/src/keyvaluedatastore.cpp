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
#include <sys/stat.h>
#include <sys/types.h>
#include <ftw.h>

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

#ifndef NOLEVELDB
class KeyComparator : public leveldb::Comparator
    {

   public:
	virtual int Compare(const leveldb::Slice& a, const leveldb::Slice& b) const
	    {
		DataStore::database_t db1=*((DataStore::database_t*)a.data());
		DataStore::database_t db2=*((DataStore::database_t*)b.data());
	    int i=db1-db2;
	    if(i!=0) return i;
	    const char* s1=&((char*)a.data)[sizeof(DataStore::database_t)];
	    const char* s2=&((char*)b.data)[sizeof(DataStore::database_t)];
	    return strcmp(s1,s2);
	    }
	virtual ~KeyComparator() {}
	virtual const char* Name() const { return "Key#Comparator"; }
	virtual void FindShortestSeparator(std::string*, const leveldb::Slice&) const { }
	virtual void FindShortSuccessor(std::string*) const { }
    };
#endif

DataStore::DataStore():handler(NULL),db_home(NULL),comparator(NULL)
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
	close();
#ifndef NOLEVELDB
    if(dir==NULL)
        {
    	temporary_db=true;

		char folder[FILENAME_MAX];
		strncpy(folder,"_leveldbXXXXXX.tmp");
		if(::mkdtemp(folder)==NULL)
			{
			THROW("cannot generate temporary dir");
			}
		if(mkdir(folder, 0755)!=0)
			{
			THROW("cannot create temporary "<< folder);
			}
		db_home=new string(folder);
        }
    else
    	{
    	temporary_db=false;
    	db_home=new string(dir);
    	}
    comparator=new KeyComparator;
    leveldb::Options options;
    options.create_if_missing = true;
    options.error_if_exists=temporary_db;
    options.comparator=(leveldb::Comparator*)comparator;
    leveldb::DB* D=NULL;
    leveldb::Status status = leveldb::DB::Open(options,dir,&D);
    if(!status.ok())
		{
		close();
		THROW("Cannot open leveldb file "<< dir <<".\n");
		}
    handler=D;
#else
    THROW("Compiled without leveldb");
#endif
    }

static int _recursive_ftw(const char *fpath, const struct stat *sb, int typeflag)
      {
      if(S_ISREG(sb->st_mode))
		  {
		  remove(fpath);
		  }
      else if(S_ISDIR(sb->st_mode))
		  {
		  rmdir(fpath);
		  }
      return 0;
      }

void DataStore::close()
    {
#ifndef NOLEVELDB

    if(db_home!=NULL)
		{
    	if(temporary_db)
    	    	{
    			::ftw(db_home->c_str(),_recursive_ftw,1);
    	    	}
		delete db_home;
		db_home=NULL;
		}
    if(handler!=NULL)
		{
		delete HANDLER;
		handler=NULL;
		}
    if(comparator!=NULL)
    		 {
    	    delete ((KeyComparator*)comparator);
    	    comparator=NULL;
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
    leveldb::ReadOptions opt;
    leveldb::Status status = HANDLER->Get(opt, key1, value);
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


bool DataStore::contains(DataStore::database_t db,const char* key)
    {
#ifndef NOLEVELDB
	auto_ptr<string> p=get(db,key);
	return p.get()!=NULL;
#else
    THROW("Compiled without leveldb");
#endif
    }




