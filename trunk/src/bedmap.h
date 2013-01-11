#ifndef BEDMAP_H
#define BEDMAP_H
#include <string>
#include <map>
#include <vector>
#include <iterator>
#include <memory>
#include <stdint.h>

template <typename T>
class BedRanges
	{
	protected:
		class Row
			{
			public:
				int32_t chromStart;
				int32_t chromEnd;
				T data;
				Row(int32_t chromStart):chromStart(chromStart),chromEnd(chromStart+1)
					{
					}
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
				bool operator < (const Row& cp) const
					{
					return chromStart< cp.chromStart;
					} 
			};
		typedef typename std::vector<Row> rows_t;
		typedef typename std::map<int32_t,rows_t> bin2rows_t;

		bool overlap(int32_t chromStart,int32_t chromEnd, const Row& r) const
			{
			return !(chromEnd< r.chromStart || r.chromEnd<chromStart);
			}
		int32_t bin(int32_t chromStart,int32_t chromEnd) const
			{
			uint32_t beg(chromStart);
			uint32_t end(chromEnd);
			--end;
			if (beg>>14 == end>>14) return 4681 + (beg>>14);
			if (beg>>17 == end>>17) return  585 + (beg>>17);
			if (beg>>20 == end>>20) return   73 + (beg>>20);
			if (beg>>23 == end>>23) return    9 + (beg>>23);
			if (beg>>26 == end>>26) return    1 + (beg>>26);
			return 0;
			}
		
		void fill_bins(int32_t chromStart,int32_t chromEnd,std::vector<int32_t>& L) const
			{
			int32_t beg(chromStart);
			int32_t end(chromEnd);
			
			L.clear();

			int k;
			if (beg >= end) return;
			if ((uint32_t)end >= 1u<<29) end = 1u<<29;
			--end;
			L.push_back(0);
			for (k =    1 + (beg>>26); k <=    1 + (end>>26); ++k) { L.push_back(k); }
			for (k =    9 + (beg>>23); k <=    9 + (end>>23); ++k) { L.push_back(k); }
			for (k =   73 + (beg>>20); k <=   73 + (end>>20); ++k) { L.push_back(k); }
			for (k =  585 + (beg>>17); k <=  585 + (end>>17); ++k) { L.push_back(k); }
			for (k = 4681 + (beg>>14); k <= 4681 + (end>>14); ++k) { L.push_back(k); }
			}

		bin2rows_t _m;
	public:
		BedRanges() {}
		virtual ~BedRanges() {}
		
		typedef int (*CallBack)(const BedMap<T>* ,int32_t ,int32_t , int32_t ,int32_t,const T& ,void*);



		virtual bool contains(int32_t chromStart,int32_t chromEnd) const
			{
			return one(chrom,chromStart,chromEnd)!=0;
			}

		virtual void put(int32_t chromStart,int32_t chromEnd,const T data)
			{
			int32_t b=bin(chromStart,chromEnd);
			typename bin2rows_t::iterator r2=this->_m.find(b);
			if(r2==r->second.end())
				{
				rows_t tmp;
				r2=r->second.insert(std::make_pair<int32_t,rows_t>(b,tmp)).first;
				}
			Row row(chromStart,chromEnd,data);
			typename rows_t::iterator rd=std::lower_bound(r2->second.begin(), r2->second.end(),row);
			r2->second.insert(rd,row);
			}
		virtual void clear()
			{
			_mp.clear();
			}
	public:
	
		class Walker
			{
			private:
				enum STATE { INIT,WALK_NEXT_DATA,WALK_NEXT_BIN,WALK_END};
			
				const BedMap<T>* owner;
				STATE state;
				typename chrom2bins_t::const_iterator rc;
				typename bin2rows_t::const_iterator rb;
				typename rows_t::const_iterator rd;
				std::string limitChrom;
				int32_t limitStart;
				int32_t limitEnd;
				std::vector<int32_t> bin_list;
				std::vector<int32_t>::const_iterator r_bin;
				bool is_user_defined_chrom() const
					{
					return !this->limitChrom.empty();
					}
				bool is_user_defined_range() const
					{
					return is_user_defined_chrom() && limitStart!=-1 && limitEnd>=limitStart;
					}
			public:
				Walker(const Walker& cp):owner(cp.owner),state(cp.state),
						rc(cp.rc),rb(cp.rb),rd(cp.rb),
						limitChrom(cp.limitChrom),
						limitStart(cp.limitStart),
						limitEnd(cp.limitEnd),
						bin_list(cp.bin_list),
						r_bin(cp.r_bin)
					{
					
					}
				/** whole genome scanning */
				Walker(const BedMap<T>* owner):owner(owner),state(INIT),
						limitChrom(""),limitStart(-1),limitEnd(-1)
					{
					}
					
				Walker(const BedMap<T>* owner,const char* chrom,int32_t chromStart,int32_t chromEnd)
						:owner(owner),state(INIT),
						limitChrom(chrom),limitStart(chromStart),limitEnd(chromEnd)
					{
					owner->fill_bins(chromStart,chromEnd,bin_list);
					}
				/** one chromosome scanning */
				Walker(const BedMap<T>* owner,const char* chrom)
						:owner(owner),state(INIT),
						limitChrom(chrom),limitStart(-1),limitEnd(-1)
					{
					}
				const std::string& chrom() const
					{
					return rc->first;
					}
				const T& data() const
					{
					return rd->data;
					}
				int32_t start() const
					{
					return rd->chromStart;
					}
				int32_t end() const
					{
					return rd->chromEnd;
					}
				bool next()
					{
					for(;;)
						{
						//WHERE("state " << state);
						switch(state)
							{
							case INIT:
								if(!this->is_user_defined_chrom())
									{
									rc=owner->_mp.begin();
									}
								else
									{
									/* find chromosome */
									rc=owner->_mp.find(this->limitChrom);
									}
								/** chromosome is not in the list */
								if(rc==owner->_mp.end())
									{
									state=WALK_END;
									return false;
									}
								
								/* no range defined */
								if(!is_user_defined_range())
									{
									rb=rc->second.begin();
									}
								else
									{
									/* loop until we find a bin */
									r_bin = bin_list.begin();
									while(r_bin != bin_list.end() )
										{
										//WHERE("bin-id is "<< *r_bin <<" "<< owner->bin(100,200));
										rb=rc->second.find(*r_bin);
										if(rb!=rc->second.end()) break;
										++r_bin;
										}
									}
								/* no bin in that map */
								if(rb==rc->second.end())
									{
									/* user range has been defined */
									if(is_user_defined_range())
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
								
								
								/* user range has been defined */
								if(is_user_defined_range())
									{
									Row tmp(limitStart);
									rd=std::lower_bound(rb->second.begin(),rb->second.end(),tmp);
									while(rd!=rb->second.end())
										{
										//WHERE("("<< limitStart << "-" << limitEnd << " vs "<< rd->chromStart << "-" << rd->chromEnd);
										if(owner->overlap(limitStart,limitEnd,*rd))
											{
											break;
											}
										++rd;
										}
									if(rd==rb->second.end())
										{
										state=WALK_NEXT_BIN;
										continue;
										}
									state=WALK_NEXT_DATA;
									return true;
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
								/* advance one */
								++rd;
								/* user defined range */
								if(is_user_defined_range())
									{
									while(rd!=rb->second.end())
										{
										if(owner->overlap(limitStart,limitEnd,*rd)) break;
										++rd;
										}
									if(rd==rb->second.end())
										{
										state=WALK_END;
										return false;
										}
									state=WALK_NEXT_DATA;
									return true;
									}
								
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
								/* user defined range */
								if(is_user_defined_range())
									{
									/* loop until we find a bin */
									++r_bin;
									while(r_bin != bin_list.end() )
										{
										this->rb=rc->second.find(*r_bin);
										if(rb!=rc->second.end())
											{
											Row tmp(limitStart);
											rd=std::lower_bound(rb->second.begin(),rb->second.end(),tmp);
											while(rd!=rb->second.end())
												{
												if(owner->overlap(limitStart,limitEnd,*rd))
													{
													state=WALK_NEXT_DATA;
													return true;
													}
												++rd;
												}
											}
										++r_bin;
										}
									
									state=WALK_END;
									return false;	
									}
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
								if(this->is_user_defined_chrom())
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
		
		std::auto_ptr<Walker> walker() const
			{
			Walker* w=new Walker(this);
			return std::auto_ptr<Walker>(w);
			}
		
		std::auto_ptr<Walker> walker(int32_t chromStart,int32_t chromEnd) const
			{
			Walker* w=new Walker(this,chrom,chromStart,chromEnd);
			return std::auto_ptr<Walker>(w);
			}
		
	};


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
				Row(int32_t chromStart):chromStart(chromStart),chromEnd(chromStart+1)
					{
					}
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
				bool operator < (const Row& cp) const
					{
					return chromStart< cp.chromStart;
					} 
			};
		typedef typename std::vector<Row> rows_t;
		typedef typename std::map<int32_t,rows_t> bin2rows_t;
		typedef typename std::map<std::string,bin2rows_t> chrom2bins_t;
		chrom2bins_t _mp;

		bool overlap(int32_t chromStart,int32_t chromEnd, const Row& r) const
			{
			return !(chromEnd< r.chromStart || r.chromEnd<chromStart);
			}
		int32_t bin(int32_t chromStart,int32_t chromEnd) const
			{
			uint32_t beg(chromStart);
			uint32_t end(chromEnd);
			--end;
			if (beg>>14 == end>>14) return 4681 + (beg>>14);
			if (beg>>17 == end>>17) return  585 + (beg>>17);
			if (beg>>20 == end>>20) return   73 + (beg>>20);
			if (beg>>23 == end>>23) return    9 + (beg>>23);
			if (beg>>26 == end>>26) return    1 + (beg>>26);
			return 0;
			}
		
		void fill_bins(int32_t chromStart,int32_t chromEnd,std::vector<int32_t>& L) const
			{
			int32_t beg(chromStart);
			int32_t end(chromEnd);
			
			L.clear();

			int k;
			if (beg >= end) return;
			if ((uint32_t)end >= 1u<<29) end = 1u<<29;
			--end;
			L.push_back(0);
			for (k =    1 + (beg>>26); k <=    1 + (end>>26); ++k) { L.push_back(k); }
			for (k =    9 + (beg>>23); k <=    9 + (end>>23); ++k) { L.push_back(k); }
			for (k =   73 + (beg>>20); k <=   73 + (end>>20); ++k) { L.push_back(k); }
			for (k =  585 + (beg>>17); k <=  585 + (end>>17); ++k) { L.push_back(k); }
			for (k = 4681 + (beg>>14); k <= 4681 + (end>>14); ++k) { L.push_back(k); }
			}
		
	public:
		BedMap() {}
		virtual ~BedMap() {}
		
		typedef int (*CallBack)(const BedMap<T>* ,const char* ,int32_t ,int32_t , int32_t ,int32_t,const T& ,void*);



		virtual bool contains(const char* chrom,int32_t chromStart,int32_t chromEnd) const
			{
			return one(chrom,chromStart,chromEnd)!=0;
			}

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
			typename rows_t::iterator rd=std::lower_bound(r2->second.begin(), r2->second.end(),row);
			r2->second.insert(rd,row);
			}
		virtual void clear()
			{
			_mp.clear();
			}
	public:
	
		class Walker
			{
			private:
				enum STATE { INIT,WALK_NEXT_DATA,WALK_NEXT_BIN,WALK_NEXT_CHROM,WALK_END};
			
				const BedMap<T>* owner;
				STATE state;
				typename chrom2bins_t::const_iterator rc;
				typename bin2rows_t::const_iterator rb;
				typename rows_t::const_iterator rd;
				std::string limitChrom;
				int32_t limitStart;
				int32_t limitEnd;
				std::vector<int32_t> bin_list;
				std::vector<int32_t>::const_iterator r_bin;
				bool is_user_defined_chrom() const
					{
					return !this->limitChrom.empty();
					}
				bool is_user_defined_range() const
					{
					return is_user_defined_chrom() && limitStart!=-1 && limitEnd>=limitStart;
					}
			public:
				Walker(const Walker& cp):owner(cp.owner),state(cp.state),
						rc(cp.rc),rb(cp.rb),rd(cp.rb),
						limitChrom(cp.limitChrom),
						limitStart(cp.limitStart),
						limitEnd(cp.limitEnd),
						bin_list(cp.bin_list),
						r_bin(cp.r_bin)
					{
					
					}
				/** whole genome scanning */
				Walker(const BedMap<T>* owner):owner(owner),state(INIT),
						limitChrom(""),limitStart(-1),limitEnd(-1)
					{
					}
					
				Walker(const BedMap<T>* owner,const char* chrom,int32_t chromStart,int32_t chromEnd)
						:owner(owner),state(INIT),
						limitChrom(chrom),limitStart(chromStart),limitEnd(chromEnd)
					{
					owner->fill_bins(chromStart,chromEnd,bin_list);
					}
				/** one chromosome scanning */
				Walker(const BedMap<T>* owner,const char* chrom)
						:owner(owner),state(INIT),
						limitChrom(chrom),limitStart(-1),limitEnd(-1)
					{
					}
				const std::string& chrom() const
					{
					return rc->first;
					}
				const T& data() const
					{
					return rd->data;
					}
				int32_t start() const
					{
					return rd->chromStart;
					}
				int32_t end() const
					{
					return rd->chromEnd;
					}
				bool next()
					{
					for(;;)
						{
						//WHERE("state " << state);
						switch(state)
							{
							case INIT:
								if(!this->is_user_defined_chrom())
									{
									rc=owner->_mp.begin();
									}
								else
									{
									/* find chromosome */
									rc=owner->_mp.find(this->limitChrom);
									}
								/** chromosome is not in the list */
								if(rc==owner->_mp.end())
									{
									state=WALK_END;
									return false;
									}
								
								/* no range defined */
								if(!is_user_defined_range())
									{
									rb=rc->second.begin();
									}
								else
									{
									/* loop until we find a bin */
									r_bin = bin_list.begin();
									while(r_bin != bin_list.end() )
										{
										//WHERE("bin-id is "<< *r_bin <<" "<< owner->bin(100,200));
										rb=rc->second.find(*r_bin);
										if(rb!=rc->second.end()) break;
										++r_bin;
										}
									}
								/* no bin in that map */
								if(rb==rc->second.end())
									{
									/* user range has been defined */
									if(is_user_defined_range())
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
								
								
								/* user range has been defined */
								if(is_user_defined_range())
									{
									Row tmp(limitStart);
									rd=std::lower_bound(rb->second.begin(),rb->second.end(),tmp);
									while(rd!=rb->second.end())
										{
										//WHERE("("<< limitStart << "-" << limitEnd << " vs "<< rd->chromStart << "-" << rd->chromEnd);
										if(owner->overlap(limitStart,limitEnd,*rd))
											{
											break;
											}
										++rd;
										}
									if(rd==rb->second.end())
										{
										state=WALK_NEXT_BIN;
										continue;
										}
									state=WALK_NEXT_DATA;
									return true;
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
								/* advance one */
								++rd;
								/* user defined range */
								if(is_user_defined_range())
									{
									while(rd!=rb->second.end())
										{
										if(owner->overlap(limitStart,limitEnd,*rd)) break;
										++rd;
										}
									if(rd==rb->second.end())
										{
										state=WALK_END;
										return false;
										}
									state=WALK_NEXT_DATA;
									return true;
									}
								
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
								/* user defined range */
								if(is_user_defined_range())
									{
									/* loop until we find a bin */
									++r_bin;
									while(r_bin != bin_list.end() )
										{
										this->rb=rc->second.find(*r_bin);
										if(rb!=rc->second.end())
											{
											Row tmp(limitStart);
											rd=std::lower_bound(rb->second.begin(),rb->second.end(),tmp);
											while(rd!=rb->second.end())
												{
												if(owner->overlap(limitStart,limitEnd,*rd))
													{
													state=WALK_NEXT_DATA;
													return true;
													}
												++rd;
												}
											}
										++r_bin;
										}
									
									state=WALK_END;
									return false;	
									}
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
								if(this->is_user_defined_chrom())
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
		
		std::auto_ptr<Walker> walker() const
			{
			Walker* w=new Walker(this);
			return std::auto_ptr<Walker>(w);
			}
		std::auto_ptr<Walker> walker(const char* chrom) const
			{
			Walker* w=new Walker(this,chrom);
			return std::auto_ptr<Walker>(w);
			}
		
		std::auto_ptr<Walker> walker(const char* chrom,int32_t chromStart,int32_t chromEnd) const
			{
			Walker* w=new Walker(this,chrom,chromStart,chromEnd);
			return std::auto_ptr<Walker>(w);
			}
		
		virtual void forEach(const char* chrom,int32_t chromStart,int32_t chromEnd,CallBack callback,void* userData) const
			{
			std::auto_ptr<Walker> w = walker(chrom,chromStart,chromEnd);
			
			while(w->next())
				{
				if(callback(this,chrom,chromStart,chromEnd,w->start(),w->end(),w->data(),userData)!=0) 
					{
					break;
					}
				}
			}
		
	private:
		static int _one_callback(const BedMap<T>* ,const char* chrom,int32_t qStart,int32_t qEnd,int32_t chromStart,int32_t chromEnd, const T& data,void* userData)
			{
			T const ** D=static_cast<T const **>(userData);
			*D=&data;
			return 1;
			}
	public:
		virtual const T* one(const char* chrom,int32_t chromStart,int32_t chromEnd) const
			{
			T const* data=0;
			forEach(chrom,chromStart,chromEnd,_one_callback,&data);
			return data;			
			}	
	};

#endif

