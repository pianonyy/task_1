#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <inttypes.h>
#include <cmath>
#include <vector>
#include <cassert>
#include <iostream>

template <size_t DIM, typename T> struct vec {
    vec() { for (size_t i=DIM; i--; data_[i] = T()); }
          T& operator[](const size_t i)       { assert(i<DIM); return data_[i]; }
    const T& operator[](const size_t i) const { assert(i<DIM); return data_[i]; }
    vec(const vec& v)
    {
        *this = v;
    }
    vec& operator=(const vec& v)
    {
        for (size_t i=DIM; i--; data_[i] = v[i]);
        return *this;
    }
    T length() const
    {
        T l{};
        for (size_t i=DIM; i--; l += data_[i] * data_[i]);
        return sqrt(l);
    }
private:
    T data_[DIM];
};





template <typename T> struct vec<3,T> {
    const size_t DIM = 3;
    vec() : x(T()), y(T()), z(T()) {}
    vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
    vec(const vec& v) : x(v.x), y(v.y), z(v.z) { }
          T& operator[](const size_t i)       { assert(i<3); return i<=0 ? x : (1==i ? y : z); }
    const T& operator[](const size_t i) const { assert(i<3); return i<=0 ? x : (1==i ? y : z); }
    float norm() { return std::sqrt(x*x+y*y+z*z); }
    vec<3,T> & normalize(T l=1) { *this = (*this)*(l/norm()); return *this; }
    T x,y,z;
    T length() const
    {
        T l = x * x + y * y + z * z;
        return sqrt(l);
    }
    vec& operator=(const vec& v)
    {
        x = v.x, y = v.y, z = v.z;
        return *this;
    }

};

typedef vec<3, float> Vec3f;

template <typename T> struct vec<4,T> {
    vec() : x(T()), y(T()), z(T()), w(T()) {}
    vec(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) {}
          T& operator[](const size_t i)       { assert(i<4); return i<=0 ? x : (1==i ? y : (2==i ? z : w)); }
    const T& operator[](const size_t i) const { assert(i<4); return i<=0 ? x : (1==i ? y : (2==i ? z : w)); }
    T x,y,z,w;
};
typedef vec<4, float> Vec4f;

template<size_t DIM,typename T> T operator*(const vec<DIM,T>& lhs, const vec<DIM,T>& rhs) {
    T ret = T();
    for (size_t i=DIM; i--; ret+=lhs[i]*rhs[i]);
    return ret;
}

template<size_t DIM,typename T>vec<DIM,T> operator+(vec<DIM,T> lhs, const vec<DIM,T>& rhs) {
    for (size_t i=DIM; i--; lhs[i]+=rhs[i]);
    return lhs;
}

template<size_t DIM,typename T>vec<DIM,T> operator-(vec<DIM,T> lhs, const vec<DIM,T>& rhs) {
    for (size_t i=DIM; i--; lhs[i]-=rhs[i]);
    return lhs;
}

template<size_t DIM,typename T,typename U> vec<DIM,T> operator*(const vec<DIM,T> &lhs, const U& rhs) {
    vec<DIM,T> ret;
    for (size_t i=DIM; i--; ret[i]=lhs[i]*rhs);
    return ret;
}

template<size_t DIM,typename T> vec<DIM,T> operator-(const vec<DIM,T> &lhs) {
    return lhs*T(-1);
}

template <typename T> vec<3,T> cross(vec<3,T> v1, vec<3,T> v2) {
    return vec<3,T>(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
}

template <size_t DIM, typename T> std::ostream& operator<<(std::ostream& out, const vec<DIM,T>& v) {
    for(unsigned int i=0; i<DIM; i++) out << v[i] << " " ;
    return out ;
}
#endif //__GEOMETRY_H__
