#ifndef BEDMAP_H
#define BEDMAP_H
#include <string>
#include <map>
#include <list>
#include <iterator>
#include <memory>
#include <stdint.h>

template <typename T>
class BedMap
	{
	protected:
		class Row
			{
			public:
				int32_t chromStart;
				int32_t chromEnd;
				T data;	
				Row(int32_t chromStart,int32_t chromEnd,T data):chromStart(chromStart),
					chromEnd(chromEnd),
					data(data)
					{
					}
				Row(const Row& cp):chromStart(cp.chromStart),
					chromEnd(cp.chromEnd),
					data(cp.data)
					{
					}
				Row& operator=(const Row& cp)
					{
					if(this!=&cp)
						{
						chromStart=cp.chromStart;
						chromEnd=cp.chromEnd;
						data=cp.data;
						}
					return *this;
					}
			};
		typedef typename std::list<Row> rows_t;
		typedef typename std::map<int32_t,rows_t> bin2rows_t;
		typedef typename std::map<std::string,bin2rows_t> chrom2bins_t;
		chrom2bins_t _mp;
		int32_t bin(int32_t chromStart,int32_t chromEnd) const
			{
			
			return 0;
			}
		
		void fill_bins(int32_t chromStart,int32_t chromEnd,std::list<int32_t>& L)
			{
			L.clear();
			}
		
	public:
		BedMap() {}
		virtual ~BedMap() {}
		virtual void put(const char* chrom,int32_t chromStart,int32_t chromEnd,const T data)
			{
			std::string C(chrom);
			typename chrom2bins_t::iterator r=_mp.find(C);
			if(r==_mp.end())
				{
				bin2rows_t tmp;
				r=_mp.insert(std::make_pair<std::string,bin2rows_t>(C,tmp)).first;
				}
			int32_t b=bin(chromStart,chromEnd);
			typename bin2rows_t::iterator r2= r->second.find(b);
			if(r2==r->second.end())
				{
				rows_t tmp;
				r2=r->second.insert(std::make_pair<int32_t,rows_t>(b,tmp)).first;
				}
			Row row(chromStart,chromEnd,data);
			r2->second.push_back(row);
			}
		virtual void clear()
			{
			_mp.clear();
			}
	public:
		class BedRecord
			{
			public:
			};
		
		class Walker
			{
			enum { INIT,WALK_NEXT_DATA,WALK_NEXT_BIN,WALK_NEXT_CHROM,WALK_END};
			
				BedMap<T>* owner;
				int state;
				typename chrom2bins_t::iterator rc;
				typename bin2rows_t::iterator rb;
				typename rows_t::iterator rd;
				std::string limitChrom;
				int32_t limitStart;
				int32_t limitEnd;
				std::list<int32_t> bin_list;
			public:
				Walker(BedMap<T>* owner):owner(owner),state(INIT),
						limitChrom(""),limitStart(-1),limitEnd(-1)
					{
					}
					
				Walker(BedMap<T>* owner,const char* chrom,int32_t chromStart,int32_t chromEnd)
						:owner(owner),state(INIT),
						limitChrom(chrom),limitStart(chromStart),limitEnd(chromEnd)
					{
					owner->fill_bins(chromStart,chromEnd,bin_list);
					}
				Walker(BedMap<T>* owner,const char* chrom)
						:owner(owner),state(INIT),
						limitChrom(chrom),limitStart(-1),limitEnd(-1)
					{
					}
				const std::string& chrom() const
					{
					return rc->first;
					}
				bool next()
					{
					for(;;)
						{
						switch(state)
							{
							case INIT:
								if(this->limitChrom.empty())
									{
									rc=owner->_mp.begin();
									}
								else
									{
									rc=owner->_mp.find(this->limitChrom);
									}
								
								if(rc==owner->_mp.end())
									{
									state=WALK_END;
									return false;
									}
								if(chromStart>=0 && chromEnd>=0)
									{
									rb=rc->second.begin();
									}
								else
									{
									rb=rc->second.find(bin_list.pop_back());
									}
								if(rb==rc->second.end())
									{
									if(chromStart>=0 && chromEnd>=0)
										{
										state=WALK_END;
										return false;
										}
									else
										{
										state=WALK_NEXT_CHROM;
										}
									continue;
									}
								rd=rb->second.begin();
								if(rd==rb->second.end())
									{
									state=WALK_NEXT_BIN;
									continue;
									}
								state=WALK_NEXT_DATA;
								return true;
								break;
							case WALK_NEXT_DATA:
								{
								++rd;
								if(rd==rb->second.end())
									{
									state=WALK_NEXT_BIN;
									continue;
									}
								return true;
								break;
								}
							case WALK_NEXT_BIN:
								{
								++rb;
								if(rb==rc->second.end())
									{
									state=WALK_NEXT_CHROM;
									continue;
									}
								return true;
								break;
								}
							case WALK_NEXT_CHROM:
								{
								if(!this->limitChrom.empty())
									{
									state=WALK_END;
									return false;
									}
								++rc;
								if(rc==owner->_mp.end())
									{
									state=WALK_END;
									return false;
									}
								return true;
								}
							case WALK_END: return false;
							}
						}
					return false;
					}
					
			friend class BedMap;
			};
		
		std::auto_ptr<Walker> scan()
			{
			Walker* w=new Walker(this);
			return std::auto_ptr<Walker>(w);
			}
		std::auto_ptr<Walker> scan(const char* chrom)
			{
			Walker* w=new Walker(this,chrom);
			return std::auto_ptr<Walker>(w);
			}
		
		std::auto_ptr<Walker> scan(const char* chrom,int32_t chromStart,int32_t chromEnd)
			{
			Walker* w=new Walker(this,chrom,chromStart,chromEnd);
			return std::auto_ptr<Walker>(w);
			}
		
		class Iterator: public std::iterator<
				std::input_iterator_tag,
				BedRecord,std::ptrdiff_t,
				const BedRecord*,
				const BedRecord&>
			{
			private:
				BedMap<T>* owner;
				int32_t chromStart;
				int32_t chromEnd;
				typename chrom2bins_t::iterator rc;
				typename bin2rows_t::iterator rb;
				typename rows_t::iterator rd;
			public:
				Iterator(BedMap<T>* owner):owner(owner),chromStart(-1),chromEnd(-1)
					{
					
					}
				Iterator(const Iterator& cp)
					{
					}
				Iterator& operator=(const Iterator& cp)
					{
					return *this;
					}
				Iterator& operator++()
					{
					return *this;
					}
        			Iterator operator++(int) const
        				{
        				Iterator cp(*this);
        				return cp;
        				}
        			bool operator==(const Iterator& cp) const
					{
					return true;
					}
				bool operator!=(const Iterator& cp) const
					{
					return true;
					}

				};
	
		
		typedef Iterator iterator;
		iterator begin()
			{
			return Iterator(this);
			}
		iterator end()
			{
			return Iterator(this);
			}
	};

#endif

