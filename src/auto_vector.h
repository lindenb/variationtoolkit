#ifndef auto_vector_h
#define auto_vector_h

#include <vector>

template<class T>
class auto_vector
    {
    private:
        std::vector<T*> delegate;
    public:
        typedef typename std::vector<T*>::size_type size_type;
        auto_vector() {}
        virtual ~auto_vector()
            {
            while(!delegate.empty())
                {
                T* item=delegate.back();
                if(item!=NULL) delete item;
                delegate.pop_back();
                }
            }
        void clear()
        	{
        	 while(!delegate.empty())
				{
				T* item=delegate.back();
				if(item!=NULL) delete item;
				delegate.pop_back();
				}
        	}

        bool empty() const
            {
            return delegate.empty();
            }

        T* push_back(T* t)
            {
            delegate.push_back(t);
            return t;
            }

        T* at(size_type i)
            {
            return delegate.at(i);
            }
        const T* at(size_type i) const
            {
            return delegate.at(i);
            }
        size_type size() const
            {
            return delegate.size();
            }
        T* operator[](size_type i)
            {
            return at(i);
            }

        const T* operator[](size_type i) const
            {
            return at(i);
            }

        void erase(size_type i)
        	{
        	T*  item=at(i);
        	if(item!=NULL) delete item;
        	delegate.erase(delegate.begin()+i);
        	}

        T* release(size_type i)
			{
			T* v=delegate.at(i);
			delegate.erase(delegate.begin()+i);
			return v;
			}

        void assign(size_type i,T* item)
            {
            if(delegate.at(i)!=NULL && delegate.at(i)!=item) delete delegate.at(i);
            delegate.assign(i,item);
            }

        void resize(size_type n)
            {
            while(size() < n)
        	{
        	push_back(NULL);
        	}
            while(size()>=n)
        	{
        	 T* item=delegate.back();
        	 if(item!=NULL) delete item;
        	 delegate.pop_back();
        	 }
            }

        T* back()
            {
            return delegate.back();
            }
        T* front()
			{
			return delegate.front();
			}
    };


#endif
