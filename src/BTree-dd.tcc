/*
 * BTree-dd.tcc: lightweight C++ template for handling B-Trees
 * Copyright 2024-2025 Daniel Dwek
 *
 * This file is part of TangorineBA.
 *
 *  TangorineBA is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TangorineBA is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TangorineBA.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _BTREE_DD_TCC
#define _BTREE_DD_TCC	1
#include <iostream>		// if you do not want BTree<T>::draw () method,
#include <iomanip>		// do not include these files and compiling
				// will be faster

/*
 *
 * B-Trees are also known as "binary trees", since you have two branches or
 * children for each parent branch (i.e., ".pbranch"), and additionally, you
 * can store your data on the corresponding structure member.
 *
 * Leaves are unique and immutable IDs so far, but may or may not be removed
 * on future releases.
 *
 * Comparing BTrees with doubly-linked lists is encouraged. Due to the nature
 * of BTrees algos, insertions are slightly slower than lists. However accessing
 * to branches is immediate rather than lists, where the time spent walking
 * through such lists is even bigger and bigger as the amount of items to save
 * on them is increasing.
 *
 * Whether using either btrees or lists depends on what kind of modeling and
 * data representation you need for every single project, so whichever you want
 * to use is your call ;)
 *
 */
template<class T>
struct btree_st {
	struct btree_st<T> *pbranch;
	struct btree_st<T> *branch[2];
	int leaf;
	T data;
};

/*
 *
 * You shouldn't have any issues or compiler warnings about the template
 * declaration, specialization or instantiation syntax used below, it's
 * std=C++11 and above compatible. When in doubt, check your compiler version
 * before anything else
 *
 */
template<class T>
class BTree {
public:
	BTree<T> (T data);
	BTree<T> () = delete;
	BTree<T> (BTree<T>&) = delete;
	BTree<T> (BTree<T>&&) = delete;
	BTree<T>& operator= (BTree<T>&) = delete;
	~BTree<T> ();
	struct btree_st<T> *get_last_branch ();
	struct btree_st<T> *get_root ();

	void set_current (struct btree_st<T> *c);

	int *get_path (int leaf, int *out_level);
	int get_size ();

	void init (T data);
	// You can add a leaf to the new branch with just one single statement
	void add_branch (int leaf, T data);
	struct btree_st<T> *get_leaf (int leaf);

	// TODO: "Clear ()" should be declared as private for avoiding be
	// called from your source. Keep in mind that this method is called
	// by destructor, so be aware when invoking it explicitly!
	void clear ();
	void draw ();
private:
	struct btree_st<T> *get_next_branch (int *info);
	struct btree_st<T> *btree;
	struct btree_st<T> *current;
	struct btree_st<T> *last;
	int size;
};

template<class T>
BTree<T>::BTree (T data)
{
	init (data);
}

template<class T>
BTree<T>::~BTree ()
{
	clear ();
}

template<class T>
struct btree_st<T> *BTree<T>::get_last_branch ()
{
	return last;
}

template<class T>
struct btree_st<T> *BTree<T>::get_root ()
{
	return btree;
}

template<class T>
void BTree<T>::set_current (struct btree_st<T> *c)
{
	current = c;
}

template<class T>
struct btree_st<T> *BTree<T>::get_next_branch (int *info)
{
	int i, j = 0;
	int rlevel = 0;
	struct btree_st<T> *iter, *next;

	iter = current;
	if (!iter || !info)
		return nullptr;
	if (iter->branch[0]) {
		*info = 1;

		next = iter->branch[0];
		return next;
	} else {
newpbranch:
		*info = --rlevel;
		if (!iter->pbranch)
			return nullptr;

		for (i = 0; i < 2; i++) {
			if (iter == iter->pbranch->branch[i]) {
				j = i + 1;
				break;
			}
		}

		iter = iter->pbranch;
		if (j < 2) {
			rlevel++;
			*info = rlevel;
			next = iter->branch[j];
			return next;
		}

		goto newpbranch;
	}
}

template<class T>
int *BTree<T>::get_path (int leaf, int *out_level)
{
	int i, j, t, idx, *path = nullptr;

	for (i = 0, j = leaf; j; i++)
		j >>= 1;
	path = new int [i];
	for (t = leaf, idx = 0; idx < i - 1; idx++) {
		path[idx] = t & 0x1;
		t >>= 1;
	}

	*out_level = i - 1;
	return path;
}

template<class T>
int BTree<T>::get_size ()
{
	return this->size;
}

template<class T>
void BTree<T>::init (T data)
{
	btree = new struct btree_st<T>;
	btree->pbranch = nullptr;
	btree->branch[0] = nullptr;
	btree->branch[1] = nullptr;
	btree->leaf = 1;
	btree->data = data;
	size = 1;

	current = btree;
	last = btree;
}

template<class T>
void BTree<T>::add_branch (int leaf, T data)
{
	int i, level, *path = nullptr;

	path = get_path (leaf, &level);
	for (last = btree, i = level - 1; i > -1; i--) {
		if (!last->branch[path[i]]) {
			last->branch[path[i]] = new struct btree_st<T>;
			last->branch[path[i]]->pbranch = last;
		}
		last = last->branch[path[i]];
	}
	last->branch[0] = nullptr;
	last->branch[1] = nullptr;
	last->leaf = leaf;
	last->data = data;
	this->size++;
}

template<class T>
struct btree_st<T> *BTree<T>::get_leaf (int leaf)
{
	int i, level, *path = nullptr;

	if (!leaf)
		return btree;

	path = get_path (leaf, &level);
	for (last = btree, i = level - 1; i > -1; i--)
		last = last->branch[path[i]];

	return last;
}

template<class T>
void BTree<T>::clear ()
{
	int i;
	struct btree_st<T> *iter = nullptr;

	for (i = this->size; i > 0; i--) {
		iter = get_leaf (i);
		delete iter;
		iter = nullptr;
		this->size--;
	}
	btree = nullptr;
	current = nullptr;
	last = nullptr;
}

template<class T>
void BTree<T>::draw ()
{
	int i, rlevel = 0, lv = 0;
	struct btree_st<T> *copy;

	copy = current;
	while (current) {
		for (i = 0; i < rlevel + 1; i++)
			std::cout << ("     |");
		std::cout << std::endl;
		for (i = 0; i < rlevel; i++)
			std::cout << ("     |");
		if (current)
			std::cout << "     +---> " << current->leaf << " [" << current << "]" << std::endl;
		current = get_next_branch (&lv);
		rlevel += lv;
		if (rlevel < 0)
			break;
	}
	current = copy;
}
#endif
