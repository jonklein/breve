#ifndef _SHARED_PTR_H
#define _SHARED_PTR_H

namespace push {
namespace detail {

template <class T> class weak_ptr;
    
template <class T>
class shared_ptr {
    private:
	T* ptr;
	size_t* count; //

	/* special case, null pointer (nil-code) */
	static size_t* nil() { static size_t nil_counter(1); return &nil_counter; }
	
	void decref() { if (--(*count) == 0) { delete ptr; delete count; }} 
	void incref() { ++(*count); }
	
	friend class weak_ptr<T>;
	
    public:
	
	shared_ptr() : ptr(0), count(nil()) { incref(); }
	~shared_ptr() { decref(); }
	
	shared_ptr(const shared_ptr<T>& o) : ptr(o.ptr), count(o.count) { incref(); }
	shared_ptr(T* p) : ptr(p), count(new size_t(1)) {}
	explicit shared_ptr(const weak_ptr<T>& w) : ptr(w.ptr), count(w.count) { incref(); }
	
	shared_ptr<T>& operator=(const shared_ptr<T>& o) {
	    if (ptr == o.ptr) return *this;
	    decref();
	    ptr = o.ptr;
	    count = o.count;
	    incref();
	    return *this;
	}
	
	T* get() { return ptr; }
	T* operator->() { return ptr; }
	T& operator*()  { return *ptr; }

	const T* get() const { return ptr; }
	const T* operator->() const { return ptr; }
	const T& operator*() const { return *ptr; }

	bool operator==(const shared_ptr<T>& o) const { return ptr == o.ptr; }
	bool operator!=(const shared_ptr<T>& o) const { return ptr != o.ptr; }
	bool operator<(const shared_ptr<T>& o) const { return ptr < o.ptr; }	

	size_t refcount() const { return *count; }
};

template <class T>
class weak_ptr {
	T* ptr;
	size_t* count;
	
	friend class shared_ptr<T>;
	
    public:
	
	weak_ptr() : ptr(0), count(shared_ptr<T>::nil()) {}
	explicit weak_ptr( const shared_ptr<T>& s) : ptr(s.ptr), count(s.count) {}
	
	shared_ptr<T> lock() const { return shared_ptr<T>(*this); }

	
	T* get() { return ptr; }
	T* operator->() { return ptr; }
	T& operator*()  { return *ptr; }

	const T* get() const { return ptr; }
	const T* operator->() const { return ptr; }
	const T& operator*() const { return *ptr; }

	bool operator==(const shared_ptr<T>& o) const { return ptr == o.ptr; }
	bool operator!=(const shared_ptr<T>& o) const { return ptr != o.ptr; }
	bool operator<(const shared_ptr<T>& o) const { return ptr < o.ptr; }	
	
	size_t refcount() const { return *count; }
	
};

} // namespace detail
} // namespace push
#endif
