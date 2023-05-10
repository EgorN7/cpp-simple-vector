#pragma once

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <iterator>
#include <stdexcept>

#include "array_ptr.h"


class ReserveProxyObj {
public:
    ReserveProxyObj(const size_t capacity_to_reserve)
        :reverse_assistant_(capacity_to_reserve)
    {
    }

    size_t GetSize() {
        return reverse_assistant_;
    }

private:
    size_t reverse_assistant_;

};

ReserveProxyObj Reserve(const size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    SimpleVector(ReserveProxyObj obj) {
        Reserve(obj.GetSize());
    }

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : SimpleVector(size, Type{})
    {
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : items_(size)
        , size_(size)
        , capacity_(size)
    {
        std::fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : items_(init.size())
        , size_(init.size())
        , capacity_(init.size())
    {
        std::copy(init.begin(), init.end(), begin());
    }

    SimpleVector(const SimpleVector& other)
        : items_(other.size_)
        , size_(other.size_)
        , capacity_(other.size_)
    {
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector(SimpleVector&& other)
        : items_(std::move(other.items_))
        , size_(std::exchange(other.size_, 0))
        , capacity_(std::exchange(other.capacity_, 0))
    {
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector swap_vector(rhs);
            swap(swap_vector);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        if (this != &rhs) {
            swap(rhs);
        }
        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) { throw std::out_of_range("out_of_range"); }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) { throw std::out_of_range("out_of_range"); }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size == size_) return;
        if (new_size < capacity_) {
            for (size_t i = size_; i < new_size; ++i) { // если new_size <= size_ то сразу не выполниться условия и не произайдёт ни 1 итеррации цикла даже в случае когда новый размер равен 0
                items_[i] = std::move(Type{});
            }
        }
        else {
            ArrayPtr<Type> new_arr_for_vector(new_size);
            std::move(begin(), end(), new_arr_for_vector.Get());
            items_.swap(new_arr_for_vector);
            for (size_t i = size_; i < new_size; ++i) {
                items_[i] = std::move(Type{});
            }
            capacity_ = new_size;
        }
        size_ = new_size;
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return Iterator{ items_.Get() };
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return Iterator{ items_.Get() + size_ };
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return ConstIterator{ items_.Get() };
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return ConstIterator{ items_.Get() + size_ };
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return ConstIterator{ items_.Get() };
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return ConstIterator{ items_.Get() + size_ };
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& value) {
        if (size_ < capacity_) {
            items_[size_] = value;
        }
        else {
            size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            ArrayPtr<Type> new_arr_for_vector(new Type[new_capacity]);
            std::copy(begin(), end(), new_arr_for_vector.Get());
            items_.swap(new_arr_for_vector);
            items_[size_] = value;
            capacity_ = new_capacity;
        }
        ++size_;
    }

    void PushBack(Type&& value) {
        if (size_ < capacity_) {
            items_[size_] = std::move(value);
        }
        else {
            size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            ArrayPtr<Type> new_arr_for_vector(new Type[new_capacity]);
            std::move(begin(), end(), new_arr_for_vector.Get());
            items_.swap(new_arr_for_vector);
            items_[size_] = std::move(value);
            capacity_ = new_capacity;
        }
        ++size_;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= cbegin() && pos <= cend());
        auto pos_element = std::distance(cbegin(), pos);
        if (size_ < capacity_) {
            std::copy_backward(pos, cend(), &items_[size_ + 1]);
            items_[pos_element] = value;
        }
        else {
            size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            ArrayPtr<Type> new_arr_for_vector(new Type[new_capacity]);
            std::copy(cbegin(), pos, new_arr_for_vector.Get());
            std::copy_backward(pos, cend(), &new_arr_for_vector[size_ + 1]);
            new_arr_for_vector[pos_element] = value;
            items_.swap(new_arr_for_vector);
            capacity_ = new_capacity;
        }
        ++size_;
        return  Iterator{ &items_[pos_element] };
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= cbegin() && pos <= cend());

        auto no_const_pos = const_cast<Iterator>(pos);
        auto pos_element = std::distance(begin(), no_const_pos);
        if (size_ < capacity_) {
            std::move_backward(no_const_pos, end(), &items_[size_ + 1]);
            items_[pos_element] = std::move(value);
        }
        else {
            size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            ArrayPtr<Type> new_arr_for_vector(new Type[new_capacity]);
            std::move(begin(), no_const_pos, new_arr_for_vector.Get());
            std::move_backward(no_const_pos, end(), &new_arr_for_vector[size_ + 1]);
            new_arr_for_vector[pos_element] = std::move(value);
            items_.swap(new_arr_for_vector);
            capacity_ = new_capacity;
        }
        ++size_;
        return  Iterator{ &items_[pos_element] };
    }


    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(!IsEmpty());
        --size_;// Напишите тело самостоятельно
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= cbegin() && pos < cend());
        Iterator start_copy_pos = const_cast<Iterator>(pos);
        auto pos_element = std::distance(begin(), start_copy_pos);
        std::move(++start_copy_pos, end(), &items_[pos_element]);
        --size_;
        return &items_[pos_element];
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            SimpleVector<Type> tmp_items(new_capacity);
            std::move(begin(), end(), tmp_items.begin());
            tmp_items.size_ = size_;
            swap(tmp_items);
        }
    }

private:
    ArrayPtr<Type> items_;

    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize()) { return false; }
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}