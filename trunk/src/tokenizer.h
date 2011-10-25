#ifndef STR_TOKENIZER_H
#define STR_TOKENIZER_H

#include <string>
#include <vector>
class Tokenizer
	{
	public:
		char delim;
		Tokenizer():delim('\t')
			{
			}
		
		Tokenizer(char d):delim(d)
			{
			}

		std::vector<std::string>::size_type
		split(const std::string& line,std::vector<std::string>& tokens)
		    {
		    std::string::size_type prev=0;
		    std::string::size_type i=0;
		    tokens.clear();
		    while(i<=line.size())
				{
				if(i==line.size() || line[i]==delim)
				    {
				    tokens.push_back(line.substr(prev,i-prev));
				    if(i==line.size()) break;
				    prev=i+1;
				    }
				++i;
				}
			return tokens.size();
		    }
	};
#endif
