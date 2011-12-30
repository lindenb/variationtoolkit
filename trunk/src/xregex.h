#ifndef X_REGEX_H
#define X_REGEX_H
#include <memory>

class Regex;



class Matcher
    {
    private:
	void* _ptr;
	Matcher(void* p);
    public:
	~Matcher();
	int32_t begin() const;
	int32_t end() const;
    };


class Pattern
    {
    private:
	void* _ptr;
	Pattern(void* p);
    public:
	~Pattern();
	std::auto_ptr<Matcher> exec(const char* s);
    };

class Regex
    {
    private:
	bool case_sensible;
	bool extended;
	bool no_sub;
	bool new_line;
    public:
	Regex();
	~Regex();
	Regex& set_case_sensible(bool choice);
	Regex& set_extended(bool choice);
	Regex& set_no_sub(bool choice);
	Regex& set_new_line(bool choice);
	std::auto_ptr<Pattern> compile(const char* s);
    };


#endif
