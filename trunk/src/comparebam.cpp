#include <leveldb/db.h>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <sstream>
#include <memory>
#include <sys/stat.h>
#include <sys/types.h>
#include <ftw.h>

#define NOWHERE
#include "where.h"
#include "bam.h"
#include "bam1sequence.h"
#include "throw.h"

using namespace std;

#define IS_FLAG_SET(b,flg) (((b) & (flg))!=0)

class CompareBams
	{
	private:
		/** input BAM */
		samfile_t *in;
		std::string* db_home;
		leveldb::DB* db;
		vector<vector<string> > index2chromNames;
		leveldb::ReadOptions read_options;
		leveldb::WriteOptions write_options;
		struct Hit
			{
			unsigned short bamIndex;
			int flag;
			int32_t tid;
			int32_t pos;
			const bool mapped() const
				{
				return tid>=0 && !IS_FLAG_SET(flag,BAM_FUNMAP);
				}		
			};		
		

		static int _recursive_ftw(const char *fpath, const struct stat *sb, int typeflag)
		      {
		      clog << "[CLEANUP] "<< fpath << endl;
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
		std::auto_ptr<vector<Hit> > decode(const string& s)const
			{
			size_t n;
			std::istringstream r(s);
			r.read( (char*)&n,sizeof(size_t));
			std::auto_ptr<vector<Hit> > v(new vector<Hit>());
			v->reserve(n);
			for(size_t i=0;i< n;++i)
				{
				Hit h;
				r.read( (char*)&h,sizeof(Hit));
				v->push_back(h);
				}
			return v;			
			}
		std::auto_ptr<std::string> encode(const std::vector<Hit>* v) const
			{
			
			size_t n=v->size();
			ostringstream os;
			os.write((char*)&n,sizeof(size_t));
			for(n=0;n< v->size();++n)
				{
				os.write( (char*)&(v->at(n)),sizeof(Hit));
				}			
			return std::auto_ptr<std::string>(new string(os.str()));
			}

	public:
		CompareBams():in(0),db_home(0),db(0)
			{
			
			}

	

		void close()
			{
			if(this->in!=0) ::samclose(this->in);
			this->in=0;
			if(this->db!=NULL)
				{
				delete this->db;
				this->db=0;
				}
			if(this->db_home!=0)
				{
				::ftw(db_home->c_str(),_recursive_ftw,1);
				delete db_home;
				db_home=0;
    	    			}
			index2chromNames.clear();
			}
   
		void open()
			{
			char folder[FILENAME_MAX];
			close();
			strncpy(folder,"_leveldbXXXXXX",FILENAME_MAX);
			if(::mkdtemp(folder)==NULL)
				{
				close();
				THROW("cannot generate temporary dir "<< folder << " " << strerror(errno));
				}
			
			this->db_home=new string(folder);
			clog << "Opening temporary folder: " << folder << endl; 
			leveldb::Options options;
			options.create_if_missing = true;
			options.error_if_exists= true;
			options.
			leveldb::Status status = leveldb::DB::Open(options,folder,&(this->db));
			if(!status.ok())
				{
				close();
				THROW("Cannot open leveldb file "<< folder <<".\n");
				}
			
			}
		~CompareBams()
			{
			close();
			}

		void load(unsigned short bamIndex)
			{
			size_t n_reads=0;
			std::string value;
			bam1_t *b= bam_init1();
			
			index2chromNames.resize(bamIndex+1);
			WHERE(in->header);
			for(int32_t i=0;i< in->header->n_targets;++i)
				{
				string target_name(in->header->target_name[i]);
				
				index2chromNames.back().push_back(target_name);
				}
			while(samread(this->in, b) > 0) /* loop over the records */
				{
				std::auto_ptr<vector<Hit> > hits(0);
				Bam1Sequence seq(b);
				leveldb::Slice key1(seq.name());
				value.clear();
				WHERE(n_reads);
				leveldb::Status status = db->Get(this->read_options, key1, &value);
				
				if(!status.ok())
					{
					hits.reset(new vector<Hit>());
					n_reads++;
					if(n_reads%1000000UL==0)
						{
						clog <<  n_reads << endl;
						//break;//TODO
						}
					}
				else
					{
					hits=decode(value);
					}
				
				Hit hit;
				hit.bamIndex=bamIndex;
				hit.tid = seq.tid();
				hit.pos = seq.pos();
				hit.flag = seq.flag();
				hits->push_back(hit);
				
				std::auto_ptr<string> encoded = this->encode(hits.get());
				leveldb::Slice value1(encoded->data(),encoded->size());
				status = db->Put(this->write_options, key1, value1);
				if(!status.ok())
					{
					cerr << "Cannot insert into db" << endl;
					break;
					}
				}
			bam_destroy1(b);
			
			}

	   	void usage(ostream& out,int argc,char **argv)
		    {
		    out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
		    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		    out << "Usage:\n\t"<< argv[0] << " [options] file1.bam file2.bam\n";
		    out << "Options:\n";
		    }
			
		bool sameChromosome(const char* s1,const char* s2)
			{
			if(strncmp(s1,"chr",3)==0) return sameChromosome(&s1[3],s2);
			if(strncmp(s2,"chr",3)==0) return sameChromosome(s1,&s2[3]);
			if(strcmp(s1,"MT")==0) return sameChromosome("M",s2);
			if(strcmp(s2,"MT")==0) return sameChromosome(s1,"M");
			return strcmp(s1,s2)==0;
			}
		void print(const Hit& hit)
			{
			if(!hit.mapped() || hit.tid==-1)
				{
				cout << "-";
				}
			else 
				{
				cout << index2chromNames[hit.bamIndex][hit.tid] << ":" << hit.pos;
				}
			}
		void dump()
			{
			
			leveldb::Iterator* it=db->NewIterator(this->read_options);
			for (it->SeekToFirst(); it->Valid(); it->Next())
				{
				std::auto_ptr<vector<Hit> > hits = decode(it->value().ToString());
				size_t i0=0;
				
				while(i0 < hits->size())
					{
					size_t i1=i0+1;
					while(i1<hits->size())
						{
						if(hits->at(i0).bamIndex==hits->at(i1).bamIndex) {++i1; continue;}
		
						if(IS_FLAG_SET(hits->at(i0).flag,BAM_FREAD1)!=IS_FLAG_SET(hits->at(i1).flag,BAM_FREAD1))
							{
							++i1;
							continue;
							}
						if(IS_FLAG_SET(hits->at(i0).flag,BAM_FREAD2)!=IS_FLAG_SET(hits->at(i1).flag,BAM_FREAD2))
							{
							++i1;
							continue;
							}
						if(!hits->at(i0).mapped() && !hits->at(i1).mapped())
							{
							break;
							}

						if(hits->at(i0).tid==-1 && hits->at(i1).tid==-1)
							{
							break;
							}
						else if(sameChromosome(
							index2chromNames[hits->at(i0).bamIndex][hits->at(i0).tid].c_str(),
							index2chromNames[hits->at(i1).bamIndex][hits->at(i1).tid].c_str()
							))
							{
							if(hits->at(i0).pos==hits->at(i1).pos)
								{
								break;
								}
							}
						
						++i1;
						}
					if(i1!=	hits->size())
						{
						cout 	<< it->key().ToString()
							<< "\t";
						if(IS_FLAG_SET(hits->at(i0).flag,BAM_FREAD1)) { cout << "1\t"; }
						else if(IS_FLAG_SET(hits->at(i0).flag,BAM_FREAD2)) { cout << "2\t";}
						else cout << "?\t";
						print(hits->at(i0));
						cout << "\t";
						print(hits->at(i1));				
						cout << "\t=";
						cout << endl;
						hits->erase(hits->begin()+i1);//that one before
						hits->erase(hits->begin()+i0);
						}
					else
						{
						++i0;
						}
					}
				
				for(i0=0;i0<hits->size();++i0)
					{
					cout 	<< it->key().ToString()
						<< "\t";
					if(IS_FLAG_SET(hits->at(i0).flag,BAM_FREAD1)) cout << "1\t";
					else if(IS_FLAG_SET(hits->at(i0).flag,BAM_FREAD2)) cout << "2\t";
					else cout << "?\t";
					if(hits->at(i0).bamIndex==1) cout << ".\t";	
					print(hits->at(i0));
					if(hits->at(i0).bamIndex==0) cout << "\t.";		
					cout << endl;
					}

				
				}
			delete it;
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
				else if(strcmp(argv[optind],"--")==0)
				        {
				        ++optind;
				        break;
				        }
				else if(argv[optind][0]=='-')
				        {
				        cerr << "unknown option '" << argv[optind]<< endl;
				        usage(cerr,argc,argv);
				        return(EXIT_FAILURE);
				        }
				else
				        {
				        break;
				        }
				++optind;
				}
		        
		        if(optind+2!=argc)
				{
				cerr<<  "Illegal number of arguments"<< endl;
				return EXIT_FAILURE;
				}
			
			this->open();
			unsigned char index=0;
			while(optind+index<argc)
				{
				this->in = samopen(argv[optind+index], "rb", 0);
				if (this->in == 0)
					{
					cerr<<  "Cannot open BAM file \""<< argv[optind] << "\". "<< strerror(errno) << endl;
					this->close();
					return EXIT_FAILURE;
					}

				this->load(index);
				++index;
				::samclose(this->in);
				this->in=0;
				}
			index=0;
			cout << "#READ\tSIDE";
			while(optind+index<argc)
				{
				cout << "\t"<< argv[optind+index];
				++index;
				}
			cout << endl;
			this->dump();
			
			return EXIT_SUCCESS;
			}
			
	};

int main(int argc,char** argv)
	{
	CompareBams app;
	return app.main(argc,argv);
	}
