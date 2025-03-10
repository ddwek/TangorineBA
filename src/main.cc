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

GtkWidget *main_window, *da, *time_da;
GtkWidget *new_game_btn, *clear_game_btn;
Board board;

void activate (GtkApplication *app, void *data)
{
	GResource *res;
	GError *error = nullptr;
	GtkBuilder *builder;

	res = g_resource_load (TANGORINEBA_DATADIR "data/ui.gresource", &error);
	g_resources_register (res);
	builder = gtk_builder_new_from_resource ("/org/gtk/TangorineBA/main-window.ui");
	g_resources_unregister (res);

	main_window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
	gtk_application_add_window (app, GTK_WINDOW (main_window));

	da = GTK_WIDGET (gtk_builder_get_object (builder, "da"));
	time_da = GTK_WIDGET (gtk_builder_get_object (builder, "time_da"));
	new_game_btn = GTK_WIDGET (gtk_builder_get_object (builder, "new_game_btn"));
	clear_game_btn = GTK_WIDGET (gtk_builder_get_object (builder, "clear_game_btn"));

	g_signal_connect (main_window, "destroy", G_CALLBACK (gtk_widget_destroy), main_window);
	g_signal_connect (da, "configure-event", G_CALLBACK (configure_cb), nullptr);
	g_signal_connect (da, "draw", G_CALLBACK (draw_cb), nullptr);
	g_signal_connect (da, "button-press-event", G_CALLBACK (button_press_cb), nullptr);
	gtk_widget_set_events (da, gtk_widget_get_events (da) | GDK_BUTTON_PRESS_MASK);
	g_signal_connect (time_da, "draw", G_CALLBACK (draw_timer_cb), nullptr);
	g_signal_connect (new_game_btn, "clicked", G_CALLBACK (new_game_cb), nullptr);
	g_signal_connect (clear_game_btn, "clicked", G_CALLBACK (clear_game_cb), nullptr);

	gtk_widget_show_all (GTK_WIDGET (main_window));
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
