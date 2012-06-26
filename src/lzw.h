#ifndef LZW_H
#define LZW_H

#include <set>
#include <cctype>
#include <string>
#include <iterator>


class LZWComplexity
	{
	private:
		bool case_sensible;
	public:
		LZWComplexity():case_sensible(false)
			{
			}
		~LZWComplexity()
			{
			}
		void set_case_sensible(bool b)
			{
			this->case_sensible=b;
			}
		
		
		std::size_t complexity(const char* s,std::size_t len) const
			{
			std::set<std::string> dict;

			std::string w;
			std::string wc;
			w.reserve(len);
			wc.reserve(len);
			for(std::size_t i=0;i< len;++i)
				{
				char c=(case_sensible?s[i]:std::toupper(s[i]));
				
				if(!std::isalpha(c)) continue;
				
				wc.assign(w);
				wc.append(&c,1);
				
				if(wc.size()==1 || /* dict already contains all 1-char word */
					dict.find(wc)!= dict.end() /* wc is in dict */
					)
					{
					w.assign(wc);
					}
				else
					{
					dict.insert(wc);
					w.assign(&c,1);
					}
				}
			return dict.size();
			}
	};


#endif

