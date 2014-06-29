#pragma once

template <typename T>
class infordered
{
public:
    
    enum Type {MIN, VAL, MAX};

    Type type;
    T val;

public:

    infordered()
        : type(VAL)
        , val() {};
    
    infordered(T val)
        : type(VAL)
        , val(val) {};

    
private:
    
    infordered(Type type)
        : type(type)
        , val() {};


public:
    
    infordered<T>& operator= (const T& v)
    {
        type = VAL;
        val = v;

        return *this;
    }
    
    static infordered<T> min() { return infordered(MIN); }
    static infordered<T> max() { return infordered(MAX); }
    
};


template<typename T> bool operator < (const infordered<T>& a, const T& b)
{
    switch (a.type) {
    case infordered<T>::MIN: return true;
    case infordered<T>::VAL: return a.val < b;
    case infordered<T>::MAX: return false;
    }

    return true;
}

template<typename T> bool operator <= (const infordered<T>& a, const T& b)
{
    switch (a.type) {
    case infordered<T>::MIN: return true;
    case infordered<T>::VAL: return a.val <= b;
    case infordered<T>::MAX: return false;
    }

    return true;
}

template<typename T> bool operator == (const infordered<T>& a, const T& b)
{
    switch (a.type) {
    case infordered<T>::MIN: return false;
    case infordered<T>::VAL: return a.val == b;
    case infordered<T>::MAX: return false;
    }

    return true;
}
