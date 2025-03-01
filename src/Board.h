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
#include <vector>
#include <list>
#include <gtk/gtk.h>

typedef enum { SHAPE_SUN = 0, SHAPE_MOON, SHAPE_EMPTY, SHAPE_IMM } shape_t;

class Board {
public:
	typedef enum { ROW = 0, COL } line_type_check;
	typedef enum { EQUAL = 0, DIFF } type_cons_t;
	typedef struct constr_st {
		int cell_0;
		int cell_1;
		type_cons_t constr;
	} constr_t;
	typedef struct imm_st {
		int ncell;
		shape_t shape;
		bool operator< (const struct imm_st& r);
	} imm_t;

	Board ();
	Board (Board&) = delete;
	Board (Board&&) = delete;
	Board& operator= (Board&) = delete;
	~Board ();

	void draw_shape (int nrow, int ncol, shape_t shape);
	void draw_cells (cairo_t *cr);
	void set_hatching (int nrow, bool activate);
	bool get_hatching (int nrow) const;
	void draw_hatching (int nrow);
	void change_shape (int nrow, int ncol);
	void change_row (int nrow, shape_t shape);
	void change_col (int ncol, shape_t shape);
	shape_t get_shape_status (int ncell) const;
	shape_t get_user_guess (int ncell) const;
	void set_shape_status (int ncell, shape_t sh);
	void set_user_guess (int ncell, shape_t sh);
	int get_num_hsuns (int row);
	int get_num_vsuns (int col);
	int get_num_hmoons (int row);
	int get_num_vmoons (int col);
	int get_third_adjacent (int row, int col, line_type_check check);
	int is_valid (int *row, int *col, int *nsuns, int *nmoons);
	void validate_row (int nrow);
	void prepare ();
	void print ();
	bool is_immutable (int n) const;
	void set_immutable_cells ();
	void draw_immutable_cells ();
	void set_constraints ();
	void draw_constraints ();
	bool is_configured () const;
private:
	cairo_t *cr;
	shape_t unique_solution[6][6];
	shape_t user_guess[6][6];
	std::list<constr_t> constraints;
	imm_t immutable_cells[6];
	bool hatching_cells[36];
	bool configured;
};

extern class Board board;
#endif
