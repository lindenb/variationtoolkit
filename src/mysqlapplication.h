/*
 * mysqlapplication.h
 *
 *  Created on: Oct 18, 2011
 *      Author: lindenb
 */

#ifndef MYSQLAPPLICATION_H_
#define MYSQLAPPLICATION_H_
#include <cstring>
#include <string>
#include <mysql.h>
#include "throw.h"
#include "application.h"
class MysqlApplication: public AbstractApplication
{
public:
	MYSQL* mysql;
	std::string host;
	std::string username;
	std::string password;
	std::string database;
	int port;

	MysqlApplication():
		mysql(NULL),
		host("genome-mysql.cse.ucsc.edu"),
		username("genome"),
		database("hg19"),
		port(0)
		{
		if((mysql=::mysql_init(NULL))==NULL) THROW("Cannot init mysql");
		}
	virtual ~MysqlApplication()
		{
		::mysql_close(mysql);
		}

	virtual void connect()
		{
	    if(::mysql_real_connect(
	    	    mysql,
	    	    this->host.c_str(),
	    	    this->username.c_str(),
	    	    this->password.c_str(),
	    	    this->database.c_str(), port,NULL, 0 )==NULL)
	    	{
	    	THROW("mysql_real_connect failed "<< mysql_error(this->mysql));
	    	}
		}

	virtual int argument(int optind,int argc,char** argv)
		{
		if(std::strcmp(argv[optind],"--host")==0 && optind+1<argc)
			{
			this->host.assign(argv[++optind]);
			return optind;
			}
		else if(std::strcmp(argv[optind],"--user")==0 && optind+1<argc)
			{
			this->username.assign(argv[++optind]);
			return optind;
			}
		else if(std::strcmp(argv[optind],"--password")==0 && optind+1<argc)
			{
			this->password.assign(argv[++optind]);
			return optind;
			}
		else if(std::strcmp(argv[optind],"--database")==0 && optind+1<argc)
			{
			this->database.assign(argv[++optind]);
			return optind;
			}
		else if(std::strcmp(argv[optind],"--port")==0 && optind+1<argc)
			{
			this->port=atoi(argv[++optind]);
			return optind;
			}
		else
			{
			return AbstractApplication::argument(optind,argc,argv);
			}
		}


	void printConnectOptions(std::ostream& out)
		{
		out << "Mysql options:" << std::endl;
		out << "  --host <mysql host> default:" << host << std::endl;
		out << "  --user <mysql user> default:" << username << std::endl;
		out << "  --password <mysql password> default:" << password << std::endl;
		out << "  --database <mysql database> default:" << database << std::endl;
		out << "  --port <mysql password> default:" << port << std::endl;
		out << std::endl;
		}
};

#endif /* MYSQLAPPLICATION_H_ */
