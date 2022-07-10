#include "Common.h"

// Single-level array
template <int BITS>
class TCMalloc_PageMap1 {
private:
	static const int LENGTH = 1 << BITS;

	size_t* array_;

public:
	typedef uintptr_t Number;

	explicit TCMalloc_PageMap1() {
		array_ = (size_t*)malloc(sizeof(size_t) << BITS);
		memset(array_, 0, sizeof(size_t) << BITS);
	}

	// Ensure that the map contains initialized entries "x .. x+n-1".
	// Returns true if successful, false if we could not allocate memory.
	bool Ensure(Number x, size_t n) {
		// Nothing to do since flat array was allocated at start.  All
		// that's left is to check for overflow (that is, we don't want to
		// ensure a number y where array_[y] would be an out-of-bounds
		// access).
		return n <= LENGTH - x;   // an overflow-free way to do "x + n <= LENGTH"
	}

	void PreallocateMoreMemory() {}

	// Return the current value for KEY.  Returns NULL if not yet set,
	// or if k is out of range.
	size_t get(Number k) const {
		if ((k >> BITS) > 0) {
			return NULL;
		}
		return array_[k];
	}

	// REQUIRES "k" is in range "[0,2^BITS-1]".
	// REQUIRES "k" has been ensured before.
	//
	// Sets the value 'v' for key 'k'.
	void set(Number k, size_t v) {
		array_[k] = v;
	}

	// Return the first non-NULL pointer found in this map for
	// a page number >= k.  Returns NULL if no such number is found.
	size_t Next(Number k) const {
		while (k < (1 << BITS)) {
			if (array_[k] != NULL) return array_[k];
			k++;
		}
		return NULL;
	}
};

template <int BITS>
class TCMalloc_PageMap2 {
private:
	// Put 32 entries in the root and (2^BITS)/32 entries in each leaf.
	static const int ROOT_BITS = 5;
	static const int ROOT_LENGTH = 1 << ROOT_BITS;

	static const int LEAF_BITS = BITS - ROOT_BITS;
	static const int LEAF_LENGTH = 1 << LEAF_BITS;

	// Leaf node
	struct Leaf {
		Span* values[LEAF_LENGTH];
	};

	Leaf* root_[ROOT_LENGTH];             // Pointers to 32 child nodes

public:
	typedef uintptr_t Number;

	explicit TCMalloc_PageMap2() {
		memset(root_, 0, sizeof(root_));
		PreallocateMoreMemory();
	}

	Span* get(Number k) const {
		const Number i1 = k >> LEAF_BITS;
		const Number i2 = k & (LEAF_LENGTH - 1);
		if ((k >> BITS) > 0 || root_[i1] == NULL) {
			return NULL;
		}
		return root_[i1]->values[i2];
	}

	void set(Number k, Span* v) {
		const Number i1 = k >> LEAF_BITS;
		const Number i2 = k & (LEAF_LENGTH - 1);
		assert(i1 < ROOT_LENGTH);
		root_[i1]->values[i2] = v;
	}

	Span*& operator[](Number k)
	{
		const Number i1 = k >> LEAF_BITS;
		const Number i2 = k & (LEAF_LENGTH - 1);
		assert(i1 < ROOT_LENGTH);
		return root_[i1]->values[i2];
	}

	void erase(Number k)
	{
		const Number i1 = k >> LEAF_BITS;
		const Number i2 = k & (LEAF_LENGTH - 1);
		assert(i1 < ROOT_LENGTH);
		root_[i1]->values[i2] = nullptr;
	}

	bool Ensure(Number start, size_t n) {
		for (Number key = start; key <= start + n - 1;) {
			const Number i1 = key >> LEAF_BITS;

			// Check for overflow
			if (i1 >= ROOT_LENGTH)
				return false;

			// Make 2nd level node if necessary
			if (root_[i1] == NULL) {
				Leaf* leaf = new Leaf;
				if (leaf == NULL) return false;
				memset(leaf, 0, sizeof(*leaf));
				root_[i1] = leaf;
			}

			// Advance key past whatever is covered by this leaf node
			key = ((key >> LEAF_BITS) + 1) << LEAF_BITS;
		}
		return true;
	}

	void PreallocateMoreMemory() {
		// Allocate enough to keep track of all possible pages
		Ensure(0, 1 << BITS);
	}

	void* Next(Number k) const {
		while (k < (1 << BITS)) {
			const Number i1 = k >> LEAF_BITS;
			Leaf* leaf = root_[i1];
			if (leaf != NULL) {
				// Scan forward in leaf
				for (Number i2 = k & (LEAF_LENGTH - 1); i2 < LEAF_LENGTH; i2++) {
					if (leaf->values[i2] != NULL) {
						return leaf->values[i2];
					}
				}
			}
			// Skip to next top-level entry
			k = (i1 + 1) << LEAF_BITS;
		}
		return NULL;
	}
};

