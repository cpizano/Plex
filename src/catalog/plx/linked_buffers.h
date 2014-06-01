//#~def plx::LinkedBuffers
///////////////////////////////////////////////////////////////////////////////
// plx::LinkedBuffers
//
namespace plx {
class LinkedBuffers {
  struct Item {
    size_t size;
    std::unique_ptr<unsigned char[]> data;
    Item(size_t size)
        : size(size), data(new unsigned char[size]) {
    }

    Item(const Item& other) 
        : size(other.size), data(new unsigned char[size]) {
      memcpy(data.get(), other.data.get(), size);
    }

    Item(Item&& other) = delete;
  };

  typedef std::list<Item> BList;

  BList buffers_;
  BList::iterator loop_it_;

public:
  LinkedBuffers() {
  }

  LinkedBuffers(const LinkedBuffers& other)
      : buffers_(other.buffers_) {
  }

  LinkedBuffers(LinkedBuffers&& other) {
    buffers_.swap(other.buffers_);
    std::swap(loop_it_, other.loop_it_);
  }

  plx::Range<unsigned char> new_buffer(size_t size_bytes) {
    buffers_.emplace_back(size_bytes);
    auto start = &(buffers_.back().data)[0];
    return plx::Range<unsigned char>(start, start + size_bytes);
  }

  void remove_last_buffer() {
    buffers_.pop_back();
  }

  void first() {
    loop_it_ = begin(buffers_);
  }

  void next() {
    ++loop_it_;
  }

  bool done() {
    return (loop_it_ == end(buffers_));
  }

  plx::Range<unsigned char> get() {
    auto start = &(loop_it_->data)[0];
    return plx::Range<unsigned char>(start, start + loop_it_->size);
  }
};
}
