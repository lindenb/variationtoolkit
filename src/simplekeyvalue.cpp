#include <string>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <libxml/xmlreader.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include "tokenizer.h"

using namespace std;

class SimpleKeyValue
	{
	public:
		enum Mode { OVERWITE,NOOVERWITE,UPDATE};
		xmlDocPtr dom;
		std::vector<std::string> path;
		char command;//'A': add , 'D': delete , 'T': add if not exists
		SimpleKeyValue():dom(0),command('A')
			{
			}
			


		void insert(xmlNodePtr root,
			  std::vector<std::string>::size_type pathindex,
			  const char* key,
			  const char* value,
			  Mode mode
			  )
			{
			if(pathindex<path.size())
				{
				if(path[pathindex].empty())
					{
					insert(root,pathindex+1,key,value,mode);
					return;
					}
				//find the node <map>
				for (xmlNodePtr cur_node = root->children;
					cur_node; cur_node = cur_node->next)
					{
					if (cur_node->type != XML_ELEMENT_NODE) continue;
					if(!xmlStrEqual(cur_node->name,BAD_CAST "map")) continue;
					xmlChar *name=xmlGetProp(cur_node,BAD_CAST"name");
					if(name==NULL || !xmlStrEqual(name,BAD_CAST path[pathindex].c_str())) continue;
					insert(cur_node,pathindex+1,key,value,mode);
					return;
					}
				xmlNodePtr node=xmlNewNode(0,BAD_CAST "map");
				xmlNewProp(node,BAD_CAST "name",BAD_CAST path[pathindex].c_str());
				xmlAddChild(root,node);
				insert(node,pathindex+1,key,value,mode);
				return;
				}
			//find the node <entry>
			bool found=false;
			for (xmlNodePtr cur_node = root->children;
				cur_node; cur_node = cur_node->next)
				{
				if (cur_node->type != XML_ELEMENT_NODE) continue;
				if(!xmlStrEqual(cur_node->name,BAD_CAST "entry")) continue;
				xmlChar *name=xmlGetProp(cur_node,BAD_CAST "key");
				if(name==NULL) continue;
				if(xmlStrEqual(name,BAD_CAST key))
					{
					found=true;
					if(mode==NOOVERWITE) return;
					xmlUnlinkNode(cur_node);
					xmlFreeNode(cur_node);
					break;
					}
				
				}
			if(!found && mode==UPDATE) return;
			xmlNodePtr entry=xmlNewNode(0, BAD_CAST "entry");
			xmlNewProp(entry,BAD_CAST "key",BAD_CAST key);
			xmlAddChild(root,entry);
			xmlAddChild(entry,xmlNewText(BAD_CAST value));
			}
		
		void insert(const char* key,const char* value,Mode mode)
			{
			xmlNodePtr root=xmlDocGetRootElement(this->dom);
			if(root==0)
				{
				cerr << "DOM error. No root element" << endl;
				exit(EXIT_FAILURE);
				}
			insert(root,0,key,value,mode);
			}
		
		void remove(xmlNodePtr root,
			  std::vector<std::string>::size_type pathindex,
			  const char* key
			  )
			{
			if(pathindex<path.size())
				{
				if(path[pathindex].empty())
					{
					remove(root,pathindex+1,key);
					return;
					}
				//find the node <map>
				for (xmlNodePtr cur_node = root->children;
					cur_node; cur_node = cur_node->next)
					{
					if (cur_node->type != XML_ELEMENT_NODE) continue;
					if(!xmlStrEqual(cur_node->name,BAD_CAST "map")) continue;
					xmlChar *name=xmlGetProp(cur_node,BAD_CAST "name");
					if(name==NULL || !xmlStrEqual(name,BAD_CAST path[pathindex].c_str())) continue;
					remove(cur_node,pathindex+1,key);
					return;
					}
				return;
				}
			
			//find the node <entry> and delete it
			for (xmlNodePtr cur_node = root->children;
				cur_node; cur_node = cur_node->next)
				{
				if (cur_node->type != XML_ELEMENT_NODE) continue;

				if(!xmlStrEqual(cur_node->name,BAD_CAST "entry")) continue;
				xmlChar *name=xmlGetProp(cur_node,BAD_CAST "key");
				if(name==NULL) continue;

				if(xmlStrEqual(name,BAD_CAST key))
					{
					xmlUnlinkNode(cur_node);
					xmlFreeNode(cur_node);
					return;
					}
				
				}
			}
				
		void remove(const char* key)
			{
			xmlNodePtr root=xmlDocGetRootElement(this->dom);
			if(root==0)
				{
				cerr << "DOM error. No root element" << endl;
				exit(EXIT_FAILURE);
				}
			remove(root,0,key);
			}
		void usage(int argc,char** argv)
			{
			cout << argv[0] << endl;
			cerr << argv[0] << "Pierre Lindenbaum PHD. 2011.\n";
			cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
			cerr << "Usage: "<<".\n";
			cerr << "  "<< argv[0]<<" -f file.xml key value"<<".\n";
			cerr << "  "<< argv[0]<<" -f file.xml -A key value"<<".\n";
			cerr << "  "<< argv[0]<<" -f file.xml -T key value"<<".\n";
			cerr << "  "<< argv[0]<<" -f file.xml -U key value"<<".\n";
			cerr << "  "<< argv[0]<<" -f file.xml -D key"<<".\n";
			cerr << "Options:\n";
			cerr << "  -p (path) optional path of data\n";
			cerr << "  -f (file) xml file. REQUIRED.\n";
			cerr << "  -A add key/value. Overwrite previous data (this is the default).\n";
			cerr << "  -T add key/value. No not overwrite previous data .\n";
			cerr << "  -U update key/value. update data. Do not create the key if it doesn't already exists.\n";
			cerr << "  -D delete key.\n";
			cerr << "(stdin|file|file.gz)\n";
			}
		
		int main(int argc,char** argv)
			{
			Mode mode=OVERWITE;
			char* filename(0);
			int optind=1;
			while(optind < argc)
				{
				if(std::strcmp(argv[optind],"-h")==0)
					{
					this->usage(argc,argv);
					return EXIT_FAILURE;
					}

				else if(std::strcmp(argv[optind],"-f")==0 && optind+1<argc)
					{
					filename=argv[++optind];
					}
				else if(std::strcmp(argv[optind],"-p")==0 && optind+1<argc)
					{
					Tokenizer t('/');
					string p(argv[++optind]);
					t.split(p,this->path);
					}
				else if(std::strcmp(argv[optind],"-A")==0)
					{
					this->command='A';
					mode=OVERWITE;
					}
				else if(std::strcmp(argv[optind],"-T")==0)
					{
					this->command='T';
					mode=NOOVERWITE;
					}
				else if(std::strcmp(argv[optind],"-U")==0)
					{
					this->command='U';
					mode=UPDATE;
					}
				else if(std::strcmp(argv[optind],"-D")==0)
					{
					this->command='D';
					}
				else if(std::strcmp(argv[optind],"--")==0)
					{
					++optind;
					break;
					}
				else if(argv[optind][0]=='-')
					{
					cerr << "unknown option '"<< argv[optind] <<"'"<< endl;
					this->usage(argc,argv);
					exit(EXIT_FAILURE);
					}
				else
					{
					break;
					}
				++optind;
				}
			if(filename==0)
				{
				cerr << "filename missing." << endl;
				this->usage(argc,argv);
				exit(EXIT_FAILURE);
				}
			this->dom=xmlParseFile(filename);
			if(this->dom==0)
				{
				cerr << "[INFO] cannot read xml file "<< filename << ". will create it" << endl;
				this->dom=::xmlNewDoc( BAD_CAST "1.0");
				xmlDocSetRootElement(this->dom,xmlNewNode(0, BAD_CAST "map"));
				}
			switch(command)
				{
				case 'T':
				case 'A':
				case 'U':
					{
					if(optind+2!=argc)
						{
						cerr << "Illegal number of arguments." << endl;
						exit(EXIT_FAILURE);
						}
					insert(argv[optind],argv[optind+1],mode);
					break;
					}
				case 'D':
					{
					if(optind+1!=argc)
						{
						cerr << "Illegal number of arguments." << endl;
						exit(EXIT_FAILURE);
						}
					remove(argv[optind]);
					break;
					}
				default:
					{
					cerr << "Unknown command." << endl;
					this->usage(argc,argv);
					exit(EXIT_FAILURE);
					break;
					}
				}
			FILE* fout=fopen(filename,"w");
			if(fout==NULL)
				{
				cerr << "Cannot write to " << filename << " " << strerror(errno) << endl;
				return EXIT_FAILURE;
				}
			xmlDocDump(fout,this->dom);
			fclose(fout);
			return 0;
			}
	};
	
int main(int argc,char** argv)
	{
	SimpleKeyValue app;
	return app.main(argc,argv);
	}
