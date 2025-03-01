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
Board board (true, "./zero_board_two_regular_cells_with_horizontal_equal_shapes");

int on_tick_cb (GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data)
{
	static int i = 2, j = 2, cnt = 0;
	static shape_t shape_0 = SHAPE_MOON, shape_1 = SHAPE_MOON;
	bool finished = false;
	int ret = -1;

	if (!board.is_configured ())
		if (!test.is_input_already_parsed () || !test.is_expected_already_parsed ())
			return -1;

	if (!(gdk_frame_clock_get_frame_counter (frame_clock) & 0xf)) {
		if (board.get_user_guess (i * 6 + j).shape == SHAPE_SUN) {
			shape_0 = SHAPE_MOON;
			shape_1 = SHAPE_MOON;
		} else {
			shape_0 = SHAPE_SUN;
			shape_1 = SHAPE_SUN;
		}

		pending_event.flags = board.get_standard_solution (i * 6 + j).flags;
		shape_info_t r = board.get_user_guess (i * 6 + j);
		if (r.flags.imm)
			pending_event.flags.imm = 1;
		if (r.flags.top)
			pending_event.flags.top = 1;
		if (r.flags.right)
			pending_event.flags.right = 1;
		if (r.flags.bottom)
			pending_event.flags.bottom = 1;
		if (r.flags.left)
			pending_event.flags.left = 1;

		pending_event.ncell = i * 6 + j + 0;
		pending_event.shape = shape_0;
		pending_event.flags = board.get_standard_solution (i * 6 + j + 0).flags;
		board.set_user_guess (i * 6 + j + 0, pending_event.shape, pending_event.flags);
		redraw_cells.push_back (pending_event);
		cnt++;

		pending_event.ncell = i * 6 + j + 1;
		pending_event.shape = shape_1;
		pending_event.flags = board.get_standard_solution (i * 6 + j + 1).flags;
		board.set_user_guess (i * 6 + j + 1, pending_event.shape, pending_event.flags);
		redraw_cells.push_back (pending_event);
		cnt++;

		are_there_pending_events = true;
		if (cnt == 6)
			finished = true;

		for (int t = 0; t < 6; t++)
			board.validate_row (t);
		for (int t = 0; t < 6; t++)
			board.validate_col (t);
		gtk_widget_queue_draw (widget);

		if ((cnt / 2 - 1) < test.get_total_steps_to_parse ()) {
			board.print (true, false, cnt / 2 - 1);
			ret = test.pass (cnt / 2 - 1);
		}
	} else {
		gdk_frame_clock_get_frame_counter (frame_clock);
	}

	if (finished) {
		if (ret)
			exit (EXIT_SUCCESS);
		else
			exit (EXIT_FAILURE);
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
