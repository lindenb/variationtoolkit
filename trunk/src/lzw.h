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
		std::size_t complexity(const char* s,std::size_t len)
			{
			std::set<std::string> dict;
			for (int c = 'a'; c <= 'z';c++)
				{
				if(case_sensible)
					{
					dict.insert(std::string(1,c));
					}
				dict.insert(std::string(1,toupper(c)));
				}

			std::string w;
			for(std::size_t i=0;i< len;++i)
				{
				char c=(case_sensible?s[i]:std::toupper(s[i]));
				
				if(!std::isalpha(c)) continue;
				std::string wc(w);
				
				wc.append(&c,1);
				if( dict.find(wc)!= dict.end())
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

