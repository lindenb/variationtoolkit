#include <cstring>
#include "numeric_cast.h"
#include "vcfreader.h"
#include "throw.h"
#define NOWHERE
#include "where.h"
using namespace std;

VCFReader::AbstractMeta::AbstractMeta()
    {
    }
VCFReader::AbstractMeta::AbstractMeta(const VCFReader::AbstractMeta& cp):id(cp.id),desc(cp.desc)
    {
    }
VCFReader::AbstractMeta::~AbstractMeta()
    {
    }

/*************************************************************************/
/*************************************************************************/
/*************************************************************************/


VCFReader::MetaFilter::MetaFilter()
    {
    }

VCFReader::MetaFilter::MetaFilter(const VCFReader::MetaFilter& cp):AbstractMeta(cp)
    {
    }

VCFReader::MetaFilter::~MetaFilter()
    {
    }

VCFReader::MetaFilter&
VCFReader::MetaFilter::operator=(const VCFReader::MetaFilter& cp)
    {
    if(this!=&cp)
	{
	this->id.assign(cp.id);
	this->desc.assign(cp.desc);
	}
    return *this;
    }

bool
VCFReader::MetaFilter::operator==(const VCFReader::MetaFilter& cp) const
    {
    return id.compare(cp.id)==0;
    }

bool
VCFReader::MetaFilter::operator<(const VCFReader::MetaFilter& cp) const
    {
    return id.compare(cp.id)<0;
    }

/*************************************************************************/
/*************************************************************************/
/*************************************************************************/


VCFReader::Meta::Meta()
    {
    }

VCFReader::Meta::Meta(const VCFReader::Meta& cp):VCFReader::AbstractMeta(cp),
	count(cp.count),type(cp.type)
    {
    }

VCFReader::Meta::~Meta()
    {
    }

VCFReader::Meta&
VCFReader::Meta::operator=(const VCFReader::Meta& cp)
    {
    if(this!=&cp)
	{
	this->id.assign(cp.id);
	this->count.assign(cp.count);
	this->type.assign(cp.type);
	this->desc.assign(cp.desc);
	}
    return *this;
    }

bool
VCFReader::Meta::operator==(const VCFReader::Meta& cp) const
    {
    return id.compare(cp.id)==0;
    }

bool
VCFReader::Meta::operator<(const VCFReader::Meta& cp) const
    {
    return id.compare(cp.id)<0;
    }
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

set<string> VCFReader::Row::formats() const
    {
    set<string> fmt;
    for(vector<Call>::const_iterator r=calls.begin();r!=calls.end();++r)
	{
	set<string> fmt2=r->formats();
	fmt.insert(fmt2.begin(),fmt2.end());
	}
    return fmt;
    }

std::set<std::string> VCFReader::Row::info_keys() const
    {
    set<string> s;
    for(map<string,string>::const_iterator r=info.begin();r!=info.end();++r)
   	{
   	s.insert(r->first);
   	}
    return s;
    }

/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

set<string> VCFReader::Call::formats() const
    {
    set<string> fmt;
    for(map<string,string>::const_iterator r=call.begin();r!=call.end();++r)
	{
	fmt.insert(r->first);
	}
    return fmt;
    }

/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

VCFReader::VCFReader(std::istream& in):in(in),
	tab('\t'),colon(':'),semicolon(';'),
	_header(0),nLine(0)
    {
    }
VCFReader::~VCFReader()
    {
    if(_header!=0) delete _header;
    }

std::string VCFReader::parse_meta(const char* key,const std::string& line) const
    {
    std::string v;
    std::string::size_type i=line.find(key,0);
    if(i==string::npos) return v;
    i+=strlen(key);
    std::string::size_type j=line.find(",",i);
    if(j==string::npos)
	{
	v.assign(line.substr(i));
	}
    else
	{
	v.assign(line.substr(i,j-i));
	}
    if(v.size()>0 && v[0]=='"')
	{
	v=v.substr(1);
	}
    if(v.size()>0 && v[v.size()-1]=='\"')
	{
	v=v.substr(0,v.size()-1);
	}
    return v;
    }


VCFReader::Meta VCFReader::parse_meta(std::string line) const
    {
    Meta meta;
    meta.id=parse_meta("ID=",line);
    meta.count=parse_meta("Number=",line);
    meta.type=parse_meta("Type=",line);
    meta.desc=parse_meta("Description=",line);
    return meta;
    }

VCFReader::MetaFilter VCFReader::parse_filter(std::string line) const
    {
    MetaFilter f;
    f.id=parse_meta("ID=",line);
    f.desc=parse_meta("Description=",line);
    return f;
    }

const VCFReader::Header* VCFReader::header()
    {
    if(_header==0)
	{
	_header=new Header;
	std::string line;
	std::vector<std::string> tokens;
	 while(getline(in,line,'\n'))
	    {
	     ++nLine;
	     WHERE(line);
	    if(line.empty()) continue;
	    WHERE(line);
	    if( line.size()>1 &&
		line[0]=='#' &&
		line[1]=='#')
		{
		WHERE(line);
		if(line.size()==2) continue;
		if(line.size()>10 && line.compare(0,10,"##FORMAT=<")==0 && line.at(line.size()-1)=='>')
		    {
		    MetaFormat meta= parse_meta(line.substr(10,line.size()-11));
		    _header->format.insert(std::make_pair<std::string,MetaFormat>(meta.id,meta));
		    }
		else if(line.size()>10 && line.compare(0,8,"##INFO=<")==0 && line.at(line.size()-1)=='>')
		    {
		    MetaInfo meta= parse_meta(line.substr(8,line.size()-9));
		    _header->info.insert(std::make_pair<std::string,MetaInfo>(meta.id,meta));
		    }
		else if(line.size()>10 && line.compare(0,10,"##FILTER=<")==0 && line.at(line.size()-1)=='>')
		    {
		    MetaFilter meta= parse_filter(line.substr(8,line.size()-9));
		    _header->filters.insert(std::make_pair<std::string,MetaFilter>(meta.id,meta));
		    }
		else
		    {
		    cerr << line << endl;
		    _header->meta.push_back(line.substr(2));
		    }
		}
	    else if(line.size()>0 &&
		    line[0]=='#'
		    )
		{
		WHERE(line);
		tab.split(line,tokens);
		if(tokens.size()<10) THROW("Expected at least 10 columns in " << line);
		if(tokens[0].compare("#CHROM")!=0) THROW("Expected '#CHROM' in column 1 of " << line);
		if(tokens[1].compare("POS")!=0) THROW("Expected 'POS' in column 2 of " << line);
		if(tokens[2].compare("ID")!=0) THROW("Expected 'ID' in column 3 of " << line);
		if(tokens[3].compare("REF")!=0) THROW("Expected 'REF' in column 4 of " << line);
		if(tokens[4].compare("ALT")!=0) THROW("Expected 'ALT' in column 5 of " << line);
		if(tokens[5].compare("QUAL")!=0) THROW("Expected 'QUAL' in column 6 of " << line);
		if(tokens[6].compare("FILTER")!=0) THROW("Expected 'FILTER' in column 7 of " << line);
		if(tokens[7].compare("INFO")!=0) THROW("Expected 'INFO' in column 8 of " << line);
		if(tokens[8].compare("FORMAT")!=0) THROW("Expected 'FORMAT' in column 9 of " << line);

		for(size_t i=9;i< tokens.size();++i)
		    {
		    _header->samples.push_back(tokens[i]);
		    }

		break;//end of read header
		}
	    else
		{
		WHERE(line);
		THROW("Boum "<< line);
		}
	    }
	}
	return _header;
	}

std::auto_ptr<VCFReader::Row> VCFReader::next()
    {
    if(_header==0)
	{
	header();
	}
    std::string line;
    for(;;)
	{
	if(!getline(in,line,'\n')) return std::auto_ptr<Row>(0);
	++nLine;
	WHERE(line);
	if(line.empty()) continue;
	break;
	}
    vector<string> tokens;
    Row* row=new Row;
    row->nLine=nLine;
    tab.split(line,tokens);
    if(tokens.size()!=9+_header->samples.size())
	{
	THROW("Expected " << (9+_header->samples.size()) << " in "<< line);
	}
    row->chrom.assign(tokens[0]);
    if(!numeric_cast<int32_t>(tokens[1].c_str(),&(row->pos)))
	{
	THROW("Bad POS in "<< line);
	}
    row->id.assign(tokens[2]);
    row->ref.assign(tokens[3]);
    row->alt.assign(tokens[4]);
    row->qual.assign(tokens[5]);


    if(tokens[6].compare(".")!=0 && !tokens[6].empty())
	{
	vector<string> filters;
	this->semicolon.split(tokens[6],filters);
	for(size_t j=0;j< filters.size();++j)
	    {
	    if(filters[j].empty()) continue;
	    row->filter.insert(filters[j]);
	    }
	}


	{
	vector<string> infos;
	this->semicolon.split(tokens[7],infos);
	for(size_t j=0;j< infos.size();++j)
	    {
	    if(infos[j].empty()) continue;
	    string::size_type eq=infos[j].find_first_of('=');


	    if(eq==std::string::npos)
		{
		row->info.insert(make_pair<string,string>(infos[j],""));
		}
	    else
		{
		string k=infos[j].substr(0,eq);
		string v=infos[j].substr(eq+1);
		row->info.insert(make_pair<string,string>(k,v));
		}
	    }
	}

	{
	vector<string> format;

	this->colon.split(tokens[8],format);
	for(size_t j=0;j< _header->samples.size();++j)
	    {
	    vector<string> call;
	    this->colon.split(tokens[9+j],call);
	    if(call.size()!=format.size())
		{
		THROW("size(FORMAT)!=size(call) in "<< line);
		}
	    Call c;
	    c.sample=_header->samples[j];

	    for(size_t k=0;k< format.size();++k)
		{
		c.call.insert(std::make_pair<string,string>(format[k],call[k]));
		}
	    row->calls.push_back(c);
	    }
	}
    return auto_ptr<Row>(row);
    }


std::ostream& operator << (std::ostream& out,const VCFReader::MetaFilter& o)
    {
    return out;
    }
std::ostream& operator << (std::ostream& out,const VCFReader::Meta& o)
    {
    return out;
    }
std::ostream& operator << (std::ostream& out,const VCFReader::Row& o)
    {
    out << o.chrom
	    << "\t" << o.pos
	    << "\t" << o.id
	    << "\t" << o.ref
	    << "\t" << o.alt
	    << "\t" << o.qual
	    << "\t";
    for(set<string>::const_iterator r=o.filter.begin();
	    r!=o.filter.end();
	    ++r)
	{
	if(r!=o.filter.begin())
	    {
	    out << ";";
	    }
	 out << (*r);
	}
    out << "\t";
    for(multimap<string,string>::const_iterator r=o.info.begin();
    	    r!=o.info.end();
    	    ++r)
    	{
    	if(r!=o.info.begin())
    	    {
    	    out << ";";
    	    }
    	 out <<  r->first;
	 if(!r->second.empty())
	     {
	     out << "=" << r->second;
	     }
    	}
    out << "\t";
    set<string> fmt=o.formats();
    for(set<string>::const_iterator r=fmt.begin();
	    r!=fmt.end();
	    ++r)
	{
	if(r!=fmt.begin())
	    {
	    out << ":";
	    }
	out << (*r);
	}
    for(size_t i=0;i< o.calls.size();++i)
	{
	out << "\t";
	 for(set<string>::const_iterator r=fmt.begin();
		    r!=fmt.end();
		    ++r)
	     {
	     if(r!=fmt.begin())
		{
		out << ":";
		}
	     map<string,string>::const_iterator r2=o.calls[i].call.find(*r);
	     if(r2!=o.calls[i].call.end())
		 {
		 out << r2->second;
		 }
	     }
	}
    return out;
    }

#ifdef TEST_THIS_CODE

int main(int argc,char** argv)
    {
    VCFReader in(cin);
    for(;;)
	{
	auto_ptr<VCFReader::Row> r=in.next();
	if(r.get()==0) break;
	cout << (*r) << endl;
	}
    return 0;
    }

#endif
