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
#include <vector>
#include <list>

typedef struct _digraph_st {
	int a;
	int b;
	int c;
	int d;
	int name[4];
} digraph_t;

std::list<digraph_t> *digraph_get_paths (int **inmtx, int dim, int a, int b, int c, int d)
{
	int i, j, niterations, row, col, level, nextrow_int = 0;
	int *savedrow = nullptr, *nextcol = nullptr, *startingcol = nullptr;
	int **mtx = nullptr;
	std::vector<int> fields (dim);
	digraph_t d_out;
	std::list<digraph_t> *lst = new std::list<digraph_t>;
	std::list<digraph_t>::iterator iter, nextrow;

	mtx = new int* [dim];
	for (i = 0; i < dim; i++)
		mtx[i] = new int [dim];
	for (i = 0; i < dim; i++)
		for (j = 0; j < dim; j++)
			mtx[i][j] = inmtx[i][j];

	nextcol = new int [dim];
	startingcol = new int [dim];
	for (i = 0; i < dim; i++)
		startingcol[i] = -1;

	savedrow = new int [dim];
	row = 0;
	col = 0;
	nextrow_int = 1;
	savedrow[0] = row;
	nextcol[0] = 1;
	for (i = 1; i < dim; i++)
		nextcol[i] = -1;
	level = 0;
	niterations = 0;
	for (;;) {
		if (mtx[row][col] == 0) {
			col = (col + 1) % dim;
			niterations++;
check_end_of_row:
			if (col == startingcol[row] || niterations == dim) {
				if (--level == -1) {
					row++;
					if (nextrow_int) {
						nextrow_int++;
						if (nextrow_int == dim + 1)
							break;
					}
					savedrow[0] = row;
					col = 0;
					nextcol[0] = 1;
					fields[0] = row;
					level = 0;
					niterations = 0;
					for (i = level; i < dim; i++)
						startingcol[i] = -1;
					goto check_end_of_row;
				}

				row = savedrow[level];
				col = nextcol[level];
				niterations = 0;
				goto check_end_of_row;
			}
			continue;
		} else {
			for (i = 0; i <= level; i++) {
				if (fields[i] == col) {
					if (startingcol[row] == -1)
						startingcol[row] = col;
					col = (col + 1) % dim;
					goto check_end_of_row;
				}
			}

			nextcol[level] = (col + 1) % dim;
			if (startingcol[row] == -1)
				startingcol[row] = col;
			savedrow[level] = row;
			level++;
			fields[level] = col;

			if (level != dim - 1) {
				row = col;
				col = nextcol[level];
				niterations = 0;
				startingcol[row] = -1;
				continue;
			}

			d_out.a = fields[0];
			d_out.b = fields[1];
			d_out.c = fields[2];
			d_out.d = fields[3];
			d_out.name[0] = a;
			d_out.name[1] = b;
			d_out.name[2] = c;
			d_out.name[3] = d;
			lst->push_back (d_out);
			level--;
			row = savedrow[level];
			col = nextcol[level];
			niterations = 0;
			goto check_end_of_row;
		}
	}

	for (i = 0; i < dim; i++)
		delete [] mtx[i];
	delete [] mtx;
	delete [] savedrow;
	delete [] nextcol;
	delete [] startingcol;

	return lst;
}
