/*
 * Copyright 2025 Daniel Dwek
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

	int *get_path (int leaf);

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
	struct btree_st<T> *pb;
	struct btree_st<T> *last;
};

template<class T>
BTree<T>::BTree (T data)
{
	btree = new struct btree_st<T>;
	btree->pbranch = nullptr;
	btree->leaf = 1;
	btree->data = data;
	current = btree;
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
int *BTree<T>::get_path (int leaf)
{
	int i, j, sz = 0, level = 0;
	int *pos = nullptr;

	for (j = 0; j < 0x20; j++) {
		if (leaf >= (1 << j) && leaf < (1 << (j + 1))) {
			level = j;
			break;
		}
	}

	sz = level;
	pos = new int [sz];
	for (i = 0; i < sz; i++)
		pos[i] = 1 << i;
	for (i = sz - 1, j = 1; i > -1; i--, j++)
		if (i == sz - j)
			pos[i] = leaf / (1 << (j - 1));
	for (i = 0; i < sz; i++)
		pos[i] &= 1;

	return pos;
}

template<class T>
void BTree<T>::add_branch (int leaf, T data)
{
	int i, j, t;
	static int cnt[0x20] = { 0 };

	if (leaf == 2 || leaf == 3) {
		pb = btree;
		goto setbranch;
	}

	for (i = 0; i < 0x20; i++) {
		if (leaf == (1 << i)) {
			for (j = 0; j < i - 2; j++)
				pb = pb->pbranch;
			for (j = 0; j < i - 1; j++)
				pb = pb->branch[0];
			goto setbranch;
		}
	}

	for (i = 4, j = 0; j < 0x20; j++, i <<= 1) {
		if (leaf == (i * cnt[j] + 6 * (1 << j))) {
			for (t = 0; t < j + 1; t++)
				pb = pb->pbranch;
			pb = pb->branch[1];
			for (t = 0; t < j; t++)
				pb = pb->branch[0];
			cnt[j]++;
			goto setbranch;
		}
	}

setbranch:
	if (leaf & 1)
		pb = last->pbranch;
	last = pb->branch[leaf & 1];
	last = new struct btree_st<T>;
	last->leaf = leaf;
	last->data = data;
	last->pbranch = pb;
	pb->branch[leaf & 1] = last;
}

template<class T>
struct btree_st<T> *BTree<T>::get_leaf (int leaf)
{
	int i, level, *path = nullptr;
	struct btree_st<T> *iter = btree;

	for (i = 0; ; i++) {
		if (!leaf) {
			level = 1;
			break;
		} else if (leaf >= (1 << i) && leaf < (1 << (i + 1))) {
			level = i + 1;
			break;
		}
	}

	path = get_path (leaf);
	for (i = 0; i < level - 1; i++)
		iter = iter->branch[path[i]];

	return iter;
}

template<class T>
void BTree<T>::clear ()
{
	int i, j, level = 0, *path = nullptr;
	struct btree_st<T> *iter;

	i = last->leaf;
	while (1) {
		for (j = 0; j < 0x20; j++) {
			if (i >= (1 << j) && i < (1 << (j + 1))) {
				level = j;
				break;
			}
		}

		path = get_path (i);
		for (iter = btree, j = 0; j < level; j++)
			iter = iter->branch[path[j]];
		// std::cout << "leaf = " << iter->leaf << std::endl;
		delete iter;

		i--;
		if (i < 2)
			break;
	};
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
			std::cout << "     +---> " << current->leaf << " [" << (current->data == 1 ? "S" : "M") << "]" << std::endl;
//				<< " [" << rlevel << "]"
//				<< std::endl;
		current = get_next_branch (&lv);
		rlevel += lv;
		if (rlevel < 0)
			break;
	}
	current = copy;
}
#endif
