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
 
#include "GtkAsuroFlash.h"
#include <gtkmm/icontheme.h>
#include <sys/stat.h>
#include <string.h>

#define TTY_LIST_SIZE 3

GtkAsuroFlash::GtkAsuroFlash():
main_box(false, 5),
settings_table(2, 2),
term_label("<b>Terminal:</b>"),
image_label("<b>Hex Image:</b>"),
image_chooser("Select Hex Image"),
flash_button("_Flash Asuro"),
close_button(Gtk::Stock::CLOSE)
{
	text_buffer = Gtk::TextBuffer::create();
	text_buffer->create_tag("normal");
	
	Glib::RefPtr<Gtk::TextTag> success_tag = text_buffer->create_tag("success");
	success_tag->property_foreground() = "darkgreen";
	success_tag->property_weight() = Pango::WEIGHT_BOLD;
	
	Glib::RefPtr<Gtk::TextTag> warning_tag = text_buffer->create_tag("warning");
	warning_tag->property_foreground() = "orange";
	warning_tag->property_weight() = Pango::WEIGHT_BOLD;
	
	Glib::RefPtr<Gtk::TextTag> error_tag = text_buffer->create_tag("error");
	error_tag->property_foreground() = "red";
	error_tag->property_weight() = Pango::WEIGHT_BOLD;

	this->set_title("Asuro Flash Tool");
	this->set_default_size(350, 400);
	this->set_border_width(10);

	Glib::RefPtr<Gtk::IconTheme> icon_theme = Gtk::IconTheme::get_default();
	
	try {
		this->set_icon(icon_theme->load_icon("network-transmit", 16, ~Gtk::ICON_LOOKUP_NO_SVG));
	} catch(Gtk::IconThemeError err) {
		std::cout << "error loading window icon" << std::endl;
	}
	
	settings_table.set_col_spacing(0, 30);
	settings_table.set_row_spacing(0, 5);
	
	term_label.set_use_markup(true);
	term_label.set_alignment(0, 0);
	settings_table.attach(term_label, 0, 1, 0, 1, Gtk::FILL, ~Gtk::FILL);
	
	set_term_cbox();

	settings_table.attach(term_cbox, 1, 2, 0, 1, Gtk::FILL|Gtk::EXPAND, ~Gtk::EXPAND);
	
	image_label.set_use_markup(true);
	image_label.set_alignment(0, 0);
	settings_table.attach(image_label, 0, 1, 1, 2, Gtk::FILL, ~Gtk::FILL);
	
	hex_filter.add_mime_type("text/plain");
	image_chooser.set_filter(hex_filter);
	
	if(this->image_path != NULL and g_file_test(this->image_path, G_FILE_TEST_EXISTS))
		image_chooser.set_filename(this->image_path);

	settings_table.attach(image_chooser, 1, 2, 1, 2, Gtk::FILL|Gtk::EXPAND, ~Gtk::EXPAND);
	
	main_box.pack_start(settings_table, false, false);
	
	progress_bar.set_text("0%");
	main_box.pack_start(progress_bar, false, false);
	
	scrolled_win.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
	scrolled_win.set_shadow_type(Gtk::SHADOW_IN);
	
	text_view.set_buffer(text_buffer);
	text_view.set_editable(false);
	text_view.signal_size_request().connect(sigc::mem_fun(*this, &GtkAsuroFlash::text_view_size_request));
	
	scrolled_win.add(text_view);
	main_box.pack_start(scrolled_win, true, true);
	
	flash_image.set_from_icon_name("document-save", Gtk::ICON_SIZE_BUTTON);
	flash_button.set_image(flash_image);
	flash_button.set_use_underline(true);
	flash_button.signal_clicked().connect(sigc::mem_fun(*this, &GtkAsuroFlash::flash_clicked));
	bottom_box.pack_start(flash_button, false, false);
	
	close_button.signal_clicked().connect(sigc::ptr_fun(&Gtk::Main::quit));
	bottom_box.pack_end(close_button, false, false);
	
	main_box.pack_start(bottom_box, false, false);
	
	main_box.show_all();
	this->add(main_box);
}

void GtkAsuroFlash::set_term_cbox() {
	int i;
	char dev[20];
	const char *tty_list[] = {"ttyS0", "ttyS1", "ttyUSB0"};
	
	for (i = 0; i < TTY_LIST_SIZE; ++i) {
		sprintf(dev, "/dev/%s", tty_list[i]);
		
		if(!g_file_test(dev, G_FILE_TEST_EXISTS))
			continue;
		
		term_cbox.append_text(tty_list[i]);
		
		if(this->terminal != NULL and !strcmp(this->terminal, dev))
			term_cbox.set_active(i);
	}
	
	if(this->terminal == NULL) {
		term_cbox.set_active(0);
		return;
	}
	
	if(term_cbox.get_active_row_number() == -1) {
		term_cbox.append_text(g_path_get_basename(this->terminal));
		term_cbox.set_active(i);
	}	
}

void GtkAsuroFlash::Programm()
{
	AsuroFlash::Programm();
	
	this->flash_button.set_sensitive(true);
}

void GtkAsuroFlash::text_buffer_print(Glib::ustring message, Glib::ustring tag) {
	this->text_buffer->insert_with_tag(this->text_buffer->end(), message, tag);
}

void GtkAsuroFlash::flash_clicked()
{
    this->text_buffer->set_text("");
	asprintf(&terminal, "/dev/%s", this->term_cbox.get_active_text().c_str());
	image_path = strdup(this->image_chooser.get_filename().c_str());
	
	Glib::Thread::create(sigc::mem_fun(*this, &GtkAsuroFlash::Programm), false);
	
	this->flash_button.set_sensitive(false);
}

void GtkAsuroFlash::text_view_size_request(Gtk::Requisition *req)
{
	Gtk::Adjustment *adjustment = this->scrolled_win.get_vadjustment();
	adjustment->set_value(adjustment->get_upper());
	this->scrolled_win.set_vadjustment(adjustment);
}

void GtkAsuroFlash::progress(float fraction)
{
	char text[5];
	
	sprintf(text, "%.0f%%", (fraction * 100));

	Glib::signal_idle().connect(
		sigc::bind_return(
			sigc::bind(sigc::mem_fun(this->progress_bar, &Gtk::ProgressBar::set_text), std::string(text)), false));

	Glib::signal_idle().connect(
		sigc::bind_return(
			sigc::bind(sigc::mem_fun(this->progress_bar, &Gtk::ProgressBar::set_fraction), fraction), false));
}

void GtkAsuroFlash::normal_out(std::string message)
{
		Glib::signal_idle().connect(
		sigc::bind_return(
		sigc::bind(sigc::mem_fun(*this, &GtkAsuroFlash::text_buffer_print), message, "normal"), false));
}

void GtkAsuroFlash::warning_out(std::string message)
{
		Glib::signal_idle().connect(
		sigc::bind_return(
		sigc::bind(sigc::mem_fun(*this, &GtkAsuroFlash::text_buffer_print), message, "warning"), false));
}

void GtkAsuroFlash::success_out(std::string message)
{
		Glib::signal_idle().connect(
		sigc::bind_return(
		sigc::bind(sigc::mem_fun(*this, &GtkAsuroFlash::text_buffer_print), message+"\n", "success"), false));
}

void GtkAsuroFlash::error_out(std::string message)
{
		Glib::signal_idle().connect(
		sigc::bind_return(
		sigc::bind(sigc::mem_fun(*this, &GtkAsuroFlash::text_buffer_print), message+"\n", "error"), false));
}
