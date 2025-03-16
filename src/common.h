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
#ifndef _COMMON_H_
#define _COMMON_H_	1
#include <gtk/gtk.h>

// Bitmask of flags for each cell
typedef struct bm_flags_st {
	unsigned int imm : 1;		// Immutable
	unsigned int top : 1;		// Top constraint
	unsigned int top_equal : 1;	// Top must be equal (1) or diff (0) to prev row cell
	unsigned int right : 1;
	unsigned int right_equal : 1;
	unsigned int bottom : 1;
	unsigned int bottom_equal : 1;
	unsigned int left : 1;
	unsigned int left_equal : 1;	// ...
	unsigned int padding : 23;	// Reserved amount of bits up to complete 32-bit int
	int claim_for_hor_hatching;	// Horizontal hatching
	int claim_for_ver_hatching;	// Vertical hatching
} bm_flags_t;

// Constraints' struct
typedef struct cons_st {
	int ncell;
	bm_flags_t flags;
	bm_flags_t& operator= (bm_flags_t&);	// Overload operator = for easy copy of flags
} cons_t;

// Input parsed info only meaningfull for testing (i.e., `make check`)
typedef struct parsed_st {
	int seed;		// Random seed
	int imm[6];		// Just 6 cells will be immutable
	cons_t cons[8];		// Just 8 constraints per board, but one cell can have more than one constraint
} in_parsed_t;

typedef enum { SHAPE_SUN = 0, SHAPE_MOON, SHAPE_EMPTY } shape_t;

typedef struct pending_events_st {
	int ncell;
	shape_t shape;
	bm_flags_t flags;
} pending_events_t;

typedef struct history_st {
	pending_events_t pe;
	int uid;
} history_t;

extern GtkWidget *undo_btn, *redo_btn;
#endif
