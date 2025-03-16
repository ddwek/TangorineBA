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
#include <fstream>
#include <list>
#include <gtk/gtk.h>
#include "../src/common.h"
#include "../src/Board.h"
#include "../src/Callback.h"

typedef struct expected_st {
	int n_step;
	std::string exp[6][6];
} expected_t;

extern bool are_there_pending_events;
extern std::list<pending_events_t> redraw_cells;
pending_events_t pending_event;

/*
 * We must to shape a test-suite so that we can run unit tests as simple
 * as we can, and then create the pipeline which will be able to connect
 * this test-suite with autotools' test-driver
 *
 */
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

Test::Test ()
{
	in_parsed = new in_parsed_t;
	in_parsed->seed = 0;

	for (int i = 0; i < 6; i++)
		in_parsed->imm[i] = 0;

	for (int i = 0; i < 8; i++) {
		in_parsed->cons[i].ncell = 0;
		in_parsed->cons[i].flags.imm = 0;
		in_parsed->cons[i].flags.top = 0;
		in_parsed->cons[i].flags.top_equal = 0;
		in_parsed->cons[i].flags.right = 0;
		in_parsed->cons[i].flags.right_equal = 0;
		in_parsed->cons[i].flags.bottom = 0;
		in_parsed->cons[i].flags.bottom_equal = 0;
		in_parsed->cons[i].flags.left = 0;
		in_parsed->cons[i].flags.left_equal = 0;
		in_parsed->cons[i].flags.claim_for_hor_hatching = 0;
		in_parsed->cons[i].flags.claim_for_ver_hatching = 0;
	}

	expected = new expected_t;
}

Test::~Test ()
{
	delete in_parsed;
}

bool Test::parse_input (std::string in_filename)
{
	int i, j, t, pos;
	std::fstream ifile;
	std::string str, aux_str;
	std::string ncell_str, cons_dir_str, cons_tst_str;
	char buf[0x40] = { '\0' };

	if (!in_filename.length ())
		return false;
	this->in_filename = in_filename;

	ifile.open (in_filename, std::ios::in);
	while (!ifile.eof ()) {
		/* seed */
		ifile.getline (buf, 0x40);
		str = buf;
		pos = str.find ("seed =");
		for (i = pos; str[i] != '\0'; i++)
			if (str[i] >= '0' && str[i] <= '9')
				aux_str += str[i];
		in_parsed->seed = atoi (aux_str.c_str ());
		aux_str.clear ();
		for (j = 0; j < 0x40; j++)
			buf[j] = '\0';

		/* set_immutable_cells */
		ifile.getline (buf, 0x40);
		str = buf;
		pos = str.find ("set_immutable_cells(): ");
		j = 0;
		for (t = 0, i = pos; str[i] != '\0' && t < 6; i++, t++) {
			for (; str[j] != ','; j++) {
				if (str[j] >= '0' && str[j] <= '9')
					aux_str += str[j];
			}
			in_parsed->imm[t] = atoi (aux_str.c_str ());
			aux_str.clear ();
			j++;
		}
		for (j = 0; j < 0x40; j++)
			buf[j] = '\0';

		/* set_constraints */
		ifile.getline (buf, 0x40);
		str = buf;
		pos = str.find ("set_constraints():");
		j = 0;
		for (t = 0, i = pos; str[i] != '\0' && t < 8; i++, t++) {
			ifile.getline (buf, 0x80);
			str = buf;
			for (j = 0; str[j] != '\0'; j++) {
				if (str[j] >= '0' && str[j] <= '9')
					ncell_str += str[j];
				if (str[j] >= 'A' && str[j] <= 'Z')
					cons_dir_str += str[j];
				if (str[j] == '=' || str[j] == 'x')
					cons_tst_str += str[j];
			}

			in_parsed->cons[t].ncell = atoi (ncell_str.c_str ());

			if (cons_dir_str == "TOP") {
				in_parsed->cons[t].flags.top = 1;
				if (cons_tst_str == "=")
					in_parsed->cons[t].flags.top_equal = 1;
				else
					in_parsed->cons[t].flags.top_equal = 0;
			}

			if (cons_dir_str == "RIGHT") {
				in_parsed->cons[t].flags.right = 1;
				if (cons_tst_str == "=")
					in_parsed->cons[t].flags.right_equal = 1;
				else
					in_parsed->cons[t].flags.right_equal = 0;
			}

			if (cons_dir_str == "BOTTOM") {
				in_parsed->cons[t].flags.bottom = 1;
				if (cons_tst_str == "=")
					in_parsed->cons[t].flags.bottom_equal = 1;
				else
					in_parsed->cons[t].flags.bottom_equal = 0;
			}

			if (cons_dir_str == "LEFT") {
				in_parsed->cons[t].flags.left = 1;
				if (cons_tst_str == "=")
					in_parsed->cons[t].flags.left_equal = 1;
				else
					in_parsed->cons[t].flags.left_equal = 0;
			}

			ncell_str.clear ();
			cons_dir_str.clear ();
			cons_tst_str.clear ();
			for (j = 0; j < 0x40; j++)
				buf[j] = '\0';
		}
		break;
	}
	ifile.close ();
	input_already_parsed = true;

	return true;
}

in_parsed_t *Test::get_input_parsed ()
{
	return in_parsed;
}

bool Test::is_input_already_parsed ()
{
	return input_already_parsed;
}

/*
 * At this point, we have already parsed the input information to recreate
 * the stage for error-proof tests. So now it's time to gather the output
 * information, that is, what we expect to get from stdout and check for
 * identical strings as each unit test is executed.
 *
 * Note that @exp_filename gets filenames ending in "*.expected" extension.
 * However, it has nothing to do with the dialect Expect which is shipped
 * with Tcl programming language. No pun intended, it's just a coincidence
 *
 */
bool Test::parse_expected (std::string exp_filename)
{
	int i, j, t, pos, row = 0, col = 0, step;
	std::fstream ifile;
	std::string str, aux_str;
	char buf[0x100] = { '\0' };

	if (!exp_filename.length ())
		return false;
	this->exp_filename = exp_filename;

	ifile.open (exp_filename, std::ios::in);
	while (!ifile.eof ()) {
		/* n_steps */
		ifile.getline (buf, 0x100);
		str = buf;
		pos = str.find ("n_steps = ");

		for (i = pos; str[i] != '\0'; i++)
			if (str[i] >= '0' && str[i] <= '9')
				aux_str += str[i];

		total_steps_to_parse = atoi (aux_str.c_str ());
		aux_str.clear ();
		for (j = 0; j < 0x100; j++)
			buf[j] = '\0';
		break;
	};

	j = 0;
	this->expected = new expected_t [total_steps_to_parse];
	while (!ifile.eof ()) {
		/* steps */
		ifile.getline (buf, 0x100);
		str = buf;
		pos = str.find ("steps: ");

		for (i = pos; str[i] != '\0'; i++) {
			if (str[i] == ',') {
				expected[j].n_step = atoi (aux_str.c_str ());
				aux_str.clear ();
				j++;
				continue;
			}
			if (str[i] < '0' || str[i] > '9')
				continue;
			aux_str += str[i];
		}

		for (i = 0; i < 0x100; i++)
			buf[i] = '\0';
		break;
	};

	t = 0;
	pos = 0;
	step = 0;
	while (!ifile.eof ()) {
		/* rows */
new_step:
		ifile.getline (buf, 0x100);
		str = buf;
		if (str.length () == 1) {
			step++;
			str.clear ();
			goto new_step;
		}

		if (str.length () == 0)
			continue;

keep_processing:
		for (i = pos; str[i] != ' '; i++)
			if (str[i] >= 0x1b && str[i] <= 0x7f)
				expected[step].exp[row][col] += str[i];
		if (str[i] == ' ')
			col++;
		if (col == 6) {
			for (j = 0; j < 0x100; j++)
				buf[j] = '\0';
			str.clear ();
			pos = 0;
			col = 0;
			row++;
		} else {
			pos = 0;
			for (i = 0; i < col; i++)
				pos += expected[step].exp[row][i].length () + 1;
			goto keep_processing;
		}

		if (row == 6) {
			std::cout << "Expected() done: Step n = " << expected[step].n_step << std::endl;
			for (i = 0; i < 6; i++) {
				for (j = 0; j < 6; j++)
					std::cout << expected[step].exp[i][j] << " ";
				std::cout << std::endl;
			}
			std::cout << std::endl;
			t++;
			pos = 0;
			row = 0;
			col = 0;
			goto new_step;
		}

		if (t == total_steps_to_parse)
			break;
	};
	ifile.close ();
	expected_already_parsed = true;

	return true;
}

expected_t Test::get_expected_parsed (int n_step) const
{
	return expected[n_step];
}

bool Test::is_expected_already_parsed ()
{
	return expected_already_parsed;
}

int Test::get_total_steps_to_parse ()
{
	return total_steps_to_parse;
}

int Test::get_expected_step (int n_step)
{
	return expected[n_step].n_step;
}

int Test::pass (int n_step)
{
	int i, j, t;
	bool ret = false;

	t = n_step;
	board.print (true, false, t);
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 6; j++) {
			if (expected[t].exp[i][j] == board.get_debug (t, i, j)) {
				ret = 1;
			} else {
				ret = 0;
				goto out;
			}
		}
	}
out:
	return ret;
}
#include "../src/Stack.h"
