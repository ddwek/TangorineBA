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
#include <iostream>
#include <iomanip>
#include <string>
#include <stack>
#include <math.h>
#include "common.h"

class Stack {
public:
	Stack () = default;
	~Stack () = default;

	history_t top () const;
	long unsigned get_size () const;

	void push (history_t& c);
	void pop ();
	void remove_downwards (int base);
	void display (std::string name);
private:
	std::stack<history_t> stk;
};

history_t Stack::top () const
{
	return stk.top ();
}

long unsigned Stack::get_size () const
{
	return stk.size ();
}

void Stack::push (history_t& c)
{
	stk.push (c);
}

void Stack::pop ()
{
	stk.pop ();
}

void Stack::remove_downwards (int base)
{
	for (int i = (int) stk.size (); (int) i > base; i--)
		stk.pop ();
}

void Stack::display (std::string name)
{
	long unsigned i, sz;

	Stack cp = *this;
	sz = cp.stk.size ();
	for (i = 0; i < sz; i++) {
		std::cout << name << ": " << cp.top().uid << ": " <<
			cp.top().pe.ncell << ", " <<
			(cp.top().pe.shape ? "S" : "M") << std::endl;
		cp.pop ();
	}
	std::cout << "-----------------------------------------" << std::endl;
}
