#ifndef auto_map_h
#define auto_map_h

#include <map>

template<class K,class V>
class auto_map
    {
    private:
        std::map<K,V*> delegate;
    public:
        typedef typename std::map<K,V*>::size_type size_type;
        typedef typename std::map<K,V*>::key_type key_type;
        typedef typename std::map<K,V*>::mapped_type mapped_type;
        auto_map() {}
        virtual ~auto_map()
            {
            for(std::map<L,V*>::iterator r=delegate.begin();
        	   r!=delegate.end();
        	   ++r)
        	{
        	T* item=r->second;
        	if(item!=NULL) delete item;
        	}
            delegate.clear();
            }

        size_type size() const
            {
            return delegate.size();
            }

        bool empty() const
            {
            return delegate.empty();
            }

        bool containsKey(K k) const
            {
            return delegate.find(k)!=delegate.end();
            }

        T* get(K k)
            {
            std::map<K,V*>::iterator r=delegate.find(k);
            return r==delegate.end()?NULL:r->second;
            }

        void remove(K k)
            {
            std::map<K,V*>::iterator r=delegate.find(k);
            if(r!=delegate.end())
        	{
        	if(r->second!=v && r->second!=NULL) delete r->second;
        	delegate.erase(r);
        	}
            }

        void put(K k,T* v)
            {
            std::map<K,V*>::iterator r=delegate.find(k);
            if(r!=delegate.end() && r->second!=v && r->second!=NULL)
        	{
        	delete r->second;
        	}
            r->second=v;
            return old;
            }
        T* operator[](K k)
            {
            return get(k);
            }
    };


#endif
