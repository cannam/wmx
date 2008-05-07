/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#ifndef _LISTMACRO_H_
#define _LISTMACRO_H_

#include <assert.h>

// assumes efficient realloc()

#define declarePList(List, T) \
  typedef T *T##_pointer; \
  declareList(List, T##_pointer);

#define implementPList(List, T) \
  typedef T *T##_pointer; \
  implementList(List, T##_pointer);

#define declareList(List, T) \
class List { \
public: \
    List(); \
    ~List(); \
\
    long count() const { return m_count; } \
    T &item(long index) const { \
        if (!range_check(index, "item")) return m_items[0]; \
	return m_items[index]; \
    } \
    T *array(long index, long) { \
	return m_items + index; \
    } \
\
    void append(const T &); \
    void remove(long index); \
    void remove_all(); \
    void move_to_start(long index); \
    void move_to_end(long index); \
\
private: \
    bool range_check(long, const char *) const; \
    T *m_items; \
    long m_count; \
};

#define implementList(List, T) \
\
List::List() : m_items(0), m_count(0) { } \
\
List::~List() { remove_all(); } \
\
void *operator new(size_t, void *m, List *) { return m; } \
\
void List::append(const T &item) { \
    if (m_items) { \
        m_items = (T *)realloc(m_items, (m_count + 1) * sizeof(T)); \
    } else { \
	m_items = (T *)malloc(sizeof(T)); \
    } \
    assert(m_items); \
    new (&m_items[m_count++], this) T(item); \
} \
\
void List::remove(long index) { \
    if (!range_check(index, "remove")) return; \
    m_items[index].T::~T(); \
    memmove(m_items+index, m_items+index+1, (m_count-index-1) * sizeof(T)); \
    if (m_count == 1) { \
	free((void *)m_items); m_items = 0; \
    } else { \
        m_items = (T *)realloc(m_items, (m_count - 1) * sizeof(T)); \
    } \
    --m_count; \
} \
\
void List::remove_all() { \
    while (m_count > 0) remove(0); \
} \
\
void List::move_to_start(long index) { \
    if (!range_check(index, "move_to_start")) return;  \
    T temp(m_items[index]); \
    m_items[index].T::~T(); \
    if (index > 0) memmove(m_items+1, m_items, index * sizeof(T)); \
    new (&m_items[0], this) T(temp); \
} \
\
void List::move_to_end(long index) { \
    if (!range_check(index, "move_to_end")) return; \
    T temp(m_items[index]); \
    m_items[index].T::~T(); \
    if (index < m_count-1) memmove(m_items+index, m_items+index+1, \
				   (m_count-index-1) * sizeof(T)); \
    new (&m_items[m_count-1], this) T(temp); \
}\
\
bool List::range_check(long index, const char *fn) const { \
    if (index >= 0 && index < m_count) return true; \
    fprintf(stderr, "wmx: ERROR: Index %ld out of range for %ld-valued list in %s::%s\n", index, m_count, #List, fn); \
    return false; \
}

#endif
