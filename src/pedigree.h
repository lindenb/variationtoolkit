#ifndef TRANS_PEDIGREE_H
#define TRANS_PEDIGREE_H
#include <iostream>
#include <string>
#include <sstream>
#include <cstdio>
#include <set>
#include <memory>
#include <stdint.h>

class Family;
class Population;




/**
 * Individual
 */
class Individual
    {
    public:
  	    enum Sex
  	        {
  	        unknown=0,
  	        male=1,
  	        female=2
  	        };
    private:
	    /** family for this individual */
	    Family* _family;
	    /** individual's name */
	    std::string _id;
	    /** link to father or NULL */
	    Individual* _father;
	    /** link to mother or NULL */
	    Individual* _mother;
	    /** sex is good */
	    Individual::Sex _sex;
	    /** children */
	    std::set<const Individual*> children;
	    /** index in pedigree file */
	    int _index;
	    Individual();
    public:


	    virtual ~Individual();
	    int index() const;
	   const char* name() const;
	   const Family* family() const;
	   const Population* population() const;
	   const Individual* parent(int index) const;
	   const Individual* father() const;
	   const Individual* mother() const;
	   bool hasFather() const;
	   bool hasMother() const;
	   bool hasParent() const;
	   bool hasParent(const Individual* parent) const;
	   bool isFatherOf(const Individual* child) const;
	   bool isMotherOf(const Individual* child) const;
	   bool isParentOf(const Individual* child) const;
	   Sex sex() const;

    friend class Family;
    friend class Population;
    friend class PopulationReader;
    friend  std::ostream& operator << (std::ostream& out,const Individual& o);
    };

typedef Individual* IndividualPtr;

std::ostream& operator << (std::ostream& out,const Individual& o);

/**
 *
 * Family
 *
 */
class Family
    {
    protected:

	    /** population for this family */
	    Population* _population;
	    /** all individuals for this family */
	    std::vector<IndividualPtr> individuals;
	    /** family name */
	    std::string _id;
	    Family();
    public:
	   typedef int32_t size_type;
	   virtual ~Family();
	   const char* name() const;
	   const Population* population() const;

	   size_type size() const;
	   const IndividualPtr at(size_type i) const;
	   const IndividualPtr operator[](size_type i) const;
	   const IndividualPtr findByName(const char* s) const;

    friend class Population;
    friend class PopulationReader;
    friend  std::ostream& operator << (std::ostream& out,const Family& o);
    };

typedef Family* FamilyPtr;

std::ostream& operator << (std::ostream& out,const Family& o);


/**
 *
 * Population
 *
 */
class Population
    {
    protected:
	    std::vector<FamilyPtr> families;
	    Population();
    public:
	    typedef int32_t size_type;
	    virtual ~Population();
	    size_type size() const;
	    const FamilyPtr at(size_type i) const;
	    const FamilyPtr operator[](size_type i) const;
	    const FamilyPtr findByName(const char* s) const;
    friend  std::ostream& operator << (std::ostream& out,const Population& o);
    friend class PopulationReader;
    };

typedef Population* PopulationPtr;

std::ostream& operator << (std::ostream& out,const Population& o);

class PopulationReader
    {
    public:
	PopulationReader();
	~PopulationReader();
	std::auto_ptr<Population> parse(const char* filename);
    };



#endif
