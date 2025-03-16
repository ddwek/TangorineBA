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
#ifndef _TEST_H_
#define _TEST_H_	1
#include <string>
#include <gtk/gtk.h>
#include "../src/common.h"
#include "../src/Stack.h"

typedef struct expected_st {
	int n_step;
	std::string exp[6][6];
} expected_t;

class Test {
public:
	Test ();
	Test (Test&) = delete;
	Test (Test&&) = delete;
	Test& operator= (Test&) = delete;
	~Test ();

	bool parse_input (std::string in_filename);
	in_parsed_t *get_input_parsed ();
	bool is_input_already_parsed ();

	bool parse_expected (std::string exp_filename);
	expected_t get_expected_parsed (int n_step) const;
	bool is_expected_already_parsed ();
	int get_total_steps_to_parse ();
	int get_expected_step (int n_step);

	int pass (int n_step);

private:
	std::string in_filename;
	in_parsed_t *in_parsed;
	bool input_already_parsed;
	std::string exp_filename;
	expected_t *expected;
	int total_steps_to_parse;
	bool expected_already_parsed;
};
#endif
