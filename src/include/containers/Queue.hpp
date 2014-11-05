/**
 * Cyclic-array-based FIFO queue class. Is NOT thread-safe, the caller should ensure locks are used
 * Relies on the type having the assignment operator
 * Be careful using the pop method without arguments - it is both slower and doesn't check element availability
 */
template <class T>
class Queue {
public:
	/**
	 * Constructor
	 */
	Queue() :
			_head(-1),
			_tail(-1),
			_size(-1),
			_count(-1),
			_storage(nullptr),
			_tmp(-1),
			_res(true)
	{ }
	/**
	 * Destructor. Frees all the allocated memory
	 */
	~Queue() {
		if (_storage != nullptr) {
			delete[](_storage);
		}
	}

	/**
	 * Init procedure that allocates the memory. Should be called before using the Queue
	 *
	 * @param size specifies the buffer size
	 * @return if the allocation was successful or not
	 */
	bool init(ssize_t size) {
		if (_storage != nullptr || size < 1) {
			return false;
		}
		// Ensures we are able to store _size values and differentiate between full and empty buffer
		_storage = new T[size + 1];
		if (_storage == nullptr) {
			return false;
		}
		_size = size + 1;
		_head = 0;
		_tail = 0;
		_count = 0;
		return true;
	}

	/**
	 * Adds a new element at the end of the queue
	 *
	 * @param value value to add to the queue
	 * @param force specifies if the the new sample should be forced into buffer, discarding the oldest sample
	 * @return if forced is true, then returns if the add executed cleanly (without forcing) or not
	 * if forced is false, then returns if the add succeeded or not
	 */
	bool add(const T &value, bool force=false) {
		if (_storage == nullptr) {
			return false;
		}
		_res = true;
		_tmp = _head + 1;
		// Warp around
		if (_tmp >= _size) {
			_tmp = 0;
		}
		if (_tmp == _tail) {
			// Buffer is full
			if (force) {
				_res = false;
				// Advance tail to discard the oldest sample and let the method continue
				_tail += 1;
				// Warp around
				if (_tail >= _size) {
					_tail = 0;
				}
			}
			else {
				return false;
			}
		}
		_storage[_head] = value;
		_head = _tmp;
		++_count;
		return _res;
	}

	/**
	 * Pops the oldest added element and removes it from the queue
	 *
	 * @param value[out] the result will be stored in this parameter
	 * @return true if operation was successful, false if the queue was empty
	 */
	bool pop(T &value) {
		// No need to check _storage, as _tail is inited to the same value as head
		if (_tail == _head) {
			return false;
		}
		value = _storage[_tail];
		_tail += 1;
		// Warp around
		if (_tail >= _size) {
			_tail = 0;
		}
		--_count;
		return true;
	}

	/**
	 * Convenience method. Doesn't check for element availability. Is ineffective
	 *
	 * @return popped value if it is available, uninitialized value otherwise
	 */
	T pop() {
		T res;
		pop(res);
		return res;
	}

	/**
	 * Empties the queue
	 */
	void do_empty(){
		_tail = _head;
		_count = 0;
	}
	/**
	 * Checks the maximum number of values that can be stored in the queue
	 *
	 * @return maximum total number of elements in the queue, -1 if the queue isn't allocated
	 */
	ssize_t get_size() {
		if (_size > 0) {
			return _size-1;
		}
		else {
			return -1;
		}
	}

	/**
	 * Checks the number of values currently stored in the queue
	 *
	 * @return the number of values stored
	 */
	ssize_t get_value_count() {
		return _count;
		/* if (_head >= _tail) {
			return _head - _tail;
		}
		return (_head + _size - _tail); */
	}

	/**
	 * Returns the element with specified index without removing it from the queue
	 *
	 * @param index zero-based index of the element
	 * @param value[out] the result will be stored in this parameter
	 * @return true on success, false otherwise
	 */
	bool peek(ssize_t index, T &value) {
		if (index >= _count) {
			return false;
		}
		_tmp = _tail + index;
		if (_tmp >= _size) {
			_tmp -= _size;
		}
		value = _storage[_tmp];
		return true;
	}

	/**
	 * Checks if the queue is empty
	 *
	 * @return true if the queue is empty, false otherwise
	 */
	bool is_empty() {
		return (_head == _tail);
	}

	/**
	 * Checks if the queue is full
	 *
	 * @return true if the queue is full or unallocated, false otherwise
	 */
	bool is_full() {
		if (_storage != nullptr) {
			return ((_size -1) == _count);
		}
		return true;
	}
private:
	ssize_t _head, _tail, _size;
	// Optimize calls to peek
	ssize_t _count;
	T *_storage;
	// Reduce memory allocations
	ssize_t _tmp;
	bool _res;
};
