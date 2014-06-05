/*
 * LightweightCuckooHashMap.h
 *
 *  Created on: Oct 17, 2013
 *      Author: Martin Wimmer
 *	   License: Boost Software License 1.0
 */

#ifndef LIGHTWEIGHTCUCKOOHASHMAP_H_
#define LIGHTWEIGHTCUCKOOHASHMAP_H_

#include <functional>
#include <random>
//#include <set>


namespace pheet {

template <typename Key, typename TT>
struct LightweightCuckooHashMapData {
	LightweightCuckooHashMapData()
	: hash_f(-1) {}

	int8_t hash_f;
	Key k;
	TT v;
};
/*
 * Lightweight sequential implementation of cuckoo hashing
 * Focus is on speed. Only supports lookups, nothing else, no support for duplicate insertion
 * Expects lightweight data
 * Used internally by some data structures
 */
template <class Pheet, typename Key, typename TT>
class LightweightCuckooHashMap {
public:
	typedef LightweightCuckooHashMapData<Key, TT> Data;

	LightweightCuckooHashMap()
	:data(new Data[256]), size(256), mask(size-1) {
		generate_hash_values();
	}

	~LightweightCuckooHashMap() {
		delete[] data;
	}

	void put(Key key, TT value) {
/*		auto it = verify.find(key);
		pheet_assert(it == verify.end());
		verify.insert(key);*/
		while(true) {
			size_t h = hash(0, key);
			Data* d = data + h;
			if(d->hash_f == -1) {
				pheet_assert(data[hash(1, key)].hash_f == -1 || data[hash(1, key)].k != key);
				d->k = key;
				d->v = value;
				d->hash_f = 0;
				return;
			}

			int8_t hf = 1;

			// 16 tries before giving up and rehashing
			for(int i = 0; i < 16; ++i) {
				h = hash(hf, key);
				d = data + h;
				if(d->hash_f == -1) {
					d->k = key;
					d->v = value;
					d->hash_f = hf;
					return;
				}
				else {
					pheet_assert(d->k != key);
					std::swap(d->k, key);
					std::swap(d->v, value);
					std::swap(d->hash_f, hf);
					// Switch to other hash function
					hf = (hf == 1)?0:1;
				}
			}

			rehash();
		}
	}

	bool find(Key const& key, TT& value) {
		size_t h = hash(0, key);
		Data d = data[h];
		if(d.hash_f != -1 && d.k == key) {
			data = d.v;
			return true;
		}
		h = hash(1, key);
		d = data[h];
		if(d.hash_f != -1 && d.k == key) {
			data = d.v;
			return true;
		}
		return false;
	}

	bool retrieve(Key const& key, TT& value) {
		size_t h = hash(0, key);
		Data& d = data[h];
		if(d.hash_f != -1 && d.k == key) {
/*			auto it = verify.find(key);
			pheet_assert(it != verify.end());
			verify.erase(it);*/
			value = d.v;
			d.hash_f = -1;
			return true;
		}
		h = hash(1, key);
		Data& d2 = data[h];
		if(d2.hash_f != -1 && d2.k == key) {
/*			auto it = verify.find(key);
			pheet_assert(it != verify.end());
			verify.erase(it);*/
			value = d2.v;
			d2.hash_f = -1;
			return true;
		}
/*		auto it = verify.find(key);
		pheet_assert(it == verify.end());*/
		return false;
	}

private:
	/*
	 * Cheap hash function we prefer to waste space than computing power
	 */
	size_t hash(int8_t type, Key const& key) {
		pheet_assert(type == 1 || type == 0);
		size_t k = std::hash<Key>()(key);
		size_t k2 = (k  % (mask + hash2[type][k % 19])) & mask;

		return k2 ^ hash1[type][k % 17] ^ hash2[type][k % 19];
	}

	void rehash() {
		// Create new hash functions and double size of array
		Data* old_d = data;
		size_t old_s = size;
		size = size << 1;
		mask = size - 1;
		data = new Data[size];
		for(size_t i = 0; i < size; ++i) {
			pheet_assert(data[i].hash_f == -1);
		}
		generate_hash_values();

		for(size_t i = 0; i < old_s; ++i) {
			Data& d = old_d[i];
			if(d.hash_f != -1) {
				put(d.k, d.v);
			}
		}

		delete[] old_d;
	}

	void generate_hash_values() {
		std::mt19937 mt(size);
		std::uniform_int_distribution<size_t> dist(0, mask);
		for(int i = 0; i < 17; ++i) {
			hash1[0][i] = dist(mt);
			hash1[1][i] = dist(mt);
		}
		for(int i = 0; i < 19; ++i) {
			hash2[0][i] = dist(mt);
			hash2[1][i] = dist(mt);
		}
	}

	Data* data;
	size_t size;
	size_t mask;

	size_t hash1[2][17];
	size_t hash2[2][19];
//	std::set<Key> verify;
};

} /* namespace pheet */
#endif /* LIGHTWEIGHTCUCKOOHASHMAP_H_ */
