/*
 * tablemodel.h
 *
 *  Created on: Oct 26, 2011
 *      Author: lindenb
 */

#ifndef TABLEMODEL_H_
#define TABLEMODEL_H_
#include <cstddef>
#include <vector>
#include <string>

class TableModel
    {
    public:
	TableModel() {}
	virtual ~TableModel() {}
	virtual std::size_t rows()=0;
	virtual std::size_t columns() =0;
	virtual const char* column(std::size_t x)=0;
	virtual const char* get(std::size_t y,std::size_t x)=0;
    };

class DefaultTableModel:public TableModel
    {

    public:
	std::vector<std::string> header;
	std::vector<std::vector<std::string> > table;

	DefaultTableModel() {}
	virtual ~DefaultTableModel() {}
	virtual std::size_t rows()
	    {
	    return table.size();
	    }
	virtual std::size_t columns()
	    {
	    return header.size();
	    }
	virtual const char* column(std::size_t x)
	    {
	    return x>=header.size()?NULL:header.at(x).c_str();
	    }
	virtual const char* get(std::size_t y,std::size_t x)
	    {
	    return y>=table.size()?NULL : x>=table.at(y).at(x).size()?NULL:table.at(y).at(x).c_str();
	    }
    };

#endif /* TABLEMODEL_H_ */
