#ifndef _DATA_SET
#define _DATA_SET

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#define FUNC(cond)                                                             \
    bool {                                                                     \
        if (cond)                                                              \
            return true;                                                       \
        else                                                                   \
            return false;                                                      \
    }

namespace query {
enum MutationMode {
    IMMUTABLE, // leave the old container alone and create new ones
    IN_PLACE   // modify the container in place
};

// T is usually an STL container.
template <typename T, typename TPtr = std::shared_ptr<T>> class DataSet {
  public:
    typedef T underlying_container_type;
    DataSet(TPtr);

    TPtr Get();

    unsigned int Count();
    DataSet<T> Where(std::function<bool(typename T::value_type)> pred);

    // TODO: There is probably some way to get rid of the hard-coded
    // vector and instead use the same container type as T.
    template <typename S>
    DataSet<std::vector<S>>
    Select(std::function<S(typename T::value_type)> sel);

    // Map is an alias of Select.
    template <typename S>
    DataSet<std::vector<S>> Map(std::function<S(typename T::value_type)> sel) {
        return Select<S>(sel);
    }

  private:
    TPtr container_;
};

template <typename T, typename TPtr>
DataSet<T, TPtr>::DataSet(TPtr container) : container_(container) {}

template <typename T, typename TPtr> TPtr DataSet<T, TPtr>::Get() {
    return container_;
}

template <typename T, typename TPtr> unsigned int DataSet<T, TPtr>::Count() {
    return container_->size();
}

template <typename T, typename TPtr>
DataSet<T>
DataSet<T, TPtr>::Where(std::function<bool(typename T::value_type)> pred) {
    std::shared_ptr<T> new_container(new T());
    for (typename T::const_iterator i = container_->begin();
         i != container_->end(); ++i) {
        if (pred(*i)) {
            new_container->push_back(*i);
        }
    }
    return DataSet<T>(new_container);
}

template <typename T, typename TPtr>
template <typename S>
DataSet<std::vector<S>>
DataSet<T, TPtr>::Select(std::function<S(typename T::value_type)> sel) {
    std::shared_ptr<vector<S>> new_container(new vector<S>());
    for (typename T::const_iterator i = container_->begin();
         i != container_->end(); ++i) {
        new_container->push_back(sel(*i));
    }
    return DataSet<std::vector<S>>(new_container);
}

template <typename T> DataSet<T> From(std::shared_ptr<T> source) {
    return DataSet<T>(source);
}

template <typename T> DataSet<T, T *> From(T *source) {
    return DataSet<T, T *>(source);
}

template <typename T> class Inserter {
  public:
    Inserter(const T &data_set) : data_set_(data_set) {}

    template <typename S> S Into(S target, MutationMode mode = IMMUTABLE);

  private:
    T data_set_;
};

template <typename T>
template <typename S>
S Inserter<T>::Into(S target, MutationMode mode) {
    std::shared_ptr<typename S::underlying_container_type> target_container;
    std::shared_ptr<typename T::underlying_container_type> source_container =
        data_set_.Get();
    if (mode == IMMUTABLE) {
        target_container.reset(new typename S::underlying_container_type);
        *target_container = *(target.Get());
    } else {
        target_container = target.Get();
    }
    typename T::underlying_container_type::const_iterator it;
    for (it = source_container->begin(); it != source_container->end(); ++it) {
        target_container->push_back(*it);
    }
    return From(target_container);
}

template <typename C> Inserter<DataSet<C>> Insert(DataSet<C> data_set) {
    return Inserter<DataSet<C>>(data_set);
}

} // namespace query
#define LX_ATTR(A) &_1->*&(A)
#endif