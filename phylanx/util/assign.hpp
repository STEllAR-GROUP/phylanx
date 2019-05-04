//  Copyright (c) 2019 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef util_assign_hpp
#define util_assign_hpp
template<typename T>
class assign_vector
{
    T& data;
public:
    assign_vector(T& data_) : data(data_) {}
    ~assign_vector() {}

    template<typename V>
    void operator=(const V& value) {
        if(data.is_ref())
            data = value;
        else
            data.vector() = value;
    }
};

template<typename T>
class assign_matrix
{
    T& data;
public:
    assign_matrix(T& data_) : data(data_) {}
    ~assign_matrix() {}

    template<typename V>
    void operator=(const V& value) {
        if(data.is_ref())
            data = value;
        else
            data.matrix() = value;
    }
};

template<typename T>
class assign_tensor
{
    T& data;
public:
    assign_tensor(T& data_) : data(data_) {}
    ~assign_tensor() {}

    template<typename V>
    void operator=(const V& value) {
        if(data.is_ref())
            data = value;
        else
            data.tensor() = value;
    }
};
#endif
