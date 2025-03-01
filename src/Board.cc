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
#include <gtk/gtk.h>
#include "Digraph.h"
#include "BTree-dd.tcc"

typedef enum { SHAPE_SUN = 0, SHAPE_MOON, SHAPE_EMPTY, SHAPE_IMM } shape_t;
typedef struct pending_events_st {
	int ncell;
	shape_t shape;
} pending_events_t;
typedef struct imm_st {
	int ncell;
	shape_t shape;
	bool operator< (const struct imm_st& r);
} imm_t;

bool imm_t::operator< (const imm_t& r)
{
	if (this->ncell < r.ncell)
		return true;
	return false;
}

extern std::list<pending_events_t> redraw_cells;

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

Board::Board ()
{
	int seed = time (nullptr);

	std::cout << "seed = " << seed << std::endl;
	srand (seed);
	prepare ();

	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 6; j++)
			user_guess[i][j] = SHAPE_EMPTY;
}

Board::~Board ()
{
}

void Board::draw_shape (int nrow, int ncol, shape_t shape)
{
	struct _GdkRGBA color[2] = {	{ 0.7, 0.7, 0.2, 1.0, },
					{ 0.8, 0.8, 0.8, 0.5, },
	};
	struct _GdkRGBA gridcolor = { 0.1, 0.2, 0.3, 1.0 };
	struct _GdkRGBA darkercolor = { 0.0, 0.1, 0.2, 1.0 };

	cairo_save (cr);
	if (shape == SHAPE_SUN) {
		gdk_cairo_set_source_rgba (cr, &color[SHAPE_SUN]);
		cairo_arc (cr, ncol * 80 + 40, nrow * 80 + 40, 30, 0, 2 * G_PI);
		cairo_fill (cr);
	} else if (shape == SHAPE_MOON) {
		for (int i = 0; i < 6; i++)
			if (nrow * 6 + ncol == immutable_cells[i].ncell)
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
	} else if (shape == SHAPE_EMPTY) {
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
	imm_t imm_value;
	std::list<imm_t> imm;
	std::list<imm_t>::iterator iter;

	this->cr = cr;
	cairo_save (cr);
	gdk_cairo_set_source_rgba (cr, &gridcolor);
	for (i = 0; i < 6; i++)
		for (j = 0; j < 6; j++)
			cairo_rectangle (cr, i * 80 + 2, j * 80 + 2, 76, 76);
	cairo_fill (cr);
	cairo_restore (cr);

	if (!this->configured) {
		set_immutable_cells ();
		set_constraints ();
	}
	draw_immutable_cells ();
	draw_constraints ();

	for (i = 0; i < 6; i++) {
		imm_value.ncell = immutable_cells[i].ncell;
		for (j = 0; j < 6; j++)
			imm_value.shape = unique_solution[i][j];
		imm.push_back (imm_value);
	}
	imm.sort ();

	for (i = 0; i < 6; i++) {
		for (j = 0; j < 6; j++) {
			for (iter = imm.begin (); iter != imm.end (); iter++) {
				if (iter->ncell == i * 6 + j) {
					set_shape_status (i * 6 + j, SHAPE_IMM);
					draw_shape (i, j, (shape_t) unique_solution[i][j]);
					break;
				}
			}
		}
	}
}

void Board::set_hatching (int nrow, bool activate)
{
	for (int i = 0; i < 6; i++)
		hatching_cells[nrow * 6 + i] = activate;
}

bool Board::get_hatching (int nrow) const
{
	int i, match = 0;

	for (i = 0; i < 6; i++)
		if (hatching_cells[nrow * 6 + i])
			match++;

	if (match == 6)
		return true;
	return false;
}

void Board::draw_hatching (int nrow)
{
	struct _GdkRGBA hatching_color = { 0.6, 0.0, 0.0, 1.0 };

	cairo_save (cr);
	gdk_cairo_set_source_rgba (cr, &hatching_color);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 6; j++) {
			cairo_move_to (cr, j * 80 + 2, nrow * 80 + 80 - 2 - i * 20);
			cairo_line_to (cr, j * 80 + 80 - 2 - i * 20, nrow * 80);
		}
	}
	cairo_stroke (cr);

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 6; j++) {
			cairo_move_to (cr, j * 80 + 2 + i * 20, nrow * 80 + 80 - 2);
			cairo_line_to (cr, j * 80 + 80 - 2, nrow * 80 + i * 20);
		}
	}
	cairo_stroke (cr);
	cairo_restore (cr);
}

void Board::change_shape (int nrow, int ncol)
{
	if (unique_solution[nrow][ncol] == SHAPE_SUN)
		unique_solution[nrow][ncol] = SHAPE_MOON;
	else
		unique_solution[nrow][ncol] = SHAPE_SUN;
}

void Board::change_row (int nrow, shape_t shape)
{
	for (int j = 0; j < 4; j++)
		if (unique_solution[nrow][j] == shape && unique_solution[nrow][j + 1] == shape)
			unique_solution[nrow][j + 2] = (shape == SHAPE_SUN) ? SHAPE_MOON : SHAPE_SUN;
}

void Board::change_col (int ncol, shape_t shape)
{
	for (int i = 0; i < 4; i++)
		if (unique_solution[i][ncol] == shape && unique_solution[i + 1][ncol] == shape)
			unique_solution[i + 2][ncol] = (shape == SHAPE_SUN) ? SHAPE_MOON : SHAPE_SUN;
}

shape_t Board::get_shape_status (int ncell) const
{
	return unique_solution[ncell / 6][ncell % 6];
}

shape_t Board::get_user_guess (int ncell) const
{
	return user_guess[ncell / 6][ncell % 6];
}

void Board::set_shape_status (int ncell, shape_t sh)
{
	unique_solution[ncell / 6][ncell % 6] = sh;
}

void Board::set_user_guess (int ncell, shape_t sh)
{
	user_guess[ncell / 6][ncell % 6] = sh;
}

int Board::get_num_hsuns (int row)
{
	int num_suns = 0;

	for (int i = 0; i < 6; i++)
		if (unique_solution[row][i] == SHAPE_SUN)
			num_suns++;

	return num_suns;
}

int Board::get_num_vsuns (int col)
{
	int num_suns = 0;

	for (int i = 0; i < 6; i++)
		if (unique_solution[i][col] == SHAPE_SUN)
			num_suns++;

	return num_suns;
}

int Board::get_num_hmoons (int row)
{
	int num_moons = 0;

	for (int i = 0; i < 6; i++)
		if (unique_solution[row][i] == SHAPE_MOON)
			num_moons++;

	return num_moons;
}

int Board::get_num_vmoons (int col)
{
	int num_moons = 0;

	for (int i = 0; i < 6; i++)
		if (unique_solution[i][col] == SHAPE_MOON)
			num_moons++;

	return num_moons;
}

int Board::get_third_adjacent (int row, int col, line_type_check check)
{
	int i, adj = 0;

	if (check == ROW) {
		for (i = 0; i < 4; i++) {
			if (unique_solution[row][i] == unique_solution[row][i + 1])
				adj = 2;
			else
				adj = 0;
			if (adj == 2 && unique_solution[row][i + 1] == unique_solution[row][i + 2])
				return i + 2;
		}
	} else {
		for (i = 0; i < 4; i++) {
			if (unique_solution[i][col] == unique_solution[i + 1][col])
				adj = 2;
			else
				adj = 0;
			if (adj == 2 && unique_solution[i + 1][col] == unique_solution[i + 2][col])
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

void Board::validate_row (int nrow)
{
	int i, j, n_non_empty = 0, match = 0, cnt = 0;
	bool non_empty_row[6] = { 0 };

	for (i = 0; i < 6; i++) {
		if (unique_solution[nrow][i] == user_guess[nrow][i])
			non_empty_row[i] = true;
		else
			non_empty_row[i] = false;
	}

	for (i = 0; i < 6; i++)
		if (non_empty_row[i] || immutable_cells[i].shape == unique_solution[nrow][i])
			n_non_empty++;

	if (n_non_empty >= 3) {
		for (j = 0; j < 6; j++) {
			if (unique_solution[nrow][j] == user_guess[nrow][j] || unique_solution[nrow][j] == SHAPE_IMM)
				match++;
			if (match == 6) {
				set_hatching (nrow, false);
				for (i = 0; i < 36; i++)
					if (unique_solution[i / 6][i % 6] == user_guess[i / 6][i % 6] &&
					    unique_solution[i / 6][i % 6] != SHAPE_IMM)
						cnt++;
				if (cnt == 30)
					std::cout << "Board solved!!!" << std::endl;
			} else {
				set_hatching (nrow, true);
			}
		}
	}
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
			unique_solution[t][j] = (shape_t) (*iter)[j];

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
						unique_solution[i + 2][t] = (shape_t) (*iter)[t];
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

void Board::print ()
{
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			if (unique_solution[i][j] == SHAPE_SUN)
				std::cout << "* ";
			else
				std::cout << "D ";
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;
}

bool Board::is_immutable (int n) const
{
	for (int i = 0; i < 6; i++)
		if (immutable_cells[i].ncell == n)
			return true;

	return false;
}

void Board::set_immutable_cells ()
{
	int i = 0, j, rnd, ncell;

	while (true) {
new_rand:
		rnd = rand () % 36;
		for (; i < 6; ) {
			for (j = 0; j < 6; j++)
				if (immutable_cells[j].ncell == rnd)
					goto new_rand;
			if (immutable_cells[i].ncell != rnd) {
				immutable_cells[i].ncell = rnd;
				ncell = immutable_cells[i].ncell;
				immutable_cells[i].shape = unique_solution[ncell / 6][ncell % 6];
				i++;
				break;
			} else {
				break;
			}
		}

		if (i == 6)
			break;
	}
}

void Board::draw_immutable_cells ()
{
	int i;
	struct _GdkRGBA darkercolor = { 0.0, 0.1, 0.2, 1.0 };

	cairo_save (cr);
	for (i = 0; i < 6; i++) {
		gdk_cairo_set_source_rgba (cr, &darkercolor);
		cairo_rectangle (cr, (immutable_cells[i].ncell % 6) * 80 + 2, (immutable_cells[i].ncell / 6) * 80 + 2, 76, 76);
		cairo_fill (cr);
		draw_shape (immutable_cells[i].ncell / 6, immutable_cells[i].ncell % 6, immutable_cells[i].shape);
	}
	cairo_restore (cr);
}

void Board::set_constraints ()
{
	int dir;
	int _unique_solution[2] = { 0 };
	constr_t cons;
	std::list<constr_t>::iterator iter;
	char sep = '\0';

	while (true) {
new_rand:
		bool can_add = false;
		_unique_solution[0] = rand () % 0x24;
		dir = rand () & 3;
		if (dir == 0) {
			if (_unique_solution[0] < 6)
				goto new_rand;
			_unique_solution[1] = _unique_solution[0] - 6;
			if (unique_solution[_unique_solution[0] / 6][_unique_solution[0] % 6] == unique_solution[_unique_solution[1] / 6][_unique_solution[1] % 6])
				sep = '=';
			else
				sep = 'x';
		} else if (dir == 1) {
			if ((_unique_solution[0] + 1) % 6 == 0)
				goto new_rand;
			_unique_solution[1] = _unique_solution[0] + 1;
			if (unique_solution[_unique_solution[0] / 6][_unique_solution[0] % 6] == unique_solution[_unique_solution[1] / 6][_unique_solution[1] % 6])
				sep = '=';
			else
				sep = 'x';
		} else if (dir == 2) {
			if (_unique_solution[0] >= 30)
				goto new_rand;
			_unique_solution[1] = _unique_solution[0] + 6;
			if (unique_solution[_unique_solution[0] / 6][_unique_solution[0] % 6] == unique_solution[_unique_solution[1] / 6][_unique_solution[1] % 6])
				sep = '=';
			else
				sep = 'x';
		} else if (dir == 3) {
			if (!_unique_solution[0] || (_unique_solution[0] - 1) % 6 == 5)
				goto new_rand;
			_unique_solution[1] = _unique_solution[0] - 1;
			if (unique_solution[_unique_solution[0] / 6][_unique_solution[0] % 6] == unique_solution[_unique_solution[1] / 6][_unique_solution[1] % 6])
				sep = '=';
			else
				sep = 'x';
		}
		cons.cell_0 = _unique_solution[0];
		cons.cell_1 = _unique_solution[1];
		cons.constr = (sep == '=' ? EQUAL : DIFF);
		if (!constraints.size ())
			constraints.push_back (cons);
		for (iter = constraints.begin (); /*, i = 0; i < 36; i++, */ iter != constraints.end (); iter++) {
			if ((iter->cell_0 == _unique_solution[0] && iter->cell_1 == _unique_solution[1]) ||
			    (iter->cell_0 == _unique_solution[1] && iter->cell_1 == _unique_solution[0])) {
				can_add = false;
				break;
			} else {
				can_add = true;
			}
		}

		if (can_add)
			constraints.push_back (cons);
		if (constraints.size () == 8)
			break;
		else
			continue;
	}

	this->configured = true;
}

void Board::draw_constraints ()
{
	struct _GdkRGBA color = { 0.0, 1.0, 0.0, 1.0 };
	std::list<constr_st>::iterator iter;

	cairo_save (cr);
	gdk_cairo_set_source_rgba (cr, &color);
	for (iter = constraints.begin (); iter != constraints.end (); iter++) {
		if (iter->cell_0 == iter->cell_1 - 6) {
			if (iter->constr == EQUAL) {
				cairo_move_to (cr, (iter->cell_0 % 6) * 80 + 40 - 5, (iter->cell_0 / 6) * 80 + 80 - 3);
				cairo_line_to (cr, (iter->cell_0 % 6) * 80 + 40 + 5, (iter->cell_0 / 6) * 80 + 80 - 3);
				cairo_move_to (cr, (iter->cell_0 % 6) * 80 + 40 - 5, (iter->cell_0 / 6) * 80 + 80 + 3);
				cairo_line_to (cr, (iter->cell_0 % 6) * 80 + 40 + 5, (iter->cell_0 / 6) * 80 + 80 + 3);
			} else {
				cairo_move_to (cr, (iter->cell_0 % 6) * 80 + 40 - 5, (iter->cell_0 / 6) * 80 + 80 - 5);
				cairo_line_to (cr, (iter->cell_0 % 6) * 80 + 40 + 5, (iter->cell_0 / 6) * 80 + 80 + 5);
				cairo_move_to (cr, (iter->cell_0 % 6) * 80 + 40 + 5, (iter->cell_0 / 6) * 80 + 80 - 5);
				cairo_line_to (cr, (iter->cell_0 % 6) * 80 + 40 - 5, (iter->cell_0 / 6) * 80 + 80 + 5);
			}
		} else if (iter->cell_0 == iter->cell_1 - 1) {
			if (iter->cell_0 % 6 == 5)
				continue;
			if (iter->constr == EQUAL) {
				cairo_move_to (cr, (iter->cell_0 % 6) * 80 + 80 - 5, (iter->cell_0 / 6) * 80 + 80 - 40 - 3);
				cairo_line_to (cr, (iter->cell_0 % 6) * 80 + 80 + 5, (iter->cell_0 / 6) * 80 + 80 - 40 - 3);
				cairo_move_to (cr, (iter->cell_0 % 6) * 80 + 80 - 5, (iter->cell_0 / 6) * 80 + 80 - 40 + 3);
				cairo_line_to (cr, (iter->cell_0 % 6) * 80 + 80 + 5, (iter->cell_0 / 6) * 80 + 80 - 40 + 3);
			} else {
				cairo_move_to (cr, (iter->cell_0 % 6) * 80 + 80 - 5, (iter->cell_0 / 6) * 80 + 40 - 5);
				cairo_line_to (cr, (iter->cell_0 % 6) * 80 + 80 + 5, (iter->cell_0 / 6) * 80 + 40 + 5);
				cairo_move_to (cr, (iter->cell_0 % 6) * 80 + 80 + 5, (iter->cell_0 / 6) * 80 + 40 - 5);
				cairo_line_to (cr, (iter->cell_0 % 6) * 80 + 80 - 5, (iter->cell_0 / 6) * 80 + 40 + 5);
			}
		} else if (iter->cell_0 == iter->cell_1 + 6) {
			if (iter->constr == EQUAL) {
				cairo_move_to (cr, (iter->cell_0 % 6) * 80 + 40 - 5, (iter->cell_0 / 6) * 80 - 3);
				cairo_line_to (cr, (iter->cell_0 % 6) * 80 + 40 + 5, (iter->cell_0 / 6) * 80 - 3);
				cairo_move_to (cr, (iter->cell_0 % 6) * 80 + 40 - 5, (iter->cell_0 / 6) * 80 + 3);
				cairo_line_to (cr, (iter->cell_0 % 6) * 80 + 40 + 5, (iter->cell_0 / 6) * 80 + 3);
			} else {
				cairo_move_to (cr, (iter->cell_0 % 6) * 80 + 40 - 5, (iter->cell_0 / 6) * 80 - 5);
				cairo_line_to (cr, (iter->cell_0 % 6) * 80 + 40 + 5, (iter->cell_0 / 6) * 80 + 5);
				cairo_move_to (cr, (iter->cell_0 % 6) * 80 + 40 + 5, (iter->cell_0 / 6) * 80 - 5);
				cairo_line_to (cr, (iter->cell_0 % 6) * 80 + 40 - 5, (iter->cell_0 / 6) * 80 + 5);
			}
		} else if (iter->cell_0 == iter->cell_1 + 1) {
			if (!(iter->cell_0 % 6))
				continue;
			if (iter->constr == EQUAL) {
				cairo_move_to (cr, (iter->cell_0 % 6) * 80 - 5, (iter->cell_0 / 6) * 80 + 40 - 3);
				cairo_line_to (cr, (iter->cell_0 % 6) * 80 + 5, (iter->cell_0 / 6) * 80 + 40 - 3);
				cairo_move_to (cr, (iter->cell_0 % 6) * 80 - 5, (iter->cell_0 / 6) * 80 + 40 + 3);
				cairo_line_to (cr, (iter->cell_0 % 6) * 80 + 5, (iter->cell_0 / 6) * 80 + 40 + 3);
			} else {
				cairo_move_to (cr, (iter->cell_0 % 6) * 80 - 5, (iter->cell_0 / 6) * 80 + 40 - 5);
				cairo_line_to (cr, (iter->cell_0 % 6) * 80 + 5, (iter->cell_0 / 6) * 80 + 40 + 5);
				cairo_move_to (cr, (iter->cell_0 % 6) * 80 + 5, (iter->cell_0 / 6) * 80 + 40 - 5);
				cairo_line_to (cr, (iter->cell_0 % 6) * 80 - 5, (iter->cell_0 / 6) * 80 + 40 + 5);
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
