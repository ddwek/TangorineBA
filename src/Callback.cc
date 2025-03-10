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
#include <sstream>
#include <string>
#include <list>
#include <set>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "common.h"
#include "Board.h"
#include "../test/Test.h"

typedef struct region_st {
	int x0;
	int y0;
	int x1;
	int y1;
} cell_region_t;

extern class Test test;
extern GtkWidget *da;
bool are_there_pending_events = false;
std::list<pending_events_t> redraw_cells;

class CallbackData {
public:
	CallbackData ();
	CallbackData (CallbackData&) = delete;
	CallbackData (CallbackData&&) = delete;
	CallbackData& operator= (CallbackData&) = delete;
	~CallbackData () = default;

	GtkApplication *get_app () const;
	GtkWindow *get_window () const;
	cairo_t *get_cr () const;
	cairo_t *get_timer_cr () const;
	cell_region_t *get_region (int n);
	int get_seconds () const;
	int get_minutes () const;

	void set_app (GtkApplication *app);
	void set_window (GtkWindow *window);
	void set_cr (cairo_t *cr);
	void set_timer_cr (cairo_t *timer_cr);
	void set_region (int nregion, int x0, int y0, int x1, int y1);
	void set_seconds (int seconds);
	void set_minutes (int minutes);

private:
	GtkApplication *app;
	GtkWindow *window;
	cairo_t *cr;
	cairo_t *timer_cr;
	cell_region_t region[36];
	int seconds;
	int minutes;
} cbdata;

CallbackData::CallbackData ()
{
	seconds = 0;
	minutes = 0;
}

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

cairo_t *CallbackData::get_timer_cr () const
{
	return timer_cr;
}

cell_region_t *CallbackData::get_region (int n)
{
	if (n < 36)
		return &region[n];

	return nullptr;
}

int CallbackData::get_seconds () const
{
	return this->seconds;
}

int CallbackData::get_minutes () const
{
	return this->minutes;
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

void CallbackData::set_timer_cr (cairo_t *timer_cr)
{
	this->timer_cr = timer_cr;
}

void CallbackData::set_region (int n, int x0, int y0, int x1, int y1)
{
	region[n].x0 = x0;
	region[n].y0 = y0;
	region[n].x1 = x1;
	region[n].y1 = y1;
}

void CallbackData::set_seconds (int seconds)
{
	this->seconds = seconds;
}

void CallbackData::set_minutes (int minutes)
{
	this->minutes = minutes;
}

int on_time_ticking_cb (gpointer data)
{
	int secs, mins;

	secs = cbdata.get_seconds ();
	mins = cbdata.get_minutes ();

	if (secs == 59) {
		secs = 0;
		mins++;
		if (mins == 100)
			return false;
	} else {
		secs++;
	}

	cbdata.set_seconds (secs);
	cbdata.set_minutes (mins);
	return true;
}

std::string get_timestamp ()
{
	int mins, secs;
	std::ostringstream ret;

	mins = cbdata.get_minutes ();
	if (mins < 10)
		ret << "0";
	ret << mins;
	secs = cbdata.get_seconds ();
	if (secs < 10)
		ret << "0";
	ret << secs;

	return ret.str ();
}

int draw_curr_time_leds (cairo_t *cr)
{
	int i, j, n, add;
	std::string ts;
	struct _GdkRGBA red = { 1.0, 0.0, 0.0, 1.0 };
	int digit[10][7][3] = {
				{
					// 0
					{ 1, 1, 1 },
					{ 1, 0, 1 },
					{ 1, 0, 1 },
					{ 1, 0, 1 },
					{ 1, 0, 1 },
					{ 1, 0, 1 },
					{ 1, 1, 1 },
				},
				{
					// 1
					{ 0, 0, 1 },
					{ 0, 0, 1 },
					{ 0, 0, 1 },
					{ 0, 0, 1 },
					{ 0, 0, 1 },
					{ 0, 0, 1 },
					{ 0, 0, 1 },
				},
				{
					// 2
					{ 1, 1, 1 },
					{ 0, 0, 1 },
					{ 0, 0, 1 },
					{ 1, 1, 1 },
					{ 1, 0, 0 },
					{ 1, 0, 0 },
					{ 1, 1, 1 },
				},
				{
					// 3
					{ 1, 1, 1 },
					{ 0, 0, 1 },
					{ 0, 0, 1 },
					{ 1, 1, 1 },
					{ 0, 0, 1 },
					{ 0, 0, 1 },
					{ 1, 1, 1 },
				},
				{
					// 4
					{ 1, 0, 1 },
					{ 1, 0, 1 },
					{ 1, 0, 1 },
					{ 1, 1, 1 },
					{ 0, 0, 1 },
					{ 0, 0, 1 },
					{ 0, 0, 1 },
				},
				{
					// 5
					{ 1, 1, 1 },
					{ 1, 0, 0 },
					{ 1, 0, 0 },
					{ 1, 1, 1 },
					{ 0, 0, 1 },
					{ 0, 0, 1 },
					{ 1, 1, 1 },
				},
				{
					// 6
					{ 1, 1, 1 },
					{ 1, 0, 0 },
					{ 1, 0, 0 },
					{ 1, 1, 1 },
					{ 1, 0, 1 },
					{ 1, 0, 1 },
					{ 1, 1, 1 },
				},
				{
					// 7
					{ 1, 1, 1 },
					{ 0, 0, 1 },
					{ 0, 0, 1 },
					{ 0, 0, 1 },
					{ 0, 0, 1 },
					{ 0, 0, 1 },
					{ 0, 0, 1 },
				},
				{
					// 8
					{ 1, 1, 1 },
					{ 1, 0, 1 },
					{ 1, 0, 1 },
					{ 1, 1, 1 },
					{ 1, 0, 1 },
					{ 1, 0, 1 },
					{ 1, 1, 1 },
				},
				{
					// 9
					{ 1, 1, 1 },
					{ 1, 0, 1 },
					{ 1, 0, 1 },
					{ 1, 1, 1 },
					{ 0, 0, 1 },
					{ 0, 0, 1 },
					{ 1, 1, 1 },
				},
	};

	ts = get_timestamp ();
	if (ts.length () != 4)
		return 0;

	cairo_save (cr);
	gdk_cairo_set_source_rgba (cr, &red);

	for (add = 0, n = 0; n < 2; n++) {
		for (i = 0; i < 7; i++) {
			for (j = 0; j < 3; j++) {
				if (!digit[ts[n] - '0'][i][j])
					continue;
				cairo_move_to (cr, 24 + digit[ts[n] - '0'][i][j] * j * 4 + add, 68 + digit[ts[n] - '0'][i][j] * i * 4);
				cairo_rel_line_to (cr, 2, 0);
				cairo_stroke (cr);
			}
		}
		add = 16 * (n + 1);
	}

	for (add = 48, n = 2; n < 4; n++) {
		for (i = 0; i < 7; i++) {
			for (j = 0; j < 3; j++) {
				if (!digit[ts[n] - '0'][i][j])
					continue;
				cairo_move_to (cr, 24 + digit[ts[n] - '0'][i][j] * j * 4 + add, 68 + digit[ts[n] - '0'][i][j] * i * 4);
				cairo_rel_line_to (cr, 2, 0);
				cairo_stroke (cr);
			}
		}
		add = 16 * (n + 2);
	}

	cairo_restore (cr);
	return 1;
}

void draw_ticking_leds (cairo_t *cr)
{
	struct _GdkRGBA fg;
	struct _GdkRGBA red = { 1.0, 0.0, 0.0, 1.0 };
	struct _GdkRGBA black = { 0.0, 0.0, 0.0, 1.0 };
	int tick[7][3] = {
				{ 0, 0, 0 },
				{ 0, 1, 0 },
				{ 0, 1, 0 },
				{ 0, 0, 0 },
				{ 0, 1, 0 },
				{ 0, 1, 0 },
				{ 0, 0, 0 },
	};

	if (cbdata.get_seconds () & 1)
		fg = black;
	else
		fg = red;

	cairo_save (cr);
	gdk_cairo_set_source_rgba (cr, &fg);
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 3; j++) {
			if (!tick[i][j])
				continue;
			cairo_move_to (cr, 24 + tick[i][j] * j * 4 + 32, 68 + tick[i][j] * i * 4);
			cairo_rel_line_to (cr, 0, 2);
			cairo_stroke (cr);
		}
	}
	cairo_restore (cr);
}

int draw_timer_cb (GtkWidget *widget, cairo_t *cr, void *user_data)
{
	struct _GdkRGBA black = { 0.0, 0.0, 0.0, 1.0 };
	struct _GdkRGBA bgcolor = { 0.2, 0.3, 0.4, 1.0 };

	cairo_save (cr);
	cbdata.set_timer_cr (cr);
	gdk_cairo_set_source_rgba (cr, &bgcolor);
	cairo_paint (cr);
	gdk_cairo_set_source_rgba (cr, &black);
	cairo_rectangle (cr, 5, 5, 110, 150);
	cairo_fill (cr);

	draw_curr_time_leds (cr);
	draw_ticking_leds (cr);
	cairo_restore (cr);

	gtk_widget_queue_draw (widget);
	return 0;
}

void clear_game_cb (GtkButton *btn)
{
	for (int i = 0; i < 36; i++) {
		shape_info_t r = board.get_standard_solution (i);
		board.set_user_guess (i, r.shape, r.flags);
		board.clear_hatching (i, true);
		board.clear_hatching (i, false);
	}
	redraw_cells.clear ();
	are_there_pending_events = true;

	gtk_widget_queue_draw (da);
}

void new_game_cb (GtkButton *btn)
{
	redraw_cells.clear ();
	are_there_pending_events = true;
	board.new_game ();
}

int configure_cb (GtkWidget *widget, GdkEventConfigure *event, void *data)
{
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 6; j++)
			cbdata.set_region (i * 6 + j, j * 80, i * 80, (j + 1) * 80, (i + 1) * 80);

	g_timeout_add_seconds (1, on_time_ticking_cb, widget);
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
