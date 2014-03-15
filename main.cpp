/*
 * Flash Applikation for ASURO
 */

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <gtkmm.h>
#include "GtkAsuroFlash.h"
#include "ConAsuroFlash.h"

#define VERSION_STR "1.6.3"

void showHelp(void);

int main(int argc, char *argv[])
{		
	if(argc == 2 and !strcmp("--help", argv[1])) {
		showHelp();
		
		return 0;
	}
	
	if(argc == 2 || argc > 3) {
	    printf("wrong argument count\n");
        printf("use \"asuroflash --help\" for more information\n");
		
		return -1;
	}
	else if (argc == 1) {
		Gtk::Main kit(argc, argv);
		Glib::thread_init();

		GtkAsuroFlash asuro;
    	Gtk::Main::run(asuro);
	}
	else{
		ConAsuroFlash asuro;
	    
	    asuro.terminal = argv[1];
	    asuro.image_path = argv[2];
		
		asuro.Programm();
	}
}

void showHelp(void)
{
    printf("Usage: asuroflash Terminal File\n");
    printf("sents File through Terminal to the Asuro Robot\n");
	printf("Example: asuroflash /dev/ttyS0 main.hex");
}
