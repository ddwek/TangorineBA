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
#include <string>
#include <list>
#include <set>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "common.h"
#include "Board.h"
#include "../test/Test.h"

extern class Test test;

bool are_there_pending_events = false;
typedef struct pending_events_st {
	int ncell;
	shape_t shape;
	bm_flags_t flags;
} pending_events_t;
std::list<pending_events_t> redraw_cells;

typedef struct region_st {
	int x0;
	int y0;
	int x1;
	int y1;
} cell_region_t;

class CallbackData {
public:
	CallbackData () = default;
	CallbackData (CallbackData&) = delete;
	CallbackData (CallbackData&&) = delete;
	CallbackData& operator= (CallbackData&) = delete;
	~CallbackData () = default;

	GtkApplication *get_app () const;
	GtkWindow *get_window () const;
	cairo_t *get_cr () const;
	cell_region_t *get_region (int n);

	void set_app (GtkApplication *app);
	void set_window (GtkWindow *window);
	void set_cr (cairo_t *cr);
	void set_region (int nregion, int x0, int y0, int x1, int y1);

private:
	GtkApplication *app;
	GtkWindow *window;
	cairo_t *cr;
	cell_region_t region[36];
} cbdata;

GtkApplication *CallbackData::get_app () const
{
	return app;
}

GtkWindow *CallbackData::get_window () const
{
	return window;
}

cairo_t *CallbackData::get_cr () const
{
	return cr;
}

cell_region_t *CallbackData::get_region (int n)
{
	if (n < 36)
		return &region[n];

	return nullptr;
}

void CallbackData::set_app (GtkApplication *app)
{
	this->app = app;
}

void CallbackData::set_window (GtkWindow *window)
{
	this->window = window;
}

void CallbackData::set_cr (cairo_t *cr)
{
	this->cr = cr;
}

void CallbackData::set_region (int n, int x0, int y0, int x1, int y1)
{
	region[n].x0 = x0;
	region[n].y0 = y0;
	region[n].x1 = x1;
	region[n].y1 = y1;
}

int configure_cb (GtkWidget *widget, GdkEventConfigure *event, void *data)
{
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 6; j++)
			cbdata.set_region (i * 6 + j, j * 80, i * 80, (j + 1) * 80, (i + 1) * 80);
	return 0;
}

int draw_cb (GtkWidget *widget, cairo_t *cr, void *user_data)
{
	struct _GdkRGBA bgcolor = { 0.2, 0.3, 0.4, 1.0 };
	std::list<pending_events_t>::iterator iter;

	cairo_save (cr);
	cbdata.set_cr (cr);
	gdk_cairo_set_source_rgba (cr, &bgcolor);
	cairo_paint (cr);
	board.draw_cells (cr);
	if (are_there_pending_events) {
		for (iter = redraw_cells.begin (); iter != redraw_cells.end (); iter++) {
			board.draw_immutable_cells ();
			board.draw_shape (iter->ncell / 6, iter->ncell % 6, iter->shape);
			if (board.can_draw_hatching (iter->ncell)) {
				board.draw_hatching (iter->ncell);
				board.draw_hatching_on_immutable ();
			}
		}

		if (board.get_game_over ())
			board.show_congrats ();
		are_there_pending_events = false;
	}
	board.draw_constraints ();
	cairo_restore (cr);

	return 0;
}

bool button_press_cb (GtkWidget *widget, GdkEventButton *event, void *user_data)
{
	static std::set<int> s;
	shape_t new_guess = SHAPE_EMPTY;
	pending_events_t pending_event;
	std::set<int>::iterator iter;

	for (int i = 0; i < 36; i++) {
		if (event->x > cbdata.get_region (i)->x0 && event->x < cbdata.get_region (i)->x1 &&
		    event->y > cbdata.get_region (i)->y0 && event->y < cbdata.get_region (i)->y1) {
			switch (board.get_user_guess (i).shape) {
			case SHAPE_SUN:
				new_guess = SHAPE_MOON;
				break;
			case SHAPE_MOON:
				new_guess = SHAPE_EMPTY;
				break;
			case SHAPE_EMPTY:
				new_guess = SHAPE_SUN;
				break;
			};

			if (!board.is_immutable (i)) {
				are_there_pending_events = true;
				pending_event.ncell = i;
				pending_event.shape = new_guess;
				pending_event.flags = board.get_standard_solution (i).flags;
				board.set_user_guess (i, new_guess, pending_event.flags);
				redraw_cells.push_back (pending_event);
			} else {
				are_there_pending_events = false;
			}

			board.validate_row (i / 6);
			board.validate_col (i % 6);
			if (are_there_pending_events) {
				if (board.get_user_guess (i).shape != SHAPE_EMPTY)
					s.insert (i);
empty_shape_erased:
				for (iter = s.begin (); iter != s.end (); iter++) {
					if (board.get_user_guess (*iter).shape == SHAPE_EMPTY) {
						// Caught *iter with an empty shape! Erasing...
						s.erase (iter);
						goto empty_shape_erased;
					}
				}
				if (s.size () == 30)
					board.set_game_over (true);
				gtk_widget_queue_draw (widget);
			}
		}
	}

	return true;
}
