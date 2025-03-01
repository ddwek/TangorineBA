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
#include <vector>
#include <list>
#include <set>
#include <gtk/gtk.h>
#include "Digraph.h"
#include "BTree-dd.tcc"
#include "common.h"
#include "../test/Test.h"

typedef enum { SHAPE_SUN = 0, SHAPE_MOON, SHAPE_EMPTY } shape_t;

typedef struct pending_events_st {
	int ncell;
	shape_t shape;
	bm_flags_t flags;
} pending_events_t;

typedef struct shape_info_st {
	int ncell;
	shape_t shape;
	bm_flags_t flags;
	bool operator< (const struct shape_info_st& ref) const;
} shape_info_t;

bool shape_info_t::operator< (const shape_info_t& ref) const
{
	if (this->ncell < ref.ncell)
		return true;
	return false;
}

extern std::list<pending_events_t> redraw_cells;
Test test;

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
	int get_num_hsuns (int row);
	int get_num_vsuns (int col);
	int get_num_hmoons (int row);
	int get_num_vmoons (int col);
	int get_third_adjacent (int row, int col, line_type_check check);
	int is_valid (int *row, int *col, int *nsuns, int *nmoons);

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


// Default constructor for interactive gameplay (i.e., not for testing)
Board::Board ()
{
	seed = time (nullptr);
	std::cout << "seed = " << seed << std::endl;
	srand (seed);
	prepare ();
	game_over = false;

	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			standard_solution[i][j].ncell = i * 6 + j;
			user_guess[i][j].ncell = i * 6 + j;
			user_guess[i][j].shape = SHAPE_EMPTY;

			user_guess[i][j].flags.imm = 0;
			user_guess[i][j].flags.top = 0;
			user_guess[i][j].flags.top_equal = 0;
			user_guess[i][j].flags.right = 0;
			user_guess[i][j].flags.right_equal = 0;
			user_guess[i][j].flags.bottom = 0;
			user_guess[i][j].flags.bottom_equal = 0;
			user_guess[i][j].flags.left = 0;
			user_guess[i][j].flags.left_equal = 0;
		}
	}
}

// Constructor overloaded for testing (`make check`)
Board::Board (bool testing, std::string test_filename)
{
	if (testing) {
		this->testing = true;

		this->test_filename = test_filename + ".input";
		test.parse_input (this->test_filename);

		this->exp_filename = test_filename + ".expected";
		test.parse_expected (this->exp_filename);
	}

	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {a
			/*
			 * Some configurations demonstrate that there's no a
			 * "unique solution", but can be more than one and
			 * entirely valid, so that we decided rename it to
			 * "standard solution", indicating an arbitrary or trivial
			 * possible solution
			 */
			standard_solution[i][j].ncell = i * 6 + j;
			user_guess[i][j].ncell = i * 6 + j;
			user_guess[i][j].shape = SHAPE_EMPTY;
			for (int t = 0; t < 8; t++) {
				if (test.get_input_parsed ()->cons[t].ncell == i * 6 + j) {
					standard_solution[i][j].flags = test.get_input_parsed ()->cons[t].flags;
					user_guess[i][j].flags = test.get_input_parsed ()->cons[t].flags;
				}
			}
		}
	}

	seed = test.get_input_parsed ()->seed;
	srand (seed);
	prepare ();
}

Board::~Board ()
{
}

void Board::set_seed (int seed)
{
	this->seed = seed;
}

void Board::draw_shape (int nrow, int ncol, shape_t shape)
{
	struct _GdkRGBA color[2] = {	{ 0.7, 0.7, 0.2, 1.0, },
					{ 0.8, 0.8, 0.8, 0.5, },
	};
	struct _GdkRGBA normalcolor = { 0.1, 0.2, 0.3, 1.0 };
	struct _GdkRGBA darkercolor = { 0.0, 0.1, 0.2, 1.0 };
	struct _GdkRGBA gridcolor = normalcolor;

	cairo_save (cr);
	if (shape == SHAPE_SUN) {		// Draw just a sun...
		gdk_cairo_set_source_rgba (cr, &color[SHAPE_SUN]);
		cairo_arc (cr, ncol * 80 + 40, nrow * 80 + 40, 30, 0, 2 * G_PI);
		cairo_fill (cr);
	} else if (shape == SHAPE_MOON) {	// ...and a moon
		if (standard_solution[nrow][ncol].flags.imm)
			gridcolor = darkercolor;
		gdk_cairo_set_source_rgba (cr, &gridcolor);
		cairo_rectangle (cr, ncol * 80 + 2, nrow * 80 + 2, 76, 76);
		cairo_fill (cr);
		gdk_cairo_set_source_rgba (cr, &color[SHAPE_MOON]);
		cairo_arc_negative (cr, ncol * 80 + 40, nrow * 80 + 40, 30, 0.75 * G_PI, -0.25 * G_PI);
		cairo_fill (cr);
		gdk_cairo_set_source_rgba (cr, &gridcolor);
		cairo_arc_negative (cr, ncol * 80 + 30, nrow * 80 + 30, 32, 0.75 * G_PI, -0.25 * G_PI);
		cairo_fill (cr);
	} else if (shape == SHAPE_EMPTY) {	// Draw nothing at all
		gdk_cairo_set_source_rgba (cr, &gridcolor);
		cairo_rectangle (cr, ncol * 80 + 2, nrow * 80 + 2, 76, 76);
		cairo_fill (cr);
	}
	cairo_restore (cr);
}

void Board::draw_cells (cairo_t *cr)
{
	int i, j;
	struct _GdkRGBA gridcolor = { 0.1, 0.2, 0.3, 1.0 };

	this->cr = cr;
	cairo_save (cr);
	gdk_cairo_set_source_rgba (cr, &gridcolor);

	// The original size of the window is 480x480 px
	for (i = 0; i < 6; i++)
		for (j = 0; j < 6; j++)
			cairo_rectangle (cr, i * 80 + 2, j * 80 + 2, 76, 76);
	cairo_fill (cr);
	cairo_restore (cr);

	if (!this->configured) {
		if (this->testing) {
			set_immutable_cells (test.get_input_parsed ()->imm);
			set_constraints (test.get_input_parsed ()->cons);
		} else {
			set_immutable_cells ();
			set_constraints ();
		}
		this->configured = true;
	}
	draw_immutable_cells ();
	draw_constraints ();
}

bool Board::can_draw_hatching (int ncell)
{
	if (user_guess[ncell / 6][ncell % 6].flags.claim_for_hor_hatching ||
	    user_guess[ncell / 6][ncell % 6].flags.claim_for_ver_hatching)
		return true;
	return false;
}

void Board::set_hatching (int ncell, bool hor)
{
	if (hor)
		user_guess[ncell / 6][ncell % 6].flags.claim_for_hor_hatching = 1;
	else
		user_guess[ncell / 6][ncell % 6].flags.claim_for_ver_hatching = 1;
}

void Board::clear_hatching (int ncell, bool hor)
{
	if (hor)
		user_guess[ncell / 6][ncell % 6].flags.claim_for_hor_hatching = 0;
	else
		user_guess[ncell / 6][ncell % 6].flags.claim_for_ver_hatching = 0;
}

void Board::draw_hatching (int ncell)
{
	struct _GdkRGBA hatching_color = { 0.6, 0.0, 0.0, 1.0 };

	cairo_save (cr);
	gdk_cairo_set_source_rgba (cr, &hatching_color);
	for (int i = 0; i < 4; i++) {
		cairo_move_to (cr, (ncell % 6) * 80 + 4, (ncell / 6) * 80 + 80 - 4 - i * 20);
		cairo_line_to (cr, (ncell % 6) * 80 + 80 - i * 20 - 4, (ncell / 6) * 80 + 4);
	}

	for (int i = 0; i < 4; i++) {
		cairo_move_to (cr, (ncell % 6) * 80 + 4 + i * 20, (ncell / 6) * 80 + 80 - 4);
		cairo_line_to (cr, (ncell % 6) * 80 + 80 - 4, (ncell / 6) * 80 + 4 + i * 20);
	}
	cairo_stroke (cr);
	cairo_restore (cr);
}

void Board::draw_hatching_on_immutable ()
{
	int i, j;

	for (i = 0; i < 6; i++) {
		for (j = 0; j < 6; j++) {
			if (user_guess[i][j].flags.imm) {
				if (user_guess[i][j].flags.claim_for_hor_hatching) {
					set_hatching (i * 6 + j, true);
					draw_hatching (i * 6 + j);
				} else {
					clear_hatching (i * 6 + j, true);
				}

				if (user_guess[i][j].flags.claim_for_ver_hatching) {
					set_hatching (i * 6 + j, false);
					draw_hatching (i * 6 + j);
				} else {
					clear_hatching (i * 6 + j, false);
				}
			}
		}
	}
}

bool Board::get_game_over ()
{
	return this->game_over;
}

/*
 * The game is over iff the location of the shapes on the board are valid and
 * there's no additional hatchings on any cell (this is a workaround for a
 * bug which let you end the game successfully even when there was at least
 * one cell marked as wrong)
 */
void Board::set_game_over (bool game_over)
{
	int i, j, row = -1, col = -1, nsuns = -1, nmoons = -1;
	int err_invalid = 0, hatchings_to_change = 0;

	err_invalid = is_valid (&row, &col, &nsuns, &nmoons);
	for (i = 0; i < 6; i++)
		for (j = 0; j < 6; j++)
			if (user_guess[i][j].flags.claim_for_hor_hatching ||
			    user_guess[i][j].flags.claim_for_ver_hatching)
				hatchings_to_change++;
				
	if (!err_invalid && !hatchings_to_change)
		this->game_over = game_over;
}

void Board::show_congrats ()
{
	struct _GdkRGBA bgcolor = { 0.3, 0.6, 0.3, 0.8 };
	struct _GdkRGBA fgcolor = { 1.0, 1.0, 1.0, 1.0 };

	cairo_save (cr);
	gdk_cairo_set_source_rgba (cr, &bgcolor);
	cairo_rectangle (cr, 120, 180, 240, 120);
	cairo_fill (cr);
	cairo_restore (cr);

	cairo_save (cr);
	gdk_cairo_set_source_rgba (cr, &fgcolor);
	cairo_move_to (cr, 140, 250);
	cairo_scale (cr, 4.0, 4.0);
	cairo_select_font_face (cr, "cairo:monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_show_text (cr, "You won!!!");
	cairo_restore (cr);
}

void Board::change_shape (int nrow, int ncol)
{
	if (standard_solution[nrow][ncol].shape == SHAPE_SUN)
		standard_solution[nrow][ncol].shape = SHAPE_MOON;
	else
		standard_solution[nrow][ncol].shape = SHAPE_SUN;
}

void Board::change_row (int nrow, shape_t shape)
{
	for (int j = 0; j < 4; j++)
		if (standard_solution[nrow][j].shape == shape && standard_solution[nrow][j + 1].shape == shape)
			standard_solution[nrow][j + 2].shape = (shape == SHAPE_SUN) ? SHAPE_MOON : SHAPE_SUN;
}

void Board::change_col (int ncol, shape_t shape)
{
	for (int i = 0; i < 4; i++)
		if (standard_solution[i][ncol].shape == shape && standard_solution[i + 1][ncol].shape == shape)
			standard_solution[i + 2][ncol].shape = (shape == SHAPE_SUN) ? SHAPE_MOON : SHAPE_SUN;
}

shape_info_t Board::get_shape_status (int ncell) const
{
	return standard_solution[ncell / 6][ncell % 6];
}

shape_info_t Board::get_user_guess (int ncell) const
{
	return user_guess[ncell / 6][ncell % 6];
}

shape_info_t Board::get_standard_solution (int ncell) const
{
	return standard_solution[ncell / 6][ncell % 6];
}

void Board::set_shape_status (int ncell, shape_t sh)
{
	standard_solution[ncell / 6][ncell % 6].shape = sh;
}

void Board::set_user_guess (int ncell, shape_t sh, bm_flags_t flags)
{
	user_guess[ncell / 6][ncell % 6].shape = sh;
	user_guess[ncell / 6][ncell % 6].flags = flags;
}

int Board::get_num_hsuns (int row)
{
	int num_suns = 0;

	for (int i = 0; i < 6; i++)
		if (standard_solution[row][i].shape == SHAPE_SUN)
			num_suns++;

	return num_suns;
}

int Board::get_num_vsuns (int col)
{
	int num_suns = 0;

	for (int i = 0; i < 6; i++)
		if (standard_solution[i][col].shape == SHAPE_SUN)
			num_suns++;

	return num_suns;
}

int Board::get_num_hmoons (int row)
{
	int num_moons = 0;

	for (int i = 0; i < 6; i++)
		if (standard_solution[row][i].shape == SHAPE_MOON)
			num_moons++;

	return num_moons;
}

int Board::get_num_vmoons (int col)
{
	int num_moons = 0;

	for (int i = 0; i < 6; i++)
		if (standard_solution[i][col].shape == SHAPE_MOON)
			num_moons++;

	return num_moons;
}

int Board::get_third_adjacent (int row, int col, line_type_check check)
{
	int i, adj = 0;

	if (check == ROW) {
		for (i = 0; i < 4; i++) {
			if (standard_solution[row][i].shape == standard_solution[row][i + 1].shape)
				adj = 2;
			else
				adj = 0;
			if (adj == 2 && standard_solution[row][i + 1].shape == standard_solution[row][i + 2].shape)
				return i + 2;
		}
	} else {
		for (i = 0; i < 4; i++) {
			if (standard_solution[i][col].shape == standard_solution[i + 1][col].shape)
				adj = 2;
			else
				adj = 0;
			if (adj == 2 && standard_solution[i + 1][col].shape == standard_solution[i + 2][col].shape)
				return i + 2;
		}
	}

	return -1;
}

int Board::is_valid (int *row, int *col, int *nsuns, int *nmoons)
{
	int i, j, ret = 0;

	for (i = 0; i < 6; i++) {
		j = get_third_adjacent (-1, i, COL);
		if (j != -1) {
			*col = i;
			ret |= 1;
		}
		if (get_num_hsuns (i) != get_num_hmoons (i)) {
			*nsuns = get_num_hsuns (i);
			*nmoons = get_num_hmoons (i);
			ret |= 2;
		}
	}

	for (i = 0; i < 6; i++) {
		j = get_third_adjacent (i, -1, ROW);
		if (j != -1) {
			*row = i;
			ret |= 4;
		}
		if (get_num_vsuns (i) != get_num_vmoons (i)) {
			*nsuns = get_num_vsuns (i);
			*nmoons = get_num_vmoons (i);
			ret |= 8;
		}
	}

	return ret;
}

void Board::validate_row_diff_num_of_shapes (int nrow)
{
	int i, suns = 0, moons = 0;

	for (i = 0; i < 6; i++) {
		shape_info_t& r = user_guess[nrow][i];
		if (r.shape == SHAPE_EMPTY)
			clear_hatching (nrow * 6 + i, true);
		if (r.shape == SHAPE_SUN)
			suns++;
		else if (r.shape == SHAPE_MOON)
			moons++;
	}

	if (suns + moons == 6) {
		if (suns < 3 || moons < 3) {
			// If suns == (0, 1, 2) && moons == (6, 5, 4)
			for (i = 0; i < 6; i++)
				set_hatching (nrow * 6 + i, true);
		} else {
			for (i = 0; i < 6; i++)
				clear_hatching (nrow * 6 + i, true);
		}
	} else {
		for (i = 0; i < 6; i++)
			clear_hatching (nrow * 6 + i, true);
	}
}

void Board::validate_row_three_adjs (int nrow)
{
	int i;

	for (i = 0; i < 4; i++) {
		shape_info_t& r0 = user_guess[nrow][i + 0];
		shape_info_t& r1 = user_guess[nrow][i + 1];
		shape_info_t& r2 = user_guess[nrow][i + 2];
		if (r0.shape == SHAPE_EMPTY || r1.shape == SHAPE_EMPTY || r2.shape == SHAPE_EMPTY) {
			clear_hatching (nrow * 6 + i + 0, true);
			clear_hatching (nrow * 6 + i + 1, true);
			clear_hatching (nrow * 6 + i + 2, true);
		}

		if (r0.shape != SHAPE_EMPTY && r0.shape == r1.shape && r1.shape == r2.shape) {
			set_hatching (nrow * 6 + i + 0, true);
			set_hatching (nrow * 6 + i + 1, true);
			set_hatching (nrow * 6 + i + 2, true);
			i += 2;
		} else {
			if (get_num_hsuns (nrow) + get_num_hmoons (nrow) == 6 &&
			    get_num_hsuns (nrow) != get_num_hmoons (nrow)) {
				clear_hatching (nrow * 6 + i + 0, true);
				clear_hatching (nrow * 6 + i + 1, true);
				clear_hatching (nrow * 6 + i + 2, true);
			}
		}
	}
}

void Board::validate_row_constraints (int nrow)
{
	int i;

	for (i = 0; i < 5; i++) {
		shape_info_t& r = user_guess[nrow][i];
		if (r.flags.right) {
			shape_info_t& next = user_guess[nrow][i + 1];
			if (r.flags.right_equal) {
				if (r.shape != next.shape && r.shape != SHAPE_EMPTY) {
					set_hatching (nrow * 6 + i + 0, true);
					set_hatching (nrow * 6 + i + 1, true);
				} else {
					if (get_num_hsuns (nrow) + get_num_hmoons (nrow) == 6 &&
					    get_num_hsuns (nrow) != get_num_hmoons (nrow)) {
						clear_hatching (nrow * 6 + i + 0, true);
						clear_hatching (nrow * 6 + i + 1, true);
					}
				}
			} else {
				if (r.shape == next.shape && r.shape != SHAPE_EMPTY) {
					set_hatching (nrow * 6 + i + 0, true);
					set_hatching (nrow * 6 + i + 1, true);
				} else {
					if (get_num_hsuns (nrow) + get_num_hmoons (nrow) == 6 &&
					    get_num_hsuns (nrow) != get_num_hmoons (nrow)) {
						clear_hatching (nrow * 6 + i + 0, true);
						clear_hatching (nrow * 6 + i + 1, true);
					}
				}
			}
		}
	}

	for (i = 1; i < 6; i++) {
		shape_info_t& r = user_guess[nrow][i];
		if (r.flags.left) {
			shape_info_t& prev = user_guess[nrow][i - 1];
			if (r.flags.left_equal) {
				if (r.shape != prev.shape && r.shape != SHAPE_EMPTY) {
					set_hatching (nrow * 6 + i - 0, true);
					set_hatching (nrow * 6 + i - 1, true);
				} else {
					if (get_num_hsuns (nrow) + get_num_hmoons (nrow) == 6 &&
					    get_num_hsuns (nrow) != get_num_hmoons (nrow)) {
						clear_hatching (nrow * 6 + i - 0, true);
						clear_hatching (nrow * 6 + i - 1, true);
					}
				}
			} else {
				if (r.shape == prev.shape && r.shape != SHAPE_EMPTY) {
					set_hatching (nrow * 6 + i - 0, true);
					set_hatching (nrow * 6 + i - 1, true);
				} else {
					if (get_num_hsuns (nrow) + get_num_hmoons (nrow) == 6 &&
					    get_num_hsuns (nrow) != get_num_hmoons (nrow)) {
						clear_hatching (nrow * 6 + i - 0, true);
						clear_hatching (nrow * 6 + i - 1, true);
					}
				}
			}
		}
	}
}

void Board::validate_row (int nrow)
{
	validate_row_diff_num_of_shapes (nrow);
	validate_row_three_adjs (nrow);
	validate_row_constraints (nrow);
}

void Board::validate_col_diff_num_of_shapes (int ncol)
{
	int i, suns = 0, moons = 0;

	for (i = 0; i < 6; i++) {
		shape_info_t& r = user_guess[i][ncol];
		if (r.shape == SHAPE_EMPTY)
			clear_hatching (i * 6 + ncol, false);
		if (r.shape == SHAPE_SUN)
			suns++;
		else if (r.shape == SHAPE_MOON)
			moons++;
	}

	if (suns + moons == 6) {
		if (suns < 3 || moons < 3) {
			for (i = 0; i < 6; i++)
				set_hatching (i * 6 + ncol, false);
		} else {
			for (i = 0; i < 6; i++)
				clear_hatching (i * 6 + ncol, false);
		}
	} else {
		for (i = 0; i < 6; i++)
			clear_hatching (i * 6 + ncol, false);
	}
}

void Board::validate_col_three_adjs (int ncol)
{
	int i;

	for (i = 0; i < 4; i++) {
		shape_info_t& r0 = user_guess[i + 0][ncol];
		shape_info_t& r1 = user_guess[i + 1][ncol];
		shape_info_t& r2 = user_guess[i + 2][ncol];
		if (r0.shape == SHAPE_EMPTY || r1.shape == SHAPE_EMPTY || r2.shape == SHAPE_EMPTY) {
			clear_hatching ((i + 0) * 6 + ncol, false);
			clear_hatching ((i + 1) * 6 + ncol, false);
			clear_hatching ((i + 2) * 6 + ncol, false);
		}

		if (r0.shape != SHAPE_EMPTY && r0.shape == r1.shape && r1.shape == r2.shape) {
			set_hatching ((i + 0) * 6 + ncol, false);
			set_hatching ((i + 1) * 6 + ncol, false);
			set_hatching ((i + 2) * 6 + ncol, false);
			i += 2;
		} else {
			if (get_num_vsuns (ncol) + get_num_vmoons (ncol) == 6 &&
			    get_num_vsuns (ncol) != get_num_vmoons (ncol)) {
				clear_hatching ((i + 0) * 6 + ncol, false);
				clear_hatching ((i + 1) * 6 + ncol, false);
				clear_hatching ((i + 2) * 6 + ncol, false);
			}
		}
	}
}

void Board::validate_col_constraints (int ncol)
{
	int i;

	for (i = 1; i < 6; i++) {
		shape_info_t& r = user_guess[i][ncol];
		if (r.flags.top) {
			shape_info_t& prev = user_guess[i - 1][ncol];
			if (r.flags.top_equal) {
				if (r.shape != prev.shape && r.shape != SHAPE_EMPTY) {
					set_hatching ((i - 0) * 6 + ncol, false);
					set_hatching ((i - 1) * 6 + ncol, false);
				} else {
					if (get_num_vsuns (ncol) + get_num_vmoons (ncol) == 6 &&
					    get_num_vsuns (ncol) != get_num_vmoons (ncol)) {
						clear_hatching ((i - 0) * 6 + ncol, false);
						clear_hatching ((i - 1) * 6 + ncol, false);
					}
				}
			} else {
				if (r.shape == prev.shape && r.shape != SHAPE_EMPTY) {
					set_hatching ((i - 0) * 6 + ncol, false);
					set_hatching ((i - 1) * 6 + ncol, false);
				} else {
					if (get_num_vsuns (ncol) + get_num_vmoons (ncol) == 6 &&
					    get_num_vsuns (ncol) != get_num_vmoons (ncol)) {
						clear_hatching ((i - 0) * 6 + ncol, false);
						clear_hatching ((i - 1) * 6 + ncol, false);
					}
				}
			}
		}
	}

	for (i = 0; i < 5; i++) {
		shape_info_t& r = user_guess[i][ncol];
		if (r.flags.bottom) {
			shape_info_t& next = user_guess[i + 1][ncol];
			if (r.flags.bottom_equal) {
				if (r.shape != next.shape && r.shape != SHAPE_EMPTY) {
					set_hatching ((i + 0) * 6 + ncol, false);
					set_hatching ((i + 1) * 6 + ncol, false);
				} else {
					if (get_num_vsuns (ncol) + get_num_vmoons (ncol) == 6 &&
					    get_num_vsuns (ncol) != get_num_vmoons (ncol)) {
						clear_hatching ((i + 0) * 6 + ncol, false);
						clear_hatching ((i + 1) * 6 + ncol, false);
					}
				}
			} else {
				if (r.shape == next.shape && r.shape != SHAPE_EMPTY) {
					set_hatching ((i + 0) * 6 + ncol, false);
					set_hatching ((i + 1) * 6 + ncol, false);
				} else {
					if (get_num_vsuns (ncol) + get_num_vmoons (ncol) == 6 &&
					    get_num_vsuns (ncol) != get_num_vmoons (ncol)) {
						clear_hatching ((i + 0) * 6 + ncol, false);
						clear_hatching ((i + 1) * 6 + ncol, false);
					}
				}
			}
		}
	}
}

void Board::validate_col (int ncol)
{
	validate_col_diff_num_of_shapes (ncol);
	validate_col_three_adjs (ncol);
	validate_col_constraints (ncol);
}

std::list<std::vector<int>>::iterator get_nth_iter (std::list<std::vector<int>>& l, int nth)
{
	int i;
	std::list<std::vector<int>>::iterator iter;

	for (i = 0, iter = l.begin (); iter != l.end (); iter++, i++)
		if (i == nth)
			return iter;

	return l.end ();
}

/*
 * Before the game starts, we need to prepare all the cells for the standard
 * solution. We know that's a little bit tricky to explain what we did for it
 * to work fine, but our approach is as follows:
 *
 * 1. Create a B-Tree and populate it with 128 branches
 * 2. Select only 6-step paths starting from 64 and ending at 128
 * 3. Generate the two first rows as random but distinct from each other
 * 4. Prepare a 1's complemented boolean identity matrix of 4 possible values
 * 5. Get the paths from the digraph and map each one to numbers 0 =< x =< 3
 * 6. Complete the last 4 rows with the mappings gotten in the previous step
 *
 */
void Board::prepare ()
{
	int i, j, t, *path, err;
	int nsuns, nmoons, row, col;
	int **mtx = nullptr;
	bool has_three_adjs = false;
	BTree<int> bt (-1);
	std::vector<int> hvect (6);
	std::list<digraph_t> *l;
	std::list<std::vector<int>> hlst;
	std::list<std::vector<int>>::iterator iter;
	std::list<digraph_t>::iterator vec_iter;

	for (i = 2; i < 0x80; i++) {
		if (!(i & 1))
			bt.add_branch (i, (int) SHAPE_MOON);
		else
			bt.add_branch (i, (int) SHAPE_SUN);
	}

	for (i = 0x40; i < 0x80; i++) {
		has_three_adjs = false;
		path = bt.get_path (i);
		for (j = 0; j < 6; j++) {
			if (path[j])
				nsuns++;
			else
				nmoons++;
		}

		if (nsuns == nmoons) {
			for (j = 0; j < 4; j++) {
				if (path[j] == path[j + 1] && path[j + 1] == path[j + 2]) {
					has_three_adjs = true;
					break;
				}
			}

			if (!has_three_adjs) {
				for (j = 0; j < 6; j++)
					hvect[j] = path[j];
				hlst.push_back (hvect);
				has_three_adjs = false;
			}
		} else {
			nsuns = 0;
			nmoons = 0;
		}
	}

	int rnd0 = rand () % hlst.size ();

another_rand:
	int rnd1 = rand () % hlst.size ();
	if (rnd1 == rnd0)
		goto another_rand;
	int rnd = rnd0;
	for (i = 0, t = 0, iter = hlst.begin (); iter != hlst.end (); iter++, i++) {
		if (i < rnd)
			continue;
		for (j = 0; j < 6; j++)
			standard_solution[t][j].shape = (shape_t) (*iter)[j];

		if (rnd == rnd0) {
			rnd = rnd1;
			i = 0;
			t++;
			iter = hlst.begin ();
		} else {
			break;
		}
	}

	mtx = new int* [4];
	for (i = 0; i < 4; i++)
		mtx[i] = new int [4];
	for (iter = hlst.begin (), i = 0; i < 4; i++, iter++) {
		for (j = 0; j < 4; j++) {
			if (i != j)
				mtx[i][j] = 1;
			else
				mtx[i][j] = 0;
		}
	}

	const int c_cnt[4] = { 0, 1, 2, 3 };
	int cnt[4] = { 0, 1, 2, 3 };
	l = digraph_get_paths (mtx, 4, cnt[0], cnt[1], cnt[2], cnt[3]);
	while (true) {
new_paths:
		for (vec_iter = l->begin (); ; vec_iter++) {
			if (vec_iter == l->end ())
				return;
			while (vec_iter != l->end ()) {
				for (i = 0; i < 4; i++) {
					if (!i)
						iter = get_nth_iter (hlst, (*vec_iter).name[(*vec_iter).a]);
					else if (i == 1)
						iter = get_nth_iter (hlst, (*vec_iter).name[(*vec_iter).b]);
					else if (i == 2)
						iter = get_nth_iter (hlst, (*vec_iter).name[(*vec_iter).c]);
					else if (i == 3)
						iter = get_nth_iter (hlst, (*vec_iter).name[(*vec_iter).d]);
					if (iter == hlst.end ())
						break;
					for (int t = 0; t < 6; t++)
						standard_solution[i + 2][t].shape = (shape_t) (*iter)[t];
					if (!(err = is_valid (&row, &col, &nsuns, &nmoons)))
						return;
				}
				vec_iter++;
			};

			for (j = 3; j > -1; ) {
				if (cnt[j] < (int) hlst.size ()) {
					cnt[j]++;
					l = digraph_get_paths (mtx, 4, cnt[0], cnt[1], cnt[2], cnt[3]);
					goto new_paths;
				} else {
					cnt[j] = c_cnt[j];
					j--;
				}
			}
			return;
		}
	};
}

void Board::print (bool is_testing, bool display_values, int n_step)
{
	int i, j, step;
	typedef enum { COLOR_RED = 0, COLOR_YELLOW, COLOR_WHITE,
			COLOR_BLACK_FG_YELLOW_BG, COLOR_WHITE_FG_BLUE_BG,
			COLOR_RED_FG_WHITE_BG, COLOR_NORMAL } color_t;
	std::string esc_seq[7] = {
				"\033[01;40;31m",
				"\033[01;40;33m",
				"\033[01;40;37m",
				"\033[00;43;30m",
				"\033[01;44;37m",
				"\033[01;47;31m",
				"\033[00m",
	};

	if (!is_testing || test.get_total_steps_to_parse () > 0x40)
		return;
	for (step = 0; step < test.get_total_steps_to_parse (); step++)
		for (i = 0; i < 6; i++)
			for (j = 0; j < 6; j++)
				debug[step][i][j].clear ();

	step = n_step;
	if (display_values)
		std::cout << "step n = " << step << std::endl;
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 6; j++) {
			if (user_guess[i][j].shape == SHAPE_SUN) {
				if (user_guess[i][j].flags.claim_for_hor_hatching || user_guess[i][j].flags.claim_for_ver_hatching) {
					debug[step][i][j] = esc_seq[COLOR_RED];
					if (user_guess[i][j].flags.imm) {
						debug[step][i][j].clear ();
						debug[step][i][j] = esc_seq[COLOR_RED_FG_WHITE_BG];
					}
				} else {
					if (user_guess[i][j].flags.imm)
						debug[step][i][j] = esc_seq[COLOR_BLACK_FG_YELLOW_BG];
					else
						debug[step][i][j] = esc_seq[COLOR_YELLOW];
				}
				debug[step][i][j] += "*";
				debug[step][i][j] += esc_seq[COLOR_NORMAL];
				if (display_values)
					std::cout << debug[step][i][j] << " ";
			} else if (user_guess[i][j].shape == SHAPE_MOON) {
				if (user_guess[i][j].flags.claim_for_hor_hatching || user_guess[i][j].flags.claim_for_ver_hatching) {
					debug[step][i][j] = esc_seq[COLOR_RED];
					if (user_guess[i][j].flags.imm) {
						debug[step][i][j].clear ();
						debug[step][i][j] = esc_seq[COLOR_RED_FG_WHITE_BG];
					}
				} else {
					if (user_guess[i][j].flags.imm)
						debug[step][i][j] = esc_seq[COLOR_WHITE_FG_BLUE_BG];
					else
						debug[step][i][j] = esc_seq[COLOR_WHITE];
				}
				debug[step][i][j] += "D";
				debug[step][i][j] += esc_seq[COLOR_NORMAL];
				if (display_values)
					std::cout << debug[step][i][j] << " ";
			} else {
				debug[step][i][j] += "_";
				debug[step][i][j] += esc_seq[COLOR_NORMAL];
				if (display_values)
					std::cout << debug[step][i][j] << " ";
			}
		}
		if (display_values)
			std::cout << std::endl;
	}
	if (display_values)
		std::cout << std::endl;
}

std::string Board::get_debug (int n_step, int row, int col) const
{
	if (n_step >= test.get_total_steps_to_parse () || row > 5 || col > 5)
		return "";

	return debug[n_step][row][col];
}

bool Board::is_immutable (int n) const
{
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 6; j++)
			if (standard_solution[i][j].ncell == n && standard_solution[i][j].flags.imm)
				return true;
	return false;
}

void Board::set_immutable_cells (int *imm)
{
	for (int i = 0; i < 6; i++) {
		standard_solution[imm[i] / 6][imm[i] % 6].flags.imm = 1;
		user_guess[imm[i] / 6][imm[i] % 6].flags.imm = 1;
	}

	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			if (standard_solution[i][j].flags.imm) {
				user_guess[i][j].ncell = i * 6 + j;
				user_guess[i][j].shape = standard_solution[i][j].shape;
				user_guess[i][j].flags.imm = 1;
			}
		}
	}
}

void Board::set_immutable_cells ()
{
	int i, j, rnd;
	int row, col;
	bool ins;
	std::set<int> s;
	std::set<int>::iterator iter;

	if (this->testing)
		return;

	while (true) {
		rnd = rand () % 36;
		if (!s.size ()) {
			s.insert (rnd);
			i = 1;
		}
		for (i = 0, iter = s.begin (); iter != s.end (); iter++, i++) {
			row = rnd / 6;
			col = rnd % 6;
			if (*iter / 6 != row && *iter % 6 != col) {
				ins = true;
			} else {
				ins = false;
				break;
			}

			if ((unsigned) i == s.size () - 1 && ins)
				s.insert (rnd);
		}
		if (s.size () == 6)
			break;
	}

	for (iter = s.begin (); iter != s.end (); iter++) {
		standard_solution[*iter / 6][*iter % 6].flags.imm = 1;
		user_guess[*iter / 6][*iter % 6].flags.imm = 1;
	}

	std::cout << __FUNCTION__ << "(): ";
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 6; j++) {
			if (standard_solution[i][j].flags.imm) {
				user_guess[i][j].ncell = i * 6 + j;
				user_guess[i][j].shape = standard_solution[i][j].shape;
				std::cout << user_guess[i][j].ncell << ", ";
			}
		}
	}
	std::cout << std::endl;
}

void Board::draw_immutable_cells ()
{
	int i;
	struct _GdkRGBA darkercolor = { 0.0, 0.1, 0.2, 1.0 };

	cairo_save (cr);
	gdk_cairo_set_source_rgba (cr, &darkercolor);
	for (i = 0; i < 36; i++) {
		if (standard_solution[i / 6][i % 6].flags.imm) {
			cairo_rectangle (cr, (i % 6) * 80 + 2, (i / 6) * 80 + 2, 76, 76);
			cairo_fill (cr);
			draw_shape (i / 6, i % 6, standard_solution[i / 6][i % 6].shape);
		}
	}
	cairo_restore (cr);
}

void Board::decode_flags (shape_info_t& ref)
{
	if (ref.flags.top) {
		std::cout << ref.ncell << ": top ";
		if (ref.flags.top_equal)
			std::cout << "(=), ";
		else
			std::cout << "(x), ";
	}

	if (ref.flags.right) {
		std::cout << ref.ncell << ": right, ";
		if (ref.flags.right_equal)
			std::cout << "(=), ";
		else
			std::cout << "(x), ";
	}

	if (ref.flags.bottom) {
		std::cout << ref.ncell << ": bottom, ";
		if (ref.flags.bottom_equal)
			std::cout << "(=), ";
		else
			std::cout << "(x), ";
	}

	if (ref.flags.left) {
		std::cout << ref.ncell << ": left, ";
		if (ref.flags.left_equal)
			std::cout << "(=), ";
		else
			std::cout << "(x), ";
	}

	std::cout << std::endl;
}

void Board::set_constraints (cons_t *cons)
{
	int i;
	std::set<shape_info_t> s;
	std::set<shape_info_t>::iterator iter;

	for (int i = 0; i < 8; i++) {
		standard_solution[cons[i].ncell / 6][cons[i].ncell % 6].flags.top = cons[i].flags.top;
		standard_solution[cons[i].ncell / 6][cons[i].ncell % 6].flags.top_equal = cons[i].flags.top_equal;

		standard_solution[cons[i].ncell / 6][cons[i].ncell % 6].flags.right = cons[i].flags.right;
		standard_solution[cons[i].ncell / 6][cons[i].ncell % 6].flags.right_equal = cons[i].flags.right_equal;

		standard_solution[cons[i].ncell / 6][cons[i].ncell % 6].flags.bottom = cons[i].flags.bottom;
		standard_solution[cons[i].ncell / 6][cons[i].ncell % 6].flags.bottom_equal = cons[i].flags.bottom_equal;

		standard_solution[cons[i].ncell / 6][cons[i].ncell % 6].flags.left = cons[i].flags.left;
		standard_solution[cons[i].ncell / 6][cons[i].ncell % 6].flags.left_equal = cons[i].flags.left_equal;

		s.insert (standard_solution[cons[i].ncell / 6][cons[i].ncell % 6]);
	}

	for (i = 0, iter = s.begin (); iter != s.end (); iter++, i++) {
		standard_solution[iter->ncell / 6][iter->ncell % 6].flags = iter->flags;
		user_guess[iter->ncell / 6][iter->ncell % 6].flags = iter->flags;
	}
}

void Board::set_constraints ()
{
	int dir;
	int _u[2] = { 0 };
	std::set<shape_info_t> s;
	std::set<shape_info_t>::iterator iter;

	std::cout << __FUNCTION__ << "(): ";
	while (true) {
new_rand:
		_u[0] = rand () % 36;
		dir = rand () & 3;
		shape_info_t& ref = standard_solution[_u[0] / 6][_u[0] % 6];
		if (dir == 0) {
			if (_u[0] < 6)
				goto new_rand;
			_u[1] = _u[0] - 6;
			if (standard_solution[_u[1] / 6][_u[1] % 6].flags.bottom)
				goto new_rand;
			if (ref.shape == standard_solution[_u[1] / 6][_u[1] % 6].shape) {
				ref.flags.top = 1;
				ref.flags.top_equal = 1;
			} else {
				ref.flags.top = 1;
				ref.flags.top_equal = 0;
			}
			s.insert (ref);
		} else if (dir == 1) {
			if ((_u[0] + 1) % 6 == 0)
				goto new_rand;
			_u[1] = _u[0] + 1;
			if (standard_solution[_u[1] / 6][_u[1] % 6].flags.left)
				goto new_rand;
			if (ref.shape == standard_solution[_u[1] / 6][_u[1] % 6].shape) {
				ref.flags.right = 1;
				ref.flags.right_equal = 1;
			} else {
				ref.flags.right = 1;
				ref.flags.right_equal = 0;
			}
			s.insert (ref);
		} else if (dir == 2) {
			if (_u[0] >= 30)
				goto new_rand;
			_u[1] = _u[0] + 6;
			if (standard_solution[_u[1] / 6][_u[1] % 6].flags.top)
				goto new_rand;
			if (ref.shape == standard_solution[_u[1] / 6][_u[1] % 6].shape) {
				ref.flags.bottom = 1;
				ref.flags.bottom_equal = 1;
			} else {
				ref.flags.bottom = 1;
				ref.flags.bottom_equal = 0;
			}
			s.insert (ref);
		} else if (dir == 3) {
			if (!_u[0] || (_u[0] - 1) % 6 == 5)
				goto new_rand;
			_u[1] = _u[0] - 1;
			if (standard_solution[_u[1] / 6][_u[1] % 6].flags.right)
				goto new_rand;
			if (ref.shape == standard_solution[_u[1] / 6][_u[1] % 6].shape) {
				ref.flags.left = 1;
				ref.flags.left_equal = 1;
			} else {
				ref.flags.left = 1;
				ref.flags.left_equal = 0;
			}
			s.insert (ref);
		}

		if (s.size () == 8)
			break;
	}

	std::string flags;
	std::cout << std::endl;
	for (iter = s.begin (); iter != s.end (); iter++) {
		standard_solution[iter->ncell / 6][iter->ncell % 6].flags = iter->flags;
		user_guess[iter->ncell / 6][iter->ncell % 6].flags = iter->flags;
		std::cout << iter->ncell << ", ";
		if (iter->flags.top) {
			flags += "TOP, ";
			if (iter->flags.top_equal)
				flags += "=";
			else
				flags += "x";
		}

		if (iter->flags.right) {
			flags += "RIGHT, ";
			if (iter->flags.right_equal)
				flags += "=";
			else
				flags += "x";
		}

		if (iter->flags.bottom) {
			flags += "BOTTOM, ";
			if (iter->flags.bottom_equal)
				flags += "=";
			else
				flags += "x";
		}

		if (iter->flags.left) {
			flags += "LEFT, ";
			if (iter->flags.left_equal)
				flags += "=";
			else
				flags += "x";
		}
		std::cout << flags << std::endl;
		flags.clear ();
	}
}

void Board::draw_constraints ()
{
	int i;
	struct _GdkRGBA color = { 0.0, 1.0, 0.0, 1.0 };

	cairo_save (cr);
	gdk_cairo_set_source_rgba (cr, &color);
	for (i = 0; i < 36; i++) {
		shape_info_t& ref = standard_solution[i / 6][i % 6];
		if (ref.flags.top) {
			if (i < 6)
				continue;
			if (ref.flags.top_equal) {
				cairo_move_to (cr, (i % 6) * 80 + 40 - 5, (i / 6 - 1) * 80 + 80 - 3);
				cairo_line_to (cr, (i % 6) * 80 + 40 + 5, (i / 6 - 1) * 80 + 80 - 3);
				cairo_move_to (cr, (i % 6) * 80 + 40 - 5, (i / 6 - 1) * 80 + 80 + 3);
				cairo_line_to (cr, (i % 6) * 80 + 40 + 5, (i / 6 - 1) * 80 + 80 + 3);
			} else {
				cairo_move_to (cr, (i % 6) * 80 + 40 - 5, (i / 6 - 1) * 80 + 80 - 5);
				cairo_line_to (cr, (i % 6) * 80 + 40 + 5, (i / 6 - 1) * 80 + 80 + 5);
				cairo_move_to (cr, (i % 6) * 80 + 40 + 5, (i / 6 - 1) * 80 + 80 - 5);
				cairo_line_to (cr, (i % 6) * 80 + 40 - 5, (i / 6 - 1) * 80 + 80 + 5);
			}
		}

		if (ref.flags.right) {
			if (i % 6 == 5)
				continue;
			if (ref.flags.right_equal) {
				cairo_move_to (cr, (i % 6) * 80 + 80 - 5, (i / 6) * 80 + 80 - 40 - 3);
				cairo_line_to (cr, (i % 6) * 80 + 80 + 5, (i / 6) * 80 + 80 - 40 - 3);
				cairo_move_to (cr, (i % 6) * 80 + 80 - 5, (i / 6) * 80 + 80 - 40 + 3);
				cairo_line_to (cr, (i % 6) * 80 + 80 + 5, (i / 6) * 80 + 80 - 40 + 3);
			} else {
				cairo_move_to (cr, (i % 6) * 80 + 80 - 5, (i / 6) * 80 + 40 - 5);
				cairo_line_to (cr, (i % 6) * 80 + 80 + 5, (i / 6) * 80 + 40 + 5);
				cairo_move_to (cr, (i % 6) * 80 + 80 + 5, (i / 6) * 80 + 40 - 5);
				cairo_line_to (cr, (i % 6) * 80 + 80 - 5, (i / 6) * 80 + 40 + 5);
			}
		}

		if (ref.flags.bottom) {
			if (i > 30)
				continue;
			if (ref.flags.bottom_equal) {
				cairo_move_to (cr, (i % 6) * 80 + 40 - 5, (i / 6 + 1) * 80 - 3);
				cairo_line_to (cr, (i % 6) * 80 + 40 + 5, (i / 6 + 1) * 80 - 3);
				cairo_move_to (cr, (i % 6) * 80 + 40 - 5, (i / 6 + 1) * 80 + 3);
				cairo_line_to (cr, (i % 6) * 80 + 40 + 5, (i / 6 + 1) * 80 + 3);
			} else {
				cairo_move_to (cr, (i % 6) * 80 + 40 - 5, (i / 6 + 1) * 80 - 5);
				cairo_line_to (cr, (i % 6) * 80 + 40 + 5, (i / 6 + 1) * 80 + 5);
				cairo_move_to (cr, (i % 6) * 80 + 40 + 5, (i / 6 + 1) * 80 - 5);
				cairo_line_to (cr, (i % 6) * 80 + 40 - 5, (i / 6 + 1) * 80 + 5);
			}
		}

		if (ref.flags.left) {
			if (i % 6 == 0)
				continue;
			if (ref.flags.left_equal) {
				cairo_move_to (cr, (i % 6) * 80 - 5, (i / 6) * 80 + 40 - 3);
				cairo_line_to (cr, (i % 6) * 80 + 5, (i / 6) * 80 + 40 - 3);
				cairo_move_to (cr, (i % 6) * 80 - 5, (i / 6) * 80 + 40 + 3);
				cairo_line_to (cr, (i % 6) * 80 + 5, (i / 6) * 80 + 40 + 3);
			} else {
				cairo_move_to (cr, (i % 6) * 80 - 5, (i / 6) * 80 + 40 - 5);
				cairo_line_to (cr, (i % 6) * 80 + 5, (i / 6) * 80 + 40 + 5);
				cairo_move_to (cr, (i % 6) * 80 + 5, (i / 6) * 80 + 40 - 5);
				cairo_line_to (cr, (i % 6) * 80 - 5, (i / 6) * 80 + 40 + 5);
			}
		}
		cairo_stroke (cr);
	}
	cairo_restore (cr);
}

bool Board::is_configured () const
{
	return this->configured;
}

bool Board::is_testing () const
{
	return this->testing;
}
