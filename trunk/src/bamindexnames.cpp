
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
#include <cassert>
#include <algorithm>

//#define NOWHERE
#include "where.h"
#include "bam.h"
#include "bam1sequence.h"
#include "throw.h"
#include "xstdio.h"

using namespace std;

typedef int64_t bgzf_filepos_t;    

#define DEFAULT_FOLDER_EXTENSION ".names.idx"
#define CONFIG_EXTENSION ".cfg"

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


struct NameIndexHeader
	{
	uint64_t name_size;
	uint16_t max_hits;
	long count_items;
	};
	
	
ostream& operator<<(ostream& out,const NameIndexHeader& o)
	{
	out 	<< "max-name:"<< o.name_size
		<< " max-hits:" << o.max_hits
		<< " count-items:" << o.count_items
		;
	return out;
	}

class NameOffsets
	{
	public:
		std::string name;
		vector<bgzf_filepos_t> offsets;
		NameOffsets() {}
		NameOffsets(const string& s,bgzf_filepos_t offset):name(s) {offsets.push_back(offset);}
		NameOffsets(const char* s,bgzf_filepos_t offset):name(s){offsets.push_back(offset);}
		NameOffsets(const NameOffsets& cp):name(cp.name),offsets(cp.offsets.begin(),cp.offsets.end())
			{
			}
		~NameOffsets() {}
		NameOffsets& operator=(const NameOffsets& cp)
			{
			if(this!=&cp)
				{
				name=cp.name;
				offsets=cp.offsets;
				}
			return *this;
			}
		bool operator==(const NameOffsets& cp) const
			{
			return name==cp.name;
			}
		bool operator<(const NameOffsets& cp) const
			{
			return name<cp.name;
			}
		
		static bool comparator(const NameOffsets* o1,const NameOffsets* o2)
			{
			return o1->name < o2->name;
			}
		
		static auto_ptr<NameOffsets> readOne(FILE* in,const NameIndexHeader* config)
			{
			auto_ptr<NameOffsets> record(new NameOffsets());
			record->name.resize(config->name_size,'\0' );
			safeFRead((void*)record->name.data(),sizeof(char),config->name_size,in);
			assert(record->name.size()<=config->name_size);
			for(size_t i=0; i< record->name.size();++i)
				{
				if(record->name[i]=='\0')
					{
					record->name.resize(i);
					break;
					}
				}
			uint16_t n_hits=0;
			safeFRead((void*)&(n_hits),sizeof(uint16_t),1,in);
			
			assert( n_hits<= config->max_hits );
			for(uint16_t i=0;i< n_hits;++i)
				{
				bgzf_filepos_t pos;
				safeFRead((void*)&(pos),sizeof(bgzf_filepos_t),1,in);
				record->offsets.push_back(pos);
				}
			for(uint16_t i=n_hits;i< config->max_hits ;++i)
				{
				bgzf_filepos_t pos;
				safeFRead((void*)&(pos),sizeof(bgzf_filepos_t),1,in);
				//do nothing
				}
			
			return record;
			}

		void writeOne(FILE* out,const NameIndexHeader* config) const
			{
			const char zero('\0');
			
			assert(config->name_size>= this->name.size());
			safeFWrite((void*)this->name.data(),sizeof(char),this->name.size(),out);
			//fill with zero
			for(size_t i= this->name.size(); i< config->name_size;++i)
				{
				safeFWrite((void*)&zero,sizeof(char),1,out);
				}
			uint16_t n_hits = (uint16_t) this->offsets.size();
			safeFWrite((void*)&n_hits,sizeof(uint16_t),1,out);
			assert(config->max_hits >= this->offsets.size());
			for(size_t i=0;i< this->offsets.size();++i)
				{
				bgzf_filepos_t offset=this->offsets.at(i);
				safeFWrite((void*)&offset,sizeof(bgzf_filepos_t),1,out);
				}
			
			for(size_t i= this->offsets.size();i< config->max_hits; ++i)
				{
				bgzf_filepos_t none(-1);
				safeFWrite((void*)&none,sizeof(bgzf_filepos_t),1,out);
				}
			}


	};

ostream& operator<<(ostream& out,const NameOffsets& o)
	{
	out 	<< "["<< o.name << " : ";
	for(size_t i=0;i< o.offsets.size();++i)
		{
		if(i!=0) out << ",";
		out << o.offsets[i];
		}
	out << "]";
	return out;
	}


class TmpFile
	{
	public:
		/* temporary file */
		FILE* tmp;
		/* configuration */
		NameIndexHeader config;
		long index_read;
		TmpFile()
			{
			tmp=safeTmpFile();
			memset((void*)&config,0,sizeof(NameIndexHeader));
			index_read=0;
			}
		
		~TmpFile()
			{
			fclose(tmp);
			}
		
		auto_ptr<NameOffsets> readOne()
			{
			assert(index_read<config.count_items);
			index_read++;
			
			return NameOffsets::readOne(tmp,&config);
			}
		void writeOne(const NameOffsets* record)
			{
			record->writeOne(tmp,&config);
			config.count_items++;
			}
		void rewind()
			{
			safeFSeek(this->tmp,0L,SEEK_SET);
			index_read=0;
			}
	};


class AbstractIndexOfNames
	{
	public:
		  /** input BAM */
        	bamFile in;
        	bam_header_t* bam_header;
		std::string* db_home;
		
		
	protected:
		AbstractIndexOfNames():in(0),bam_header(0),db_home(0)
			{
			
			}
		auto_ptr<NameOffsets> readOne(FILE* in,const NameIndexHeader* config)
			{
			return NameOffsets::readOne(in,config);
			}
		
	public:
		
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
		
		virtual void open(const char* bamfile,const char* user_db_home)
			{
			this->in=bam_open(bamfile,"r");
			if(this->in==0)
				{
				THROW("Cannot open BAM \""<< bamfile <<"\".\n" << strerror(errno));
				}
			this->bam_header=bam_header_read(this->in);
			if(user_db_home==0)
				{
				this->db_home=new string(bamfile);
				this->db_home->append(DEFAULT_FOLDER_EXTENSION);
				}
			else
				{
				this->db_home=new string(user_db_home);
				}
			}
		
		virtual void close()
			{
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
			}
			
		virtual ~AbstractIndexOfNames()	
			{
			close();
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
		std::size_t buffer_capacity;
		ShortReadNameChanger* nameChanger;
		WriteIndexOfNames():buffer_capacity(1000000),nameChanger(0)
			{
			}
			
		virtual ~WriteIndexOfNames()
			{
			}
		
	private:
		
		
		
		
	public:
		void build_index()
		    {
		    //clock_t start_time = clock( );
		    
		    
		    bam1_t *b= bam_init1();
		    
    		    TmpFile* next_file=0;
    		    TmpFile* prev_file=0;
    		   
		    
    		    vector<NameOffsets*> buffer;
		    buffer.reserve(buffer_capacity);
		    bool eof_met=false;
		    auto_ptr<NameOffsets> fileItem(0);
		    

		   while(!eof_met)
		        {
		        /* clear the buffer */
		        for(size_t i=0;i<buffer.size();++i) delete buffer[i];
			buffer.clear();
		       
		       
		       assert(next_file==0);
		       		
			next_file=new TmpFile;
			if(prev_file!=0)
				{
				next_file->config.max_hits = prev_file->config.max_hits;
				next_file->config.name_size = prev_file->config.name_size;
				}
		       		
		        WHERE("Loading buffer");
		        while(!eof_met && buffer.size()<buffer_capacity)
				{
				/* get current offset in BGZF */
				bgzf_filepos_t offset=bam_tell(this->in);
				/* read the next BAM record */
				int n_read=bam_read1(this->in, b);
				/* eof met ?*/
				if(n_read<0)
					{
					eof_met=true;
					break;
					}
				
				Bam1Sequence seq(b);
				std::string shortReadName(bam1_qname(b));
				if(nameChanger!=0)
					{
					/* change the name if needed */
					nameChanger->change(shortReadName);
					}
				if(shortReadName.empty()) continue;
				
				NameOffsets* record =new NameOffsets(shortReadName,offset);
				buffer.push_back(record);
				next_file->config.name_size = std::max(next_file->config.name_size,record->name.size());
				}
			WHERE("sorting");
			sort(buffer.begin(),buffer.end(),NameOffsets::comparator);
			WHERE("merging:"<< buffer.size());
			
			long x=(long)buffer.size()-1;
			while(x>=0)
				{
				if(x+1 < (long)buffer.size() && buffer[x]->name.compare(buffer[x+1]->name)==0)
					{
					buffer[x]->offsets.insert(
							buffer[x]->offsets.end(),
							buffer[x+1]->offsets.begin(),
							buffer[x+1]->offsets.end()
							);
					delete buffer[x+1];
					buffer.erase(buffer.begin()+(x+1));
					}
				else
					{
					--x;
					}
				
				}
				
			WHERE("merged:"<< buffer.size());
			for(x=0;x< (long)buffer.size();++x)
				{
				if(buffer[x]->offsets.size() > next_file->config.max_hits)
					{
					next_file->config.max_hits= (uint16_t)(buffer[x]->offsets.size());
					WHERE("#### CONFIG CHANGED ######################");
					}
				}
			
				
			//WHERE("sort indexHeader.name_size:"+indexHeader.name_size<<" size:"<<buffer.size() << " max-len-name:"<<indexHeader.name_size );
			//sort(buffer.begin(),buffer.end());
		        
		        if(prev_file==0)
				{
				WHERE("################ JOIN INIT");
				/* first pass, dump the buffer in TMP */
				for(size_t i=0;i< buffer.size();++i)
					{
					next_file->writeOne(buffer[i]);
					}
				assert(next_file->config.count_items==(long)buffer.size());
				safeFFlush(next_file->tmp);
				prev_file=next_file;
				next_file=0;
				}
			else
				{
				WHERE("################ JOIN prev:" << prev_file->config<< " " << next_file->config);
				//std::cerr << "merge sort *********************" <<  std::endl;
				
				vector<NameOffsets*>::iterator iter_buffer=buffer.begin();
				
				fileItem.reset(0);
				//std::cerr << "items_on_file " << items_on_file <<  std::endl;
				//rewind prev_file
				prev_file->rewind();
				
				while(  iter_buffer != buffer.end() &&
					prev_file->index_read < prev_file->config.count_items
					)
					{		
					if(fileItem.get()==0)
						{
						fileItem = prev_file->readOne();
						}
					
					/* same name */
					if(**iter_buffer == *(fileItem))
						{
						/* put array offset in file-item */
						fileItem->offsets.insert(
							fileItem->offsets.end(),
							(*iter_buffer)->offsets.begin(),
							(*iter_buffer)->offsets.end()
							);
						sort(fileItem->offsets.begin(),fileItem->offsets.end());
						
						if(fileItem->offsets.size() > next_file->config.max_hits )
							{
							/* this part has never been tested */
							assert(0);
							
							next_file->rewind();
							TmpFile* copy=new TmpFile;
							memcpy((void*)&(copy->config),(void*)&(next_file->config),sizeof(NameIndexHeader));
							copy->config.max_hits = fileItem->offsets.size();
							copy->config.count_items = 0;
							
							for(long j=0;j< next_file->config.count_items ;++j)
								{
								auto_ptr<NameOffsets> olditem =next_file->readOne();
								copy->writeOne(olditem.get());	
								}
							delete next_file;
							next_file=copy;
							}
						
						next_file->writeOne(fileItem.get());
						fileItem.reset(0);
						++iter_buffer;
						}	
					else if(**iter_buffer < *(fileItem))
						{
						next_file->writeOne(*iter_buffer);
						++iter_buffer;
						}
					else
						{
						next_file->writeOne(fileItem.get());
						fileItem.reset(0);
						}
					}
				
				while(iter_buffer != buffer.end())
					{
					
					next_file->writeOne(*iter_buffer);
					++iter_buffer;
					}
				
				//last file item was not saved?
				if(fileItem.get()!=0)
					{

					next_file->writeOne(fileItem.get());
					fileItem.reset(0);
					}
				if(prev_file->index_read < prev_file->config.count_items)
					{
					while(prev_file->index_read < prev_file->config.count_items)
						{
						fileItem=prev_file->readOne();
						next_file->writeOne(fileItem.get());
						}
					fileItem.reset(0);
					}
				//next_file->config.count_items=prev_file->config.count_items+(long)buffer.size();
				delete prev_file;
				prev_file=next_file;
				next_file=0;
				for(size_t i=0;i< buffer.size();++i) delete buffer[i];
				buffer.clear();
				}
			//prev_max_hits=indexHeader.max_hits;
			
			//if(prev_file!=0 && prev_file->config.count_items> 1E6) break;//TODO
			
		   	}
		   	 bam_destroy1(b);
			
		    
			
			
			if(prev_file!=NULL)
				{
				WHERE("we're done saving to disk " << prev_file->config);
				FILE* saveAs=safeFOpen(this->db_home->c_str(),"wb");
				prev_file->rewind();
				for(long i=0;i < prev_file->config.count_items;++i)
					{
					fileItem =prev_file->readOne();
					fileItem.get()->writeOne(saveAs,&prev_file->config);
					}
				
				safeFFlush(saveAs);
				fclose(saveAs);
			
			
				string idx_name=(*(this->db_home));
				idx_name.append(CONFIG_EXTENSION);
				saveAs=safeFOpen(idx_name.c_str(),"wb");
				safeFWrite((void*)&prev_file->config,sizeof(NameIndexHeader),1,saveAs);
				safeFFlush(saveAs);
				fclose(saveAs);
				std::cerr << "end of merge" << std::endl;
			
				delete prev_file;
				}
			
			
		
		    }
		
		
	};


/**
 *
 * ReadIndexOfNames
 * 
 */
class ReadIndexOfNames:public AbstractIndexOfNames
	{	
	public:
		FILE* stream;
		NameIndexHeader config;
		
		ReadIndexOfNames():stream(0)
			{
			
			}
			
		virtual ~ReadIndexOfNames()
			{
			}
		
		size_t sizeOf() const
			{
			return config.name_size+sizeof(uint16_t)+config.max_hits*sizeof(bgzf_filepos_t);
			}
		
		virtual auto_ptr<NameOffsets> readOne()
			{
			return AbstractIndexOfNames::readOne(this->stream,&config);
			}
		
		virtual auto_ptr<NameOffsets> get(long index)
			{			
			safeFSeek(this->stream, sizeOf()*index, SEEK_SET);
			return readOne();
			}
		
		
		
		virtual long lower_bound(const char* seq)
		    {
		    auto_ptr<NameOffsets> position(0);
		    long beg=0L;
		    long end= config.count_items;
		    long len= end-beg;
		    while(len>0)
			{
			long half = len/2;
			long middle=beg+half;
			position=get(middle);

			if(position->name.compare(seq)<0)
			    {
			    beg = middle;
			    ++beg;
			    len = len - half - 1;
			    }
			 else
			    {
			    len = half;
			    }
			}
		    return beg;
		    }		
			
		
		virtual auto_ptr<NameOffsets> getByName(const char* name)
			{
			auto_ptr<NameOffsets> ret(0);
			long i= lower_bound(name);
			if(i< config.count_items)
				{
				ret= get(i);
				if(ret.get()==0 || ret->name.compare(name)!=0)
					{
					ret.reset(0);
					} 
				}
			return ret;
			}
		
		virtual void open(const char* bamfile,const char* user_db_home)
		 	{
		 	AbstractIndexOfNames::open(bamfile,user_db_home);
		 	
		 	string idx_name=(*(this->db_home));
			idx_name.append(CONFIG_EXTENSION);
			WHERE("opening " << idx_name);
			memset((void*)&config,0,sizeof(NameIndexHeader)) ;
			FILE* in=safeFOpen(idx_name.c_str(),"rb");
			safeFRead((void*)&config,sizeof(NameIndexHeader),1,in);
			fclose(in);
			WHERE(config);
			assert(config.name_size<100);
			this->stream=safeFOpen(this->db_home->c_str(),"rb");
			}
		
		virtual void close()
			{
			AbstractIndexOfNames::close();
			if(this->stream!=0)
				{
				fclose(this->stream);
				this->stream=0;
				}
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
		auto_ptr<set<ComparableBam1Record> > getBamRecordsAt(const char* sequence)
			{
			vector<bgzf_filepos_t> offsets;
			long index=lower_bound(sequence);
			if(index < config.count_items)
				{
				safeFSeek(this->stream, sizeOf()*index, SEEK_SET);
				
				auto_ptr<NameOffsets> nameoffset = readOne();
				if(nameoffset->name.compare(sequence)>0)
					{
					//rien
					}
				else if(nameoffset->name.compare(sequence)<0)
					{
					THROW("UHH?");
					}
				else
					{
					offsets.insert(
						offsets.end(),
						nameoffset->offsets.begin(),
						nameoffset->offsets.end()
						);
					}
					
				}
			return getBamRecordsAt(offsets);
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
        	

		for(long index=0; index < indexOfNames[0].config.count_items;++index)
		 	{
		 	auto_ptr<NameOffsets> curr0= indexOfNames[0].readOne();
		 	auto_ptr<NameOffsets> curr1= indexOfNames[1].getByName(curr0->name.c_str());
		 		
        		
        		
        		vector<bgzf_filepos_t> hits0(curr0->offsets.begin(),curr0->offsets.end());
		 	vector<bgzf_filepos_t> hits1;
		 	if(curr1.get()!=0)
		 		{
		 		hits1.insert(hits1.end(),curr1->offsets.begin(),curr1->offsets.end());
		 		}

		 	auto_ptr<set<ComparableBam1Record> > recs0=indexOfNames[0].getBamRecordsAt(hits0);
		 	auto_ptr<set<ComparableBam1Record> > recs1=indexOfNames[1].getBamRecordsAt(hits1);
		  	
		  	
		  	
		  	
		  	
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
	    				cout << curr0->name << "\tONLY_1\t";
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
	    				cout << curr0->name << "\tONLY_2\t";
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
	    				cout << curr0->name << "\tBOTH\t";
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
		
		
        	if(print_only_in_1)
			{
			safeFSeek(indexOfNames[1].stream,0L,SEEK_SET);
			for(long index=0; index < indexOfNames[1].config.count_items;++index)
			 	{
			 	auto_ptr<NameOffsets> curr1= indexOfNames[1].readOne();
		 		auto_ptr<NameOffsets> curr0= indexOfNames[0].getByName(curr1->name.c_str());
				 if(curr0.get()!=0) continue;//already processed
				 
				
				 auto_ptr<set<ComparableBam1Record> > recs1=indexOfNames[1].getBamRecordsAt(curr0->offsets);
				 if(!recs1->empty())
	    				{
	    				cout << curr1->name << "\tONLY_2\t";
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
        			continue;
        			}
        		std::string line;
        		while(getline(in,line,'\n'))
        			{
        			if(line.empty() || line[0]=='#') continue;
        			
				auto_ptr< NameOffsets > ret=indexOfNames.getByName(line.c_str());
        			
        			
        			
		        	if(ret.get()==0)
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
	            			for(size_t j=0;j< ret->offsets.size();++j)
	            				{
	            				bgzf_filepos_t offset=ret->offsets.at(j);
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
        	long index_file=0;
        	if(nameBegin!=0)
        		{
        		index_file=indexOfNames.lower_bound(nameBegin);
        		if(index_file< indexOfNames.config.count_items)
        			{
        			safeFSeek(indexOfNames.stream, indexOfNames.sizeOf()*index_file, SEEK_SET);
        			}
        		
        		}
        	
        	for(;index_file < indexOfNames.config.count_items; ++index_file)
        		{
        		auto_ptr< NameOffsets > ret=indexOfNames.readOne();
        		if(ret.get()==0) { WHERE("");break;}
        		if(nameEnd!=0)
		 	 	{
		 	 	if(ret->name.compare(nameEnd)>0) break;
		 	 	}
        		cout << ret->name << "\t" << ret->offsets.size() << endl;
        		++index_file;
        		}
        	
        	
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
