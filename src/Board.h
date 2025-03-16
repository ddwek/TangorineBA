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
#ifndef _BOARD_H_
#define _BOARD_H_	1
#include <set>
#include <string>
#include <gtk/gtk.h>
#include "common.h"

typedef struct shape_info_st {
	int ncell;
	shape_t shape;
	bm_flags_t flags;
} shape_info_t;

class Board {
public:
	typedef enum { ROW = 0, COL } line_type_check;
	typedef enum { EQUAL = 0, DIFF } type_cons_t;
	typedef struct constr_st {
		int cell_0;
		int cell_1;
		type_cons_t constr;
	} constr_t;

	Board ();
	Board (bool testing, std::string test_filename);
	Board (Board&) = delete;
	Board (Board&&) = delete;
	Board& operator= (Board&) = delete;
	~Board ();

	void new_game ();
	void set_seed (int seed);
	void draw_shape (int nrow, int ncol, shape_t shape);
	void draw_cells (cairo_t *cr);
	bool can_draw_hatching (int ncell);
	void set_hatching (int ncell, bool hor);
	void clear_hatching (int ncell, bool hor);
	void draw_hatching (int ncell);
	void draw_hatching_on_immutable ();
	bool get_game_over ();
	void set_game_over (bool game_over);
	void show_congrats ();
	void change_shape (int nrow, int ncol);
	void change_row (int nrow, shape_t shape);
	void change_col (int ncol, shape_t shape);
	shape_info_t get_shape_status (int ncell) const;
	shape_info_t get_user_guess (int ncell) const;
	shape_info_t get_standard_solution (int ncell) const;
	void set_shape_status (int ncell, shape_t sh);
	void set_user_guess (int ncell, shape_t sh, bm_flags_t flags);
	int get_num_hsuns (int row, bool std);
	int get_num_vsuns (int col, bool std);
	int get_num_hmoons (int row, bool std);
	int get_num_vmoons (int col, bool std);
	int get_third_adjacent (int row, int col, line_type_check check, bool std);
	int is_valid (int *row, int *col, int *nsuns, int *nmoons, bool std);

	void validate_row_three_adjs (int nrow);
	void validate_row_diff_num_of_shapes (int nrow);
	void validate_row_constraints (int nrow);
	void validate_row (int nrow);
	void validate_col_three_adjs (int ncol);
	void validate_col_diff_num_of_shapes (int ncol);
	void validate_col_constraints (int ncol);
	void validate_col (int ncol);

	void prepare ();
	void print (bool is_testing, bool display_values, int n_step);
	std::string get_debug (int n_step, int row, int col) const;
	bool is_immutable (int n) const;
	void set_immutable_cells (int *imm);
	void set_immutable_cells ();
	void draw_immutable_cells ();
	void decode_flags (shape_info_t& ref);
	void set_constraints (cons_t *cons);
	void set_constraints ();
	void draw_constraints ();
	bool is_configured () const;
	bool is_testing () const;

private:
	cairo_t *cr;
	shape_info_t standard_solution[6][6];
	shape_info_t user_guess[6][6];
	bool configured;
	bool testing;
	std::string test_filename;
	std::string exp_filename;
	int seed;
	std::string debug[0x40][6][6];
	bool game_over;
};

extern class Board board;
#endif
