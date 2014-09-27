//#~def plx::ScopedReadLock
//#~def plx::ScopedWriteLock
//#~def plx::ScopedReadLock
///////////////////////////////////////////////////////////////////////////////
// ReaderWriterLock and its scoped unlockers.
//
namespace plx {
class ScopedWriteLock {
  SRWLOCK* lock_;
  explicit ScopedWriteLock(SRWLOCK* lock) : lock_(lock) {}
  friend class ReaderWriterLock;

public:
  ScopedWriteLock() = delete;
  ScopedWriteLock(const ScopedWriteLock&) = delete;
  // void* operator new(std::size_t) =delete

  ~ScopedWriteLock() {
    ::ReleaseSRWLockExclusive(lock_);
  }
};

class ScopedReadLock {
  SRWLOCK* lock_;
  explicit ScopedReadLock(SRWLOCK* lock) : lock_(lock) {}
  friend class ReaderWriterLock;

public:
  ScopedReadLock() = delete;
  ScopedReadLock(const ScopedReadLock&) = delete;
  // void* operator new(std::size_t) =delete

  ~ScopedReadLock() {
    ::ReleaseSRWLockShared(lock_);
  }
};

// The SRW lock is implemented using a single pointer-sized atomic variable
// which can take on a number of different states, depending on the values
// of the low bits. The number of state transitions is pretty high, here are
// some common ones:
// - initial lock state: (0, ControlBits:0) -- An SRW lock starts with all
//   bits set to 0.
// - shared state: (ShareCount: n, ControlBits: 1) -- When there is no conflicting
//   exclusive acquire and the lock is held shared, the share count is stored
//   directly in the lock variable.
// - exclusive state: (ShareCount: 0, ControlBits: 1) -- When there is no
//   conflicting shared acquire or exclusive acquire, the lock has a low bit set
//   and nothing else.
// - contended case 1 : (WaitPtr:ptr, ControlBits: 3) -- When there is a conflict,
//   the threads that are waiting for the lock form a queue using data allocated
//   on the waiting threads' stacks. The lock variable stores a pointer to the
//   tail of the queue instead of a share count.

class ReaderWriterLock {
  char cache_line_pad_[64 - sizeof(SRWLOCK)];
  SRWLOCK lock_;

public:
  ReaderWriterLock() {
    ::InitializeSRWLock(&lock_);
  }

  ReaderWriterLock(ReaderWriterLock&) = delete;
  ReaderWriterLock& operator=(const ReaderWriterLock&) = delete;

  // Acquiring an exclusive lock when you don't know the initial state is a
  // single write to the lock word, to set the low bit (LOCK BTS instruction)
  // If you succeeded you can proceed into the locked region with no further
  // operations.

  ScopedWriteLock write_lock() {
    ::AcquireSRWLockExclusive(&lock_);
    return ScopedWriteLock(&lock_);
  }

  // Trying to acquire a shared lock is a more involved operation: You need to
  // first read the initial value of the lock variable to determine the old
  // share count, increment the share count you read, and then write the updated
  // value back conditionally with the LOCK CMPXCHG instruction. 

  ScopedReadLock read_lock() {
    ::AcquireSRWLockShared(&lock_);
    return ScopedReadLock(&lock_);
  }
};
}
