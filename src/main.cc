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
#include <gtk/gtk.h>
#include "Board.h"
#include "Callback.h"

Board board;

void activate (GtkApplication *app, void *data)
{
	GtkWidget *win;
	GtkWidget *da;

	win = gtk_application_window_new (app);
	da = gtk_drawing_area_new ();
	gtk_widget_set_size_request (da, 480, 480);
	gtk_container_add (GTK_CONTAINER (win), da);

	g_signal_connect (win, "destroy", G_CALLBACK (gtk_widget_destroy), win);
	g_signal_connect (da, "configure-event", G_CALLBACK (configure_cb), nullptr);
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
