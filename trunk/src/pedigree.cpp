#include <cassert>
#include "zstreambuf.h"
#include "throw.h"
#include "tokenizer.h"
#include "pedigree.h"

using namespace std;


/**
 * Individual
 */

Individual::Individual():_family(NULL),_father(NULL),_mother(NULL),_sex(Individual::unknown),_index(-1)
    {

    }

Individual::~Individual()
    {
    }

int Individual::index() const
    {
    return _index;
    }

const char*  Individual::name() const
   {
   return _id.c_str();
   }

const Family* Individual::family() const
    {
    return _family;
    }

const Population* Individual::population() const
    {
    return family()->population();
    }


const Individual* Individual::parent(int index) const
    {
    assert(index==0 || index==1);
    return (index==0?father():mother());
    }



const Individual* Individual::father() const
    {
    return _father;
    }

const Individual* Individual::mother() const
    {
    return _mother;
    }

bool Individual::hasFather() const
   {
   return father()!=NULL;
   }

bool Individual::hasMother() const
   {
   return mother()!=NULL;
   }

bool Individual::hasParent() const
   {
   return hasFather() || hasMother();
   }

bool Individual::hasParent(const Individual* parent) const
   {
   return parent!=NULL && (father()==parent || mother()==parent);
   }

bool Individual::isFatherOf(const Individual* child) const
   {
   return child!=NULL && child->father()==this;
   }

bool Individual::isMotherOf(const Individual* child) const
   {
   return child!=NULL && child->mother()==this;
   }

bool Individual::isParentOf(const Individual* child) const
   {
   return isFatherOf(child) || isMotherOf(child);
   }

Individual::Sex Individual::sex() const
   {
   return _sex;
   }


std::ostream& operator << (std::ostream& out,const Individual& o)
    {
    out << o.family()->name() << "\t" << o.name() << "\t";
    out << (o.father()==NULL?"0":o.father()->name());
    out << "\t";
    out << (o.mother()==NULL?"0":o.mother()->name());
    out << "\t" << o.sex();
    return out;

    }



Family::Family()
    {
    }


Family::~Family()
    {
    while(!individuals.empty())
	{
	delete individuals.back();
	individuals.pop_back();
	}
    }


const char* Family::name() const
   {
   return _id.c_str();
   }

const Population* Family::population() const
    {
    return _population;
    }


Family::size_type Family::size() const
    {
    return (size_type)individuals.size();
    }

const IndividualPtr Family::at(size_type i) const
    {
    return individuals.at((std::vector<IndividualPtr>::size_type)i);
    }

const IndividualPtr Family::operator[](size_type i) const
    {
    return at(i);
    }

const IndividualPtr Family::findByName(const char* s) const
   {
    for(Family::size_type i=0;i< size();++i)
	{
	const IndividualPtr indi=at(i);
	if(std::strcmp(indi->name(),s)==0) return indi;
	}
   return NULL;
   }


std::ostream& operator << (std::ostream& out,const Family& o)
    {
    for(Family::size_type i=0;i< o.size();++i)
    	{
    	out << *(o.at(i)) << std::endl;
    	}
    return out;
    }


Population::Population()
	{
	}

Population::~Population()
    {
    while(!families.empty())
	{
	delete families.back();
	families.pop_back();
	}
    }


Population::size_type Population::size() const
    {
    return (size_type)families.size();
    }

const FamilyPtr Population::at(size_type i) const
    {
    return families.at((std::vector<FamilyPtr>::size_type)i);
    }

const FamilyPtr Population::operator[](size_type i) const
    {
    return at(i);
    }

const FamilyPtr Population::findByName(const char* s) const
   {
    for(Family::size_type i=0;i< size();++i)
	{
	const FamilyPtr f=at(i);
	if(std::strcmp(f->name(),s)==0) return f;
	}
   return NULL;
   }



std::ostream& operator << (std::ostream& out,const Population& o)
    {
    for(Population::size_type i=0;i< o.size();++i)
	{
	out << *(o.at(i));
	}
    return out;
    }


auto_ptr<Population> PopulationReader::parse(const char* filename)
    {
    std::vector<std::pair<IndividualPtr,std::string> > fathers;
    std::vector<std::pair<IndividualPtr,std::string> > mothers;
    igzstreambuf buff(filename);
    std::iostream in(&buff);
    std::string line;
    int nLine=-1;
    const uint32_t familyColumn=0;
    const uint32_t nameColumn=1;
    const uint32_t fatherColumn=2;
    const uint32_t motherColumn=3;
    const uint32_t sexColumn=4;
    PopulationPtr population=new Population;

    while(std::getline(in,line,'\n'))
	{
	++nLine;
	Tokenizer tab('\t');
	vector<string> tokens;
	tab.split(line,tokens);

	FamilyPtr family=(FamilyPtr)(population->findByName(tokens[familyColumn].c_str()));
	if(family==NULL)
	    {
	    family=new Family();
	    family->_population=population;
	    family->_id=tokens[familyColumn];
	    population->families.push_back(family);
	    }
	IndividualPtr indi= new Individual();
	indi->_index=nLine;
	indi->_id=tokens[nameColumn];
	indi->_family=family;
	family->individuals.push_back(indi);
	if(tokens[fatherColumn].compare("0")!=0)
	    {
	    fathers.push_back(std::make_pair<IndividualPtr,std::string>(indi,tokens[fatherColumn]));
	    }
	if(tokens[motherColumn].compare("0")!=0)
	    {
	    mothers.push_back(std::make_pair<IndividualPtr,std::string>(indi,tokens[motherColumn]));
	    }
	if(tokens[sexColumn].compare("1")==0)
	    {
	    indi->_sex=Individual::male;
	    }
	else if(tokens[sexColumn].compare("2")==0)
	    {
	    indi->_sex=Individual::female;
	    }
	else if(tokens[sexColumn].compare("0")==0)
	    {
	    indi->_sex=Individual::unknown;
	    }
	else
	    {
	    THROW( "bad sex " << tokens[sexColumn]);
	    }
	}

    for(int side=0;side< 2;++side)
	{
	std::vector<std::pair<IndividualPtr,std::string> > & parents=(side==0?fathers:mothers);
	for(std::size_t i=0;i< parents.size();++i)
	    {
	    IndividualPtr p=(IndividualPtr)(parents.at(i).first->family()->findByName(parents.at(i).second.c_str()));
	    if(p==NULL)
		{
		std::ostringstream os;
		os << "Cannot find parent "<< parents.at(i).second << " for "<< parents.at(i).first->name();
		throw std::runtime_error(os.str());
		}
	    if(parents.at(i).first==p)
		{
		std::ostringstream os;
		os << "parent of itself ? "<< parents.at(i).second;
		throw std::runtime_error(os.str());
		}
	    p->children.insert(parents.at(i).first);

	    if(side==0)
		{
		if(p->sex()==Individual::female)
		    {
		    THROW("expected "<< p->name() << " to be a MALE father of " << parents.at(i).first->name());
		    }
		parents.at(i).first->_father=p;
		}
	    else
		{
		if(p->sex()==Individual::male)
		    {
		    THROW("expected "<< p->name() << " to be a FEMALE mother of " << parents.at(i).first->name());
		    }
		parents.at(i).first->_mother=p;
		}
	    }
	}

    return auto_ptr<Population>(population);
    }

#ifdef THIS_IS_A_TEST

PopulationList::PopulationList()
    {
    }

virtual PopulationList::~PopulationList()
    {
    while(!populations.empty())
	{
	delete populations.back();
	populations.pop_back();
	}
    }

PopulationList::size_type PopulationList::size() const
	{
	return (size_type)size();
	}

const PopulationPtr PopulationList::at(size_type i) const
    {
    return populations.at((std::vector<PopulationPtr>::size_type)i);
    }

const PopulationPtr PopulationList::operator[](size_type i) const
    {
    return at(i);
    }

const PopulationPtr PopulationList::findByName(const char* s) const
   {
    for(Family::size_type i=0;i< size();++i)
	{
	const PopulationPtr p=at(i);
	if(std::strcmp(p->name(),s)==0) return p;
	}
   return NULL;
   }

std::ostream& operator << (std::ostream& out,const PopulationList& o)
    {
    for(PopulationList::size_type i=0;i< o.size();++i)
	{
	out << *(o.at(i));
	}
    return out;
    }



void PopulationListFactory::crlf(std::string& line)
{
if(!line.empty() && line.at(line.size()-1)=='\r')
	{
	line.erase(line.size()-1,1);
	}
}

virtual int PopulationListFactory::populationColumn() const
    {
    return 6;
    }

virtual int PopulationListFactory::familyColumn() const
    {
    return 0;
    }

virtual int PopulationListFactory::nameColumn() const
    {
    return 1;
    }

virtual int PopulationListFactory::fatherColumn() const
    {
    return 2;
    }

virtual int PopulationListFactory::motherColumn() const
    {
    return 3;
    }

virtual int sexColumn() const
    {
    return 4;
    }

PopulationListFactory::PopulationListFactory()
    {
    }

virtual PopulationListFactory::~PopulationListFactory()
    {
    }





#endif
