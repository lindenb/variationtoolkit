#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cerrno>
#include <fstream>
#include <sstream>
#include <memory>
#include <set>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <db.h>
#include <algorithm>

//#define NOWHERE
#include "where.h"
#include "bam.h"
#include "bam1sequence.h"
#include "throw.h"

using namespace std;

typedef int64_t bgzf_filepos_t;    

#define DEFAULT_FOLDER_EXTENSION ".names.idx"

class ComparableBam1Record:public Bam1Record
	{
	private:
		bam_header_t* bam_header;
	public:
		ComparableBam1Record(bam_header_t* bam_header,const bam1_t* ptr):Bam1Record(ptr),bam_header(bam_header)
			{
			}
		
		ComparableBam1Record(const ComparableBam1Record& cp):Bam1Record(cp),bam_header(cp.bam_header)
			{
			}
		
		virtual ~ComparableBam1Record()
			{
			}
		const char* chromosome() const
			{
			if(tid()<0 || !is_mapped()) return 0;
			return bam_header->target_name[tid()];
			}
		string format() const
			{
			char *x= bam_format1(bam_header, ptr());
	            	string s(x);
	            	free(x);
	            	return s;
			}
		
		int compareChromosome(const char* s1,const char* s2) const
			{
			if(s1[0]=='0') return compareChromosome(&s1[1],s2);
			if(s2[0]=='0') return compareChromosome(s1,&s2[1]);
			if(strncmp(s1,"chr",3)==0) return compareChromosome(&s1[3],s2);
			if(strncmp(s2,"chr",3)==0) return compareChromosome(s1,&s2[3]);
			if(strcmp(s1,"MT")==0) return compareChromosome("M",s2);
			if(strcmp(s2,"MT")==0) return compareChromosome(s1,"M");
			return strcmp(s1,s2);
			}
			
		
		int compareTo(const ComparableBam1Record& cp) const
			{
			int i=strcmp(name(),cp.name());
			if(i!=0) return i;
			int side1=0+(is_read1()?2:0)+(is_read2()?4:0);
			int side2=0+(cp.is_read1()?2:0)+(cp.is_read2()?4:0);
			i=side1-side2;
			if(i!=0) return i;
			
			if(chromosome()==0)
				{
				if(cp.chromosome()==0) return 0;
				
				return -1;
				}
        		if(cp.chromosome()==0)
        			{
   
        			return 1;
        			}
        		i=compareChromosome(chromosome(),cp.chromosome());
        		if(i!=0)return i;
        		i=pos()-cp.pos();
        		if(i!=0) return i;
        		int strand1=(is_reverse_strand()?-1:1);
        		int strand2=(cp.is_reverse_strand()?-1:1);
        		i=strand1-strand2;
        		if(i!=0) return i;
        		
        		return 0;
			}
		bool operator==(const ComparableBam1Record& cp) const
			{
			return this==&cp || compareTo(cp)==0;
			}
		bool operator<(const ComparableBam1Record& cp) const
			{
			return compareTo(cp)<0;
			}
		ComparableBam1Record& operator=(const ComparableBam1Record& cp)
			{
			if(this!=&cp)
				{
				Bam1Record::operator=(cp);
				bam_header=cp.bam_header;
				}
			return *this;
			}
		
		void print1(ostream& out) const
			{
			if(is_read1()) out << "[1]";
			if(is_read2()) out << "[2]";
			if(chromosome()==0)
				{
				out << "0";
				}
			else
				{
				out << chromosome()<<"("<< strand()<< "):"<< pos();
				}
			}
	};




class AbstractIndexOfNames
	{
	public:
		  /** input BAM */
        	bamFile in;
        	bam_header_t* bam_header;
		std::string* db_home;
		/** berkeleyDB database */
		DB* dbp;
		
	protected:
		AbstractIndexOfNames():in(0),bam_header(0),db_home(0),dbp(0)
			{
			
			}
	public:
		
		
		std::auto_ptr<vector<bgzf_filepos_t> > decode(const string& s)const
		    {
		    size_t n;
		    std::istringstream r(s);
		    r.read( (char*)&n,sizeof(size_t));
		    std::auto_ptr<vector<bgzf_filepos_t> > v(new vector<bgzf_filepos_t>());
		    v->reserve(n);
		    for(size_t i=0;i< n;++i)
		        {
		        bgzf_filepos_t h;
		        r.read( (char*)&h,sizeof(bgzf_filepos_t));
		        v->push_back(h);
		        }
		    return v;            
		    }
		   
		std::auto_ptr<vector<bgzf_filepos_t> > decode(const DBT* slice)const
		    {
		    std::string s((const char*)slice->data,slice->size);
		    return decode(s);            
		    }
		
		std::auto_ptr<std::string> encode(const std::vector<bgzf_filepos_t>* v) const
		    {
		    size_t n=v->size();
		    ostringstream os;
		    os.write((char*)&n,sizeof(size_t));
		    for(n=0;n< v->size();++n)
		        {
		        os.write( (char*)&(v->at(n)),sizeof(bgzf_filepos_t));
		        }            
		    return std::auto_ptr<std::string>(new string(os.str()));
		    }	
		
	public:
		
		virtual ~AbstractIndexOfNames()	
			{
			
			}
			
		virtual void printHeader(std::ostream& out)
			{
			if(strlen(bam_header->text)>0)
        			{
    		    		out << bam_header->text<< endl;

        			}	
        		for (int i = 0; i < bam_header->n_targets; ++i)
        			{
				out	<< "@SQ\tSN:"<< bam_header->target_name[i]
					<<"\tLN:" <<  bam_header->target_len[i]
					<< "\n";
			
        			}
			}	
		
		virtual void open(const char* bamfile,const char* db_home)=0;
		
		virtual void _open(const char* bamfile,const char* user_db_home,bool readOnly)
			{
			close();
			
			this->in=bam_open(bamfile,"r");
			if(this->in==0)
				{WHERE("");
				THROW("Cannot open BAM \""<< bamfile <<"\".\n" << strerror(errno));
				}WHERE("");
			 this->bam_header=bam_header_read(this->in);WHERE("");
			if(user_db_home==0)
				{
				this->db_home=new string(bamfile);
				this->db_home->append(DEFAULT_FOLDER_EXTENSION);
				}
			else
				{
				this->db_home=new string(user_db_home);
				}
			
			int ret=0;
			
			
			if ((ret = ::db_create(&dbp, NULL, 0)) != 0)
				{
				THROW("db_create: " <<  db_strerror(ret));
				}
			
			if ((ret = dbp->open(
				this->dbp,
				NULL, 
				this->db_home->c_str(),
				NULL,
				(readOnly?DB_UNKNOWN:DB_BTREE),
				(readOnly?DB_RDONLY:DB_CREATE|DB_TRUNCATE) ,
				0664)) != 0)
				{
				
				this->dbp->err(dbp, ret, "%s", this->db_home->c_str());
				close();
				THROW("Cannot open database");
				}
			
			}
		
		virtual void close()
			{
			WHERE("");
			if(this->bam_header!=0)
				{
				bam_header_destroy(this->bam_header);
				this->bam_header=0;
				}
			if(this->in!=0)
				{
				bam_close(this->in);
			    	this->in=0;
			    	}
			 if(this->dbp!=0)
			 	{
			 	this->dbp->close(this->dbp, 0);
			 	this->dbp=0;
			 	}
			}
	};

class ShortReadNameChanger
	{
	public:
		ShortReadNameChanger() {}
		virtual ~ShortReadNameChanger() {}
		virtual const char* name() const=0;
		virtual const char* description() const=0;
		virtual void change(std::string& name)=0;
	};

class ChangeNameBwaToCasava:public ShortReadNameChanger
	{
	private:
		int log_error;
		int log_change;
	public:
		ChangeNameBwaToCasava():log_error(10),log_change(10) {}
		virtual ~ChangeNameBwaToCasava() {}
		virtual const char* name() const { return "bwa2casava";}
		virtual const char* description() const
			{
			return "change HW-ST994:162:D0FE4ACXX:8:1103:16915:162319 to "
				"HWI-ST980_147:5:2104:16350:3671";
			}
		virtual void change(std::string& name)
			{
			std::string::size_type colons[3];
			for(int i=0;i<3;++i)
				{
				colons[i]=name.find(':',(i==0?0:colons[i-1]+1));
				if(colons[i]==string::npos)
					{
					if(log_error>0)
						{
						clog << "[WARNING] cannot find colon $"<<(i+1)<< "in " << name << endl;
						log_error--;
						}
					return;
					}
				}
			if(log_change>0)
				{
				clog << "[INFO] change \"" << name << "\" to \"";
				}
			name[colons[0]]='_';
			name.erase(colons[1],colons[2]);
			if(log_change>0)
				{
				clog  << name << "\"";
				log_change--;
				}
			}
	};
	
class WriteIndexOfNames:public AbstractIndexOfNames
	{
	public:
		ShortReadNameChanger* nameChanger;
		WriteIndexOfNames():nameChanger(0)
			{
			}
			
		virtual ~WriteIndexOfNames()
			{
			}
			
		virtual void open(const char* bamfile,const char* user_db_home)
		 	{
			_open(bamfile,user_db_home,false);
			}
		
		void build_index()
		    {
		    size_t n_reads=0;
		    DBT value;
		    clock_t start_time = clock( );
		   
		    
		    bam1_t *b= bam_init1();
		    
		  	
		    
		    

		    for(;;)
		        {
		        bgzf_filepos_t offset=bam_tell(this->in);
		        int n_read=bam_read1(this->in, b);
		        if(n_read<0) break;
		       
		        
		        std::auto_ptr<vector<bgzf_filepos_t> > hits(0);
		        Bam1Sequence seq(b);
		        if(seq.name()==0 || seq.name()[0]==0)
		        	{
		        	continue;
		        	}
		        std::string shortReadName(seq.name());
		        if(nameChanger!=0)
		        	{
		        	nameChanger->change(shortReadName);
		        	if(shortReadName.empty()) continue;
		        	}
		        DBT key1;
		        memset((void*)&key1,0,sizeof(DBT));
		        key1.data=(void*)shortReadName.data();
		        key1.size=shortReadName.size();
		        memset((void*)&value,0,sizeof(DBT));
		        
		        int status = this->dbp->get(
		        		this->dbp,
		        		NULL,
		        		&key1, &value, 0);
		     
		        if(status==DB_NOTFOUND)
		            {
		            hits.reset(new vector<bgzf_filepos_t>());
		            n_reads++;
		            if(n_reads%1000000UL==0)
		                {
		                clog <<  n_reads << " speed: " << n_reads/((clock()-start_time)/(float)(CLOCKS_PER_SEC))<< " reads/secs" << endl;
		                }
		            }
		        else if(status==0)
		            {
		            hits=decode(&value);
		            }
		        else
		           {
		           this->dbp->err(this->dbp, status, "DB->get");
		           break; 
		           }
		        
		        hits->push_back(offset);
		        
		        
		        std::auto_ptr<string> encoded = this->encode(hits.get());
		        memset((void*)&value,0,sizeof(DBT));
		        value.data=(void*)encoded->data();
		        value.size=encoded->size();
		        status = this->dbp->put(this->dbp, NULL, &key1, &value, 0);
		        if(status!=0)
		            {
		            this->dbp->err(this->dbp, status,  "[FATAL] Cannot insert into db");
		            break;
		            }
		        
		    
		        }
		   
		    bam_destroy1(b);
		    }
		
		
	};



class ReadIndexOfNames:public AbstractIndexOfNames
	{
	public:
		
		ReadIndexOfNames()
			{
			}
			
		virtual ~ReadIndexOfNames()
			{
			}
			
		virtual void open(const char* bamfile,const char* user_db_home)
		 	{
			_open(bamfile,user_db_home,true);
			}
		
		auto_ptr<ComparableBam1Record> getBamRecordAt(bgzf_filepos_t offset)
			{
			auto_ptr<ComparableBam1Record> ret(0);
			bam1_t *b= bam_init1();
			if(bam_seek(in,offset,SEEK_SET)!=0)
				{
				cerr << "[ERROR]Cannot seek bgzf to " <<   offset << endl;
				bam_destroy1(b);
				return ret;
				}
			if(bam_read1(in, b)<0)
				{
				cerr << "[ERROR]Cannot read BAM record at offset " <<   offset << endl;
				bam_destroy1(b);
				return ret;
				}
			ret.reset(new ComparableBam1Record(bam_header,b));
			bam_destroy1(b);
			return ret;
			}
		
		auto_ptr<set<ComparableBam1Record> > getBamRecordsAt(const vector<bgzf_filepos_t>& offsets)
			{
			auto_ptr<set<ComparableBam1Record> > ret(new set<ComparableBam1Record>());
			bam1_t *b= bam_init1();
			for(size_t i=0;i< offsets.size();++i)
				{
				bgzf_filepos_t offset=offsets.at(i);
				if(bam_seek(in,offset,SEEK_SET)!=0)
					{
					cerr << "[ERROR]Cannot seek bgzf to " <<   offset << endl;
					bam_destroy1(b);
					return ret;
					}
				if(bam_read1(in, b)<0)
					{
					cerr << "[ERROR]Cannot read BAM record at offset " <<   offset << endl;
					bam_destroy1(b);
					return ret;
					}
				ComparableBam1Record read(bam_header,b);
				
				ret->insert( read);
				}
			bam_destroy1(b);
			return ret;
			}
	};


class BamIndexNames
    {
    public:
    	AbstractIndexOfNames* indexOfNamePtr;
    	ChangeNameBwaToCasava changeNameBwa2Casava;
    	std::vector<ShortReadNameChanger*> availableNameChangers;
    	
    	
        BamIndexNames():indexOfNamePtr(0)
            {
            availableNameChangers.push_back(&changeNameBwa2Casava);
            }

         ~BamIndexNames()
		 {
		
		 }

       ShortReadNameChanger* findNameChangerByName(const char* s)
       		{
       		 for(size_t i=0;i< availableNameChangers.size();++i)
			{
			if(strcmp(availableNameChangers[i]->name(),s)==0) return  availableNameChangers[i];
			}
		return 0;
       		}

           void usage(ostream& out,int argc,char **argv)
            {
            out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
            out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
            out << "Usage:\n\t"<< argv[0] << " (command) args...\n";
            out << "Commands:\n";
            out << "   build (-f [db_home] )  file.bam #build the database.\n";
            out << "   names (-f [db_home] | -s [name] | -e [name] )  file.bam #dump all the read names and number of SAM records.\n";
            out << "Options:\n";
            out << "  -f (directory-name) OPTIONAL specifiy alternate database-home (default is file.bam+" << DEFAULT_FOLDER_EXTENSION << "\n";
            out << "  -e (name) OPTIONAL start from this read name\n";
            out << "  -b (name) OPTIONAL read until this read name\n";
            out << "  -C (name) use this short read name changed\n";
            out << "Available Name changers:\n";
             for(size_t i=0;i< availableNameChangers.size();++i)
             	{
             	out << "\t"
             		<< availableNameChangers[i]->name() << "\t"
             		<< availableNameChangers[i]->description() << endl;
             	}
            }
#define END_LOOP else if(strcmp(argv[optind],"--")==0)\
			        {\
			        ++optind;\
			        break;\
			        }\
			else if(argv[optind][0]=='-')\
			        {\
			        cerr << "unknown option '" << argv[optind]<< endl;\
			        usage(cerr,argc,argv);\
			        return(EXIT_FAILURE);\
			        }\
			else\
			        {\
			        break;\
			        }\
			++optind;
			     
#define COMMON_ARGS    else if(strcmp(argv[optind],"-f")==0 && optind+1<argc) \
			        {\
			        user_index_name=argv[++optind];\
			        }
        
        
        int compare_main(int argc,char** argv,int optind)
        	{
        	bool print_both=true;
        	bool print_only_in_0=true;
        	bool print_only_in_1=true;
        	ReadIndexOfNames indexOfNames[2];
		char* user_index_name[2]={0,0};
        	
		while(optind < argc)
			{
			if(strcmp(argv[optind],"-h")==0)
			        {
			        usage(cout,argc,argv);
			        return EXIT_FAILURE;
			        }
			else if(strcmp(argv[optind],"-1")==0 && optind+1<argc) 
			        {
			        print_only_in_0=false;
			        }
			else if(strcmp(argv[optind],"-2")==0 && optind+1<argc) 
			        {
			        print_only_in_1=false;
			        }
			else if(strcmp(argv[optind],"-3")==0 && optind+1<argc) 
			        {
			        print_both=false;
			        }
			else if(strcmp(argv[optind],"-f1")==0 && optind+1<argc) 
			        {
			        user_index_name[0]=argv[++optind];
			        }
			else if(strcmp(argv[optind],"-f2")==0 && optind+1<argc) 
			        {
			        user_index_name[1]=argv[++optind];
			        }
			END_LOOP
			}
        	if(optind+2!=argc)
		        {
		        cerr << "Expected two BAM files.\n";
		        return EXIT_FAILURE;
		        }
		char* bamnames[2]={0,0};
		bamnames[0]=argv[optind++];
		bamnames[1]=argv[optind++];
        	indexOfNames[0].open(bamnames[0],user_index_name[0]);
        	indexOfNames[1].open(bamnames[1],user_index_name[1]);
        	
        	string value1;
        	
        	DBC *dbcp0;
        	DBT key0, data0;
        	
        	int ret;
        	if ((ret = indexOfNames[0].dbp->cursor(indexOfNames[0].dbp, NULL, &dbcp0, 0)) != 0)
        		{
			indexOfNames[0].dbp->err(indexOfNames[0].dbp, ret, "DB->cursor");
			return EXIT_FAILURE;
			}
		memset(&key0, 0, sizeof(DBT));
		memset(&data0, 0, sizeof(DBT));
        	 
        	
        	
		while ((ret = dbcp0->c_get(dbcp0, &key0, &data0, DB_NEXT)) == 0)
		 	{
		 	DBT data1;
		 	memset(&data1, 0, sizeof(DBT));
		 	string readName((const char*)key0.data,key0.size);
        		
        		
        		std::auto_ptr<vector<bgzf_filepos_t> > hits0=indexOfNames[0].decode(&data0);
		 	std::auto_ptr<vector<bgzf_filepos_t> > hits1(0);

        		int status = indexOfNames[1].dbp->get(
        			indexOfNames[1].dbp,
   				NULL,
   				&key0,
   				&data1,
   				0);
		         if(status==0)
		            	{
		            	hits1=indexOfNames[1].decode(&data1);
		 	 	}
		 	 else
		 	 	{
		 	 	hits1.reset(new vector<bgzf_filepos_t> );
		 	 	}
		 	
		 	 
		 	
		 	 
		 	auto_ptr<set<ComparableBam1Record> > recs0=indexOfNames[0].getBamRecordsAt(*(hits0));
		 	auto_ptr<set<ComparableBam1Record> > recs1=indexOfNames[1].getBamRecordsAt(*(hits1));
		  	
		  	
		  	
		  	
		  	
		  	if(print_only_in_0)
			  	{
			  	set<ComparableBam1Record> only_in_0;
			  	std::set_difference(
			  			recs0->begin(),recs0->end(),
			  			recs1->begin(),recs1->end(),
	    					std::inserter(only_in_0, only_in_0.end())
	    					);
	    			if(!only_in_0.empty())
	    				{
	    				cout << readName << "\tONLY_1\t";
	    				for(set<ComparableBam1Record>::iterator r=only_in_0.begin();
	    					r!=only_in_0.end();
	    					++r)
		    				{
		    				if(r!=only_in_0.begin()) cout << "|";
		    				(*r).print1(cout);
		    				}
		    			cout << endl;
		    			}
		    		}
    			
			if(print_only_in_1)
			  	{
			  	set<ComparableBam1Record> only_in_1;
		  		std::set_difference(
		  			recs1->begin(),recs1->end(),
		  			recs0->begin(),recs0->end(),
    					std::inserter(only_in_1, only_in_1.end())
    					);
    				if(!only_in_1.empty())
	    				{
	    				cout << readName << "\tONLY_2\t";
	    				for(set<ComparableBam1Record>::iterator r=only_in_1.begin();
	    					r!=only_in_1.end();
	    					++r)
		    				{
		    				if(r!=only_in_1.begin()) cout << "|";
		    				(*r).print1(cout);
		    				}
		    			cout << endl;
		    			}
    				}
    			if(print_both)
	    			{
	    			set<ComparableBam1Record> in_both;
	    			std::set_intersection(
	    				recs0->begin(),recs0->end(),
			  		recs1->begin(),recs1->end(),
			  		std::inserter(in_both, in_both.end())
			  		);
			  	
			  	if(!in_both.empty())
	    				{
	    				cout << readName << "\tBOTH\t";
	    				for(set<ComparableBam1Record>::iterator r=in_both.begin();
	    					r!=in_both.end();
	    					++r)
		    				{
		    				if(r!=in_both.begin()) cout << "|";
		    				(*r).print1(cout);
		    				}
		    			cout << endl;
		    			}
		    		}
    			
		  	}
		dbcp0->c_close(dbcp0);
		
        	if(print_only_in_1)
			{
			DBC *dbcp1;
			DBT key1,data1;
			memset(&key1, 0, sizeof(DBT));
		 	memset(&data1, 0, sizeof(DBT));
		 	
		 	if ((ret = indexOfNames[1].dbp->cursor(indexOfNames[1].dbp, NULL, &dbcp1, 0)) != 0)
				{
				indexOfNames[1].dbp->err(indexOfNames[1].dbp, ret, "DB->cursor");
				return EXIT_FAILURE;
				}
		 	
			
			while ((ret = dbcp1->c_get(dbcp1, &key1, &data1, DB_NEXT)) == 0)
			 	{
			 	memset(&data0, 0, sizeof(DBT));
				 //leveldb::Slice key1(line);
				 int status = indexOfNames[0].dbp->get(
					indexOfNames[0].dbp,
					NULL,
					&key1,
					&data0,
					0);
				 if(status!=DB_NOTFOUND) continue;//already processed
				 
				 std::auto_ptr<vector<bgzf_filepos_t> > hits1=indexOfNames[1].decode(&data1); 	
				 auto_ptr<set<ComparableBam1Record> > recs1=indexOfNames[1].getBamRecordsAt(*(hits1));
				 if(!recs1->empty())
	    				{
	    				string readName((const char*)key1.data,key1.size);
	    				cout << readName << "\tONLY_2\t";
	    				for(set<ComparableBam1Record>::iterator r=recs1->begin();
	    					r!=recs1->end();
	    					++r)
		    				{
		    				if(r!=recs1->begin()) cout << "|";
		    				(*r).print1(cout);
		    				}
		    			cout << endl;
		    			}
			  	}
			dbcp1->c_close(dbcp1);
			}
        	
        	indexOfNames[0].close();
        	indexOfNames[1].close();
        	return EXIT_SUCCESS;
        	}

        
        
        int build_main(int argc,char** argv,int optind)
        	{
        	WriteIndexOfNames indexOfNames;
		char* user_index_name=0;
        	
		while(optind < argc)
			{
			if(strcmp(argv[optind],"-h")==0)
			        {
			        usage(cout,argc,argv);
			        return EXIT_FAILURE;
			        }
			else if(strcmp(argv[optind],"C")==0 && optind+1<argc)
				{
				indexOfNames.nameChanger=findNameChangerByName(argv[++optind]);
				if(indexOfNames.nameChanger==0)
					{
					cerr << "Unknown name changed: "<< argv[optind]<< endl;
					return EXIT_FAILURE;
					}
				}
			COMMON_ARGS
			END_LOOP
			
			}
		
        	if(optind+1!=argc)
		        {
		        cerr << "Expected one and only one BAM file.\n";
		        return EXIT_FAILURE;
		        }
        	indexOfNames.open(argv[optind],user_index_name);
        	indexOfNames.build_index();
        	indexOfNames.close();
        	return EXIT_SUCCESS;
        	}
        
        
        
         int many_main(int argc,char** argv,int optind)
        	{
        	bool print_sam_header=false;
        	vector<char*> pattern_names;
        	ReadIndexOfNames indexOfNames;
		char* user_index_name=0;
        	bool print_only_not_found=false;
		while(optind < argc)
			{
			if(strcmp(argv[optind],"-h")==0)
			        {
			        usage(cout,argc,argv);
			        return EXIT_FAILURE;
			        }
			COMMON_ARGS
			else if(strcmp(argv[optind],"-H")==0)
			        {
			        print_sam_header=true;
			        }
			else if(strcmp(argv[optind],"-R")==0 && optind+1<argc)
			        {
			        pattern_names.push_back(argv[++optind]);
			        }
			else if(strcmp(argv[optind],"-v")==0)
			        {
			       print_only_not_found=true;
			        }
			END_LOOP
			}
		if(pattern_names.empty())
			{
		 	cerr << "No pattern file defined.\n";
		        return EXIT_FAILURE;
			}
        	if(optind+1!=argc)
		        {
		        cerr << "Expected one and only one BAM file.\n";
		        return EXIT_FAILURE;
		        }
        	indexOfNames.open(argv[optind],user_index_name);
        	
        	
        	if(print_sam_header && indexOfNames.bam_header->text!=0)
        		{
        		indexOfNames.printHeader(cout);
        		}
        	bam1_t *b= bam_init1();
        	for(size_t i=0;i< pattern_names.size();++i)
        		{
        		ifstream in(pattern_names[i],ios::in);
        		if(!in.is_open())
        			{
        			cerr << "Cannot open " << pattern_names[i] << " " << strerror(errno) << endl;
        			}
        		DBT key, data;
        		std::string line;
        		while(getline(in,line,'\n'))
        			{
        			if(line.empty() || line[0]=='#') continue;
        			memset(&key, 0, sizeof(key));
				memset(&data, 0, sizeof(data));
				key.data=(char*)line.data();
				key.size=line.size();
				
        			int status =  indexOfNames.dbp->get(indexOfNames.dbp, NULL, &key, &data, 0);
		        	if(status!=0)
		            		{
		            		if(print_only_not_found)
		            			{
		            			cout <<  line << endl;
		            			}
		            		else	
		            			{
		            			cerr << "#!NOT_FOUND:" << line << endl;
		            			}
		            		}
		            	else if(print_only_not_found==false)
	            			{
	            			std::auto_ptr<vector<bgzf_filepos_t> > hits=indexOfNames.decode(&data);
	            			for(size_t j=0;j< hits->size();++j)
	            				{
	            				bgzf_filepos_t offset=hits->at(j);
	            				if(bam_seek(indexOfNames.in,offset,SEEK_SET)!=0)
	            					{
	            					cerr << "[ERROR]Cannot seek bgzf to " <<   offset << endl;
	            					continue;
	            					}
	            				if(bam_read1(indexOfNames.in, b)<0)
	            					{
	            					cerr << "[ERROR]Cannot read BAM record at offset " <<   offset << endl;
	            					continue;
	            					}
	            				char *ptr= bam_format1(indexOfNames.bam_header, b);
	            				cout << ptr << endl;
	            				free(ptr);
	            				}
	            			}

        			}
        		in.close();
        		}
        	bam_destroy1(b);
        	indexOfNames.close();
        	return EXIT_SUCCESS;
        	}
        
        
        int names_main(int argc,char** argv,int optind)
        	{
        	ReadIndexOfNames indexOfNames;
		char* user_index_name=0;
        	char* nameBegin=0;
        	char* nameEnd=0;
		while(optind < argc)
			{
			if(strcmp(argv[optind],"-h")==0)
			        {
			        usage(cout,argc,argv);
			        return EXIT_FAILURE;
			        }
			COMMON_ARGS
			else if(strcmp(argv[optind],"-b")==0 && optind+1<argc)
			        {
			        nameBegin=argv[++optind];
			        }
			else if(strcmp(argv[optind],"-e")==0 && optind+1<argc)
			        {
			        nameEnd=argv[++optind];
			        }     
			END_LOOP
			}
		
        	if(optind+1!=argc)
		        {
		        cerr << "Expected one and only one BAM file.\n";
		        return EXIT_FAILURE;
		        }
        	indexOfNames.open(argv[optind],user_index_name);
        	
        	DBC *dbcp;
        	int ret;
        	 /* Acquire a cursor for the database. */
	       if ((ret = indexOfNames.dbp->cursor(indexOfNames.dbp, NULL, &dbcp, 0)) != 0)
	       		{
			indexOfNames.dbp->err(indexOfNames.dbp, ret, "DB->cursor");
			return EXIT_FAILURE;
			}
        	/* Initialize the key/data return pair. */
        	DBT key, data;
        	bool first=true;
		memset(&key, 0, sizeof(key));
		memset(&data, 0, sizeof(data));
        	if(nameBegin!=0)
        		{
        		key.data=nameBegin;
        		key.size=strlen(nameBegin);
        		}
    		
		 for (; ;)
		 	{
		 	 if(first && nameBegin!=0)	
		 	 	{
		 	 	ret = dbcp->c_get(dbcp,&key, &data, DB_SET_RANGE);
		 	 	}
		 	 else
		 	 	{
		 	 	ret = dbcp->c_get(dbcp, &key, &data, DB_NEXT);
		 	 	}
		 	 first=false;
		 	 std::auto_ptr<vector<bgzf_filepos_t> > hits=indexOfNames.decode(&data);
		 	 string readName((char*)key.data,key.size);
		 	 if(nameEnd!=0)
		 	 	{
		 	 	if(readName.compare(nameEnd)>0) break;
		 	 	}
		    	cout << readName << "\t" << hits->size() << endl;
		  	}
		 dbcp->c_close(dbcp);
        	indexOfNames.close();
        	return EXIT_SUCCESS;
        	}
        
        
        int main(int argc, char *argv[])
            {
            int optind=1;
            
            
            while(optind < argc)
                {
                if(strcmp(argv[optind],"-h")==0)
                        {
                        usage(cout,argc,argv);
                        return EXIT_FAILURE;
                        }
                END_LOOP
                }
                
                if(optind==argc)
		        {
		        usage(cout,argc,argv);
		        return EXIT_FAILURE;
		        }
		
		if(strcmp(argv[optind],"build")==0)
			{
			return build_main(argc,argv,optind+1);
			}
		else if(strcmp(argv[optind],"names")==0)
			{
			return names_main(argc,argv,optind+1);
			}
		else if(strcmp(argv[optind],"many")==0)
			{
			return many_main(argc,argv,optind+1);
			}
		else if(strcmp(argv[optind],"compare")==0)
			{
			return compare_main(argc,argv,optind+1);
			}
		else
			{
			cerr<<  "Bad command :" << argv[optind]<< endl;
			return EXIT_FAILURE;
			}
            return EXIT_SUCCESS;
            }
            
    };

int main(int argc,char** argv)
    {
    BamIndexNames app;
    return app.main(argc,argv);
    }
