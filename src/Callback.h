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
#ifndef _CALLBACK_H_
#define _CALLBACK_H_	1
#include <gtk/gtk.h>
#include "common.h"

typedef struct region_st {
	int x0;
	int y0;
	int x1;
	int y1;
} cell_region_t;

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
};

int on_tick_cb (GtkWidget *widget, GdkFrameClock *frame_clock, gpointer user_data);
int draw_timer_cb (GtkWidget *widget, cairo_t *timer_cr, gpointer data);
void clear_game_cb (GtkButton *btn);
void new_game_cb (GtkButton *btn);
int configure_cb (GtkWidget *widget, GdkEventConfigure *event, void *data);
int draw_cb (GtkWidget *widget, cairo_t *cr, gpointer data);
bool button_press_cb (GtkWidget *widget, GdkEventButton *button, void *data);
#endif
