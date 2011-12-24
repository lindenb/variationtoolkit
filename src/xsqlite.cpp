#include <cstring>
#include <cstdlib>
#include <sqlite3.h>
#include "xsqlite.h"
#include "throw.h"

const int Connection::READ_ONLY= SQLITE_OPEN_READONLY;
const int Connection::READ_WRITE= SQLITE_OPEN_READWRITE;
const int Connection::CREATE= SQLITE_OPEN_CREATE;

#define CAST_CON(p) ((sqlite3*)(p->_ptr))

Connection::Connection():_ptr(0)
    {
    }

Connection::~Connection()
    {
    close();
    }

int Connection::execute(const char* sql)
    {
    std::auto_ptr<Statement> stmt= prepare(sql);
    if(stmt->step()!= SQLITE_DONE) return -1;
    return 0;
    }


void Connection::open()
    {
    this->open("",READ_WRITE|CREATE);//tmp file
    }

void Connection::open(const char* filename)
    {
    this->open(filename,READ_ONLY);
    }

void Connection::open(const char* filename,int flags)
    {
    close();
    sqlite3* db=0;
    int ret= ::sqlite3_open_v2(
	  filename,   /* Database filename (UTF-8) */
	  &db,         /* OUT: SQLite db handle */
	  flags,              /* Flags */
	  0       /* Name of VFS module to use */
	);
    _ptr=db;
    }

void Connection::close()
    {
    if(_ptr!=0)
	{
	::sqlite3_close(CAST_CON(this));
	_ptr=0;
	}
    }

std::auto_ptr<Statement> Connection::prepare(const char* sql)
    {
    return prepare(sql,std::strlen(sql));
    }


std::auto_ptr<Statement> Connection::prepare(const char* sql,std::size_t len)
    {
    const char* pzTail=NULL;
    sqlite3_stmt *ppStmt=NULL;
    int ret=::sqlite3_prepare_v2(
      CAST_CON(this),            /* Database handle */
      (const char*)sql,       /* SQL statement, UTF-8 encoded */
      (int)len,              /* Maximum length of zSql in bytes. */
      &ppStmt,  /* OUT: Statement handle */
      &pzTail     /* OUT: Pointer to unused portion of zSql */
       );
    if(ret!=SQLITE_OK)
	{

	}
    return std::auto_ptr<Statement>(new Statement(this,ppStmt));
    }




#define CAST_STMT(p) ((sqlite3_stmt*)(p->_ptr))

Statement::Statement(Connection* owner,void* ptr):owner(owner),_ptr(ptr)
    {

    }

Statement::~Statement()
    {
    close();
    }

void Statement::close()
    {
    if(_ptr!=0)
	{
	 sqlite3_finalize(CAST_STMT(this));
	_ptr=0;
	}
    }

int Statement::step()
    {
    return ::sqlite3_step( CAST_STMT(this));
    }


int Statement::column_count()
    {
    return ::sqlite3_column_count(CAST_STMT(this));
    }

int Statement::reset()
    {
    return ::sqlite3_reset( CAST_STMT(this));
    }

int Statement::bind_double(int index1,double v)
    {
    return ::sqlite3_bind_double( CAST_STMT(this),index1,v);
    }

int Statement::bind_int(int index1,int32_t v)
    {
    return ::sqlite3_bind_int( CAST_STMT(this),index1,v);
    }

int Statement::bind_int(int index1,int64_t v)
    {
    return ::sqlite3_bind_int64( CAST_STMT(this),index1,v);
    }

int Statement::bind_null(int index1)
    {
    return ::sqlite3_bind_null( CAST_STMT(this),index1);
    }

int Statement::bind_string(int index1,const char* s)
    {
    return this->bind_string(index1,s,strlen(s));
    }

int Statement::bind_string(int index1,const char* s,std::size_t len)
    {
    char* p=(char*)std::malloc((len+1)*sizeof(char));
    if(p==0) THROW("Out of memory cannot alloc "<<len);
    std::memcpy(p,s,len*sizeof(char));
    p[len]=0;
    int ret=::sqlite3_bind_text( CAST_STMT(this),index1,s, (int)len,std::free);
    return ret;
    }

double Statement::get_double(int index1)
    {
    return  ::sqlite3_column_double(CAST_STMT(this),index1);
    }

int32_t Statement::get_int32(int index1)
    {
    return  ::sqlite3_column_int(CAST_STMT(this),index1);
    }

int64_t Statement::get_int64(int index1)
    {
    return  ::sqlite3_column_int64(CAST_STMT(this),index1);
    }

const char* Statement::get_string(int index1)
    {
    return  (const char*)::sqlite3_column_text(CAST_STMT(this),index1);
    }

#ifdef TEST_THIS_CODE
using namespace std;
#include <cstdio>
int main(int argc,char** argv)
    {
    Connection con;
    con.open("jeter.sqlite3",Connection::CREATE|Connection::READ_WRITE);
    con.execute("create table mytable(hello text)");
    con.close();
    std::printf("Done.\n");
    return 0;
    }
#endif
