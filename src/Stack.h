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
#ifndef _STACK_H_
#define _STACK_H_	1
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

extern class Stack undo, redo;
#endif
