// Pointer.h — Intrusive smart pointer and reference counting infrastructure
//
// Provides Pointer<T>, an intrusive reference-counted smart pointer built on
// top of boost::intrusive_ptr. Objects managed by Pointer<T> must inherit from
// RefCounted, which embeds the reference counter directly in the object (no
// separate control block), giving cache-friendly single-allocation semantics.

#pragma once

#include <boost/smart_ptr/intrusive_ptr.hpp>

namespace Interpreter {

//--------------------------------------------------
// Pointers
//--------------------------------------------------
template <typename T>
using IntrusivePointer = boost::intrusive_ptr<T>;

// Pointer<T> — extends boost::intrusive_ptr with a safe downcast helper
// and equality comparison for use in hash containers.
template <typename T>
class Pointer : public IntrusivePointer<T> {
public:
    using IntrusivePointer<T>::IntrusivePointer;

    // Attempts a dynamic_cast to Pointer<U>. Returns null Pointer on failure.
    template <typename U>
    Pointer<U> as() const {
        U* ptr = dynamic_cast<U*>(this->get());
        if (ptr != nullptr) {
            return Pointer<U>(ptr);
        }
        return Pointer<U>();
    }

    // Identity-based equality for use in hash maps and hash sets.
    bool operator==(const Pointer<T>& rhs) {
        return this->get() == rhs.get();
    }
};

// RefCounted — base class for all objects managed by Pointer<T>.
// Embeds a non-atomic reference counter (single-threaded use only).
// Copy and move are deleted to prevent accidental counter duplication.
struct RefCounted {
    RefCounted() = default;
    virtual ~RefCounted() = default;
    RefCounted(const RefCounted&) = delete;
    RefCounted(RefCounted&&) = delete;

    // Called by boost::intrusive_ptr on copy/assignment.
    friend void intrusive_ptr_add_ref(RefCounted* p) {
        p->_counter += 1;
    }

    // Called by boost::intrusive_ptr on destruction/reset.
    // Deletes the object when the last reference is released.
    friend void intrusive_ptr_release(RefCounted* p) {
        if (p->_counter > 1) {
            p->_counter--;
            return;
        }
        delete p;
    }

    int _counter = 0;
};

}  // namespace Interpreter
