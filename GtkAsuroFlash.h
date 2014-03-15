/*
 * GtkAsuroFlash
 * This file is part of Asuro Flash Tool
 *
 * Copyright (C) 2006 - Pavel Rojtberg
 *
 * Asuro Flash Tool is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Asuro Flash Tool is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Asuro Flash Tool; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
 
#ifndef GTKASUROFLASH_H
#define GTKASUROFLASH_H

#include "AsuroFlash.h"
#include <gtkmm.h>
#include <stdio.h>
#include <iostream>

class GtkAsuroFlash: public Gtk::Window, public AsuroFlash
{
public:
	GtkAsuroFlash();
	void normal_out(std::string message);
	void warning_out(std::string message);
	void success_out(std::string message);
	void error_out(std::string message);
	void progress(float fraction);
	void flash_clicked();
	void Programm();
protected:
	Glib::RefPtr<Gtk::TextBuffer> text_buffer;
	Gtk::VBox main_box;
	Gtk::Table settings_table;
	Gtk::HBox bottom_box;
	Gtk::Label term_label;
	Gtk::Label image_label;
	Gtk::ComboBoxText term_cbox;
	Gtk::FileChooserButton image_chooser;
	Gtk::FileFilter hex_filter;
	Gtk::ProgressBar progress_bar;
	Gtk::ScrolledWindow scrolled_win;
	Gtk::TextView text_view;
	Gtk::Image flash_image;
	Gtk::Button flash_button;
	Gtk::Button close_button;
	void text_view_size_request(Gtk::Requisition *req);
	void text_buffer_print(Glib::ustring message, Glib::ustring tag);
	void set_term_cbox(void);
};
#endif
