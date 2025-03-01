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
#include <list>
#include <gtk/gtk.h>
#include "../src/common.h"
#include "../src/Board.h"
#include "../src/Callback.h"
#include "Test.h"

extern bool are_there_pending_events;
extern std::list<pending_events_t> redraw_cells;
extern pending_events_t pending_event;
extern class CallbackData cbdata;
extern class Test test;
Board board (true, "./two_regular_cells_with_vertical_diff_shapes");

int on_tick_cb (GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data)
{
	static int i = 0, j = 0, cnt = 0, stage1 = 0;
	static shape_t new_shape = SHAPE_EMPTY;
	static bool finished = false;
	int ret = -1, now = 0;

	if (!board.is_configured ())
		if (!test.is_input_already_parsed () || !test.is_expected_already_parsed ())
			return -1;

	if (!(gdk_frame_clock_get_frame_counter (frame_clock) & 0x3)) {
		pending_event.ncell = i * 6 + j;
		pending_event.shape = new_shape != SHAPE_EMPTY ? new_shape : board.get_standard_solution (i * 6 + j).shape;
		pending_event.flags = board.get_standard_solution (i * 6 + j).flags;
		board.set_user_guess (pending_event.ncell, pending_event.shape, pending_event.flags);
		redraw_cells.push_back (pending_event);
		are_there_pending_events = true;
		cnt++;

		j++;
		if (j > 5) {
			j = 0;
			i++;
		}

		if (i > 5) {
			i = 1;
			j = 3;
			new_shape = SHAPE_MOON;
			for (int t = 0; t < 6; t++)
				board.validate_row (t);
			for (int t = 0; t < 6; t++)
				board.validate_col (t);
			stage1 = 1;
			goto check_step;
		}

		if (stage1) {
			i = 1;
			j = 3;
			new_shape = SHAPE_SUN;
			for (int t = 0; t < 6; t++)
				board.validate_row (t);
			for (int t = 0; t < 6; t++)
				board.validate_col (t);
			goto check_step;
		}
check_step:
		if (cnt - 1 == test.get_total_steps_to_parse ()) {
			finished = true;
			now = gdk_frame_clock_get_frame_counter (frame_clock);
		}
		if (cnt - 1 < test.get_total_steps_to_parse ()) {
			board.print (true, true, cnt - 1);
			ret = test.pass (cnt - 1);
		}
		gtk_widget_queue_draw (widget);
	} else {
		gdk_frame_clock_get_frame_counter (frame_clock);
	}

	if (finished) {
		if (gdk_frame_clock_get_frame_counter (frame_clock) - now > 120) {
			if (ret)
				exit (EXIT_SUCCESS);
			else
				exit (EXIT_FAILURE);
		}
	}

	return ret;
}

int configure_testing_cb (GtkWidget *widget, GdkEventConfigure *event, void *data)
{
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 6; j++)
			cbdata.set_region (i * 6 + j, j * 80, i * 80, (j + 1) * 80, (i + 1) * 80);

	in_parsed_t *p = test.get_input_parsed ();
	board.set_seed (p->seed);
	board.set_immutable_cells (p->imm);
	gtk_widget_add_tick_callback (GTK_WIDGET (widget), on_tick_cb, nullptr, nullptr);
	return 0;
}

void activate (GtkApplication *app, void *data)
{
	GtkWidget *win;
	GtkWidget *da;

	win = gtk_application_window_new (app);
	da = gtk_drawing_area_new ();
	gtk_widget_set_size_request (da, 480, 480);
	gtk_container_add (GTK_CONTAINER (win), da);

	g_signal_connect (win, "destroy", G_CALLBACK (gtk_widget_destroy), win);
	g_signal_connect (da, "configure-event", G_CALLBACK (configure_testing_cb), nullptr);
	g_signal_connect (da, "draw", G_CALLBACK (draw_cb), nullptr);
	g_signal_connect (da, "button-press-event", G_CALLBACK (button_press_cb), nullptr);
	gtk_widget_set_events (da, gtk_widget_get_events (da) | GDK_BUTTON_PRESS_MASK);

	gtk_widget_show_all (win);
}

int main (int argc, char **argv)
{
	GtkApplication *app;
	int status;

	app = gtk_application_new ("org.gtk.TangorineBA", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	return status;
}
