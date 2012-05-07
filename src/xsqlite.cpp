#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sqlite3.h>
#include "xsqlite.h"
#include "throw.h"
#include "where.h"
#define VERIFY_INDEX(i) if(i<1)  THROW("error need a 1-based index but got "<< i)

static std::string error_code(int err)
    {
    std::string msg;
    switch(err)
	{
	case SQLITE_OK:return msg;break;
	case SQLITE_ERROR: msg.append("SQLITE_ERROR");break;
	case SQLITE_INTERNAL: msg.append("SQLITE_INTERNAL");break;
	case SQLITE_PERM: msg.append("SQLITE_PERM");break;
	case SQLITE_ABORT: msg.append("SQLITE_ABORT");break;
	case SQLITE_BUSY: msg.append("SQLITE_BUSY");break;
	case SQLITE_LOCKED: msg.append("SQLITE_LOCKED");break;
	case SQLITE_NOMEM: msg.append("SQLITE_NOMEM");break;
	case SQLITE_READONLY: msg.append("SQLITE_READONLY");break;
	case SQLITE_INTERRUPT: msg.append("SQLITE_INTERRUPT");break;
	case SQLITE_IOERR: msg.append("SQLITE_IOERR");break;
	case SQLITE_CORRUPT: msg.append("SQLITE_CORRUPT");break;
	case SQLITE_NOTFOUND: msg.append("SQLITE_NOTFOUND");break;
	case SQLITE_FULL: msg.append("SQLITE_FULL");break;
	case SQLITE_CANTOPEN: msg.append("SQLITE_CANTOPEN");break;
	case SQLITE_PROTOCOL: msg.append("SQLITE_PROTOCOL");break;
	case SQLITE_EMPTY: msg.append("SQLITE_EMPTY");break;
	case SQLITE_SCHEMA: msg.append("SQLITE_SCHEMA");break;
	case SQLITE_TOOBIG: msg.append("SQLITE_TOOBIG");break;
	case SQLITE_CONSTRAINT: msg.append("SQLITE_CONSTRAINT");break;
	case SQLITE_MISMATCH: msg.append("SQLITE_MISMATCH");break;
	case SQLITE_MISUSE: msg.append("SQLITE_MISUSE");break;
	case SQLITE_NOLFS: msg.append("SQLITE_NOLFS");break;
	case SQLITE_AUTH: msg.append("SQLITE_AUTH");break;
	case SQLITE_FORMAT: msg.append("SQLITE_FORMAT");break;
	case SQLITE_RANGE: msg.append("SQLITE_RANGE");break;
	case SQLITE_NOTADB: msg.append("SQLITE_NOTADB");break;
	case SQLITE_ROW: msg.append("SQLITE_ROW");break;
	case SQLITE_DONE: msg.append("SQLITE_DONE");break;
	default: std::ostringstream os; os << "Unknown error: "<< err; msg.assign(os.str());break;
	}
    return msg;
    }




ConnectionFactory::ConnectionFactory():
    allow_create(false),
    read_only(true),
    filename(NULL)
    {

    }
ConnectionFactory::~ConnectionFactory()
    {

    }
void ConnectionFactory::set_read_only(bool b)
    {
    read_only=b;
    }
void ConnectionFactory::set_allow_create(bool b)
    {
    allow_create=b;
    }
void ConnectionFactory::set_filename(const char* f)
    {
    filename.reset(0);
    if(f!=0) filename.reset(new std::string(f));
    }

std::auto_ptr<Connection> ConnectionFactory::create()
    {
    std::auto_ptr<Connection> ret(NULL);
    int flag=0;
    if(read_only)
	{
	flag|=SQLITE_OPEN_READONLY;
	}
    else
	{
	flag|=SQLITE_OPEN_READWRITE;
	}
    if(allow_create)
	{
	FILE* fin=fopen(filename->c_str(),"rb");
	if(fin!=NULL)
	    {
	    fclose(fin);
	    }
	else
	    {
	    flag|=SQLITE_OPEN_CREATE;
	    }
	}
    if(filename.get()==NULL) THROW("Filename hasn't been defined");
    sqlite3* db=0;

    int err= ::sqlite3_open_v2(
    	  filename->c_str(),   /* Database filename (UTF-8) */
    	  &db,         /* OUT: SQLite db handle */
    	  flag,              /* Flags */
    	  0       /* Name of VFS module to use */
    	);

    if(err!=SQLITE_OK)
	{
	THROW("Cannot open db \""<< *(filename.get()) << "\":" << error_code(err));
	}
    ret.reset(new Connection((void*)db));
    return ret;
    }



#define CAST_CON(p) ((sqlite3*)(p->_ptr))


Connection::Connection(void* p):_ptr(p)
    {
    }

Connection::~Connection()
    {
    close();
    }



int Connection::execute(const char* sql)
    {
    std::auto_ptr<Statement> stmt= prepare(sql);
    stmt->execute();
    return 0;
    }

//http://www.sqlite.org/lang_vacuum.html
int Connection::compact()
    {
    return execute("VACUUM");
    }



void Connection::begin()
    {
    this->execute("BEGIN");
    }
void Connection::commit()
    {
    this->execute("COMMIT");
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
	std::string query(sql,len);
	THROW("failure " << error_code(ret) << (pzTail!=NULL?pzTail:"(no)") << ":" << query);
	}
    return std::auto_ptr<Statement>(new Statement(this,ppStmt));
    }


int64_t Connection::last_insert_id()
    {
    return ::sqlite3_last_insert_rowid(CAST_CON(this));
    }

#define CAST_STMT(p) ((sqlite3_stmt*)(p->_ptr))

const int Statement::DONE= SQLITE_DONE;

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
    int err= ::sqlite3_step( CAST_STMT(this));
    switch(err)
	{
	case SQLITE_DONE: return Statement::DONE;
	}
    return err;
    }

void Statement::execute()
    {
    int err;
    if((err=step())!=Statement::DONE) THROW("ERROR after execute:"<< error_code(err));
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
    VERIFY_INDEX(index1);
    return ::sqlite3_bind_double( CAST_STMT(this),index1,v);
    }

int Statement::bind_int(int index1,int32_t v)
    {
    VERIFY_INDEX(index1);
    return ::sqlite3_bind_int( CAST_STMT(this),index1,v);
    }

int Statement::bind_int(int index1,int64_t v)
    {
    VERIFY_INDEX(index1);
    return ::sqlite3_bind_int64( CAST_STMT(this),index1,v);
    }

int Statement::bind_null(int index1)
    {
    VERIFY_INDEX(index1);
    return ::sqlite3_bind_null( CAST_STMT(this),index1);
    }

int Statement::bind_string(int index1,const char* s)
    {
    VERIFY_INDEX(index1);
    return this->bind_string(index1,s,strlen(s));
    }

int Statement::bind_string(int index1,const char* s,std::size_t len)
    {
    VERIFY_INDEX(index1);
    char* p=(char*)std::malloc((len+1)*sizeof(char));
    if(p==0) THROW("Out of memory cannot alloc "<<len);
    std::memcpy(p,s,len*sizeof(char));
    p[len]=0;
    int ret=::sqlite3_bind_text( CAST_STMT(this),index1,p, (int)len,std::free);
    return ret;
    }

double Statement::get_double(int index1)
    {
    VERIFY_INDEX(index1);
    return  ::sqlite3_column_double(CAST_STMT(this),index1-1);
    }

int32_t Statement::get_int32(int index1)
    {
    VERIFY_INDEX(index1);
    return  ::sqlite3_column_int(CAST_STMT(this),index1-1);
    }

int64_t Statement::get_int64(int index1)
    {
    VERIFY_INDEX(index1);
    return  ::sqlite3_column_int64(CAST_STMT(this),index1-1);
    }

const char* Statement::get_string(int index1)
    {
    VERIFY_INDEX(index1);
    return  (const char*)::sqlite3_column_text(CAST_STMT(this),index1-1);
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
