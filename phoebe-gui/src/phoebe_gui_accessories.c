#include <stdlib.h>
#include <sys/stat.h>

#include <phoebe/phoebe.h>

#include "phoebe_gui_build_config.h"

#include "phoebe_gui_accessories.h"
#include "phoebe_gui_callbacks.h"
#include "phoebe_gui_types.h"
#include "phoebe_gui_global.h"

gchar   *PHOEBE_FILENAME = NULL;
gboolean PHOEBE_FILEFLAG = FALSE;

gchar   *PHOEBE_DIRNAME = NULL;
gboolean PHOEBE_DIRFLAG = FALSE;

void gui_set_text_view_from_file (GtkWidget *text_view, gchar *filename)
{
	/*
	 * This function fills the text view text_view with the first maxlines lines
	 * of a file filename.
	 */

	GtkTextBuffer *text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
	GtkTextIter iter;

	FILE *file = fopen(filename, "r");

	if(file){
		char line[255];
		int i=1;
		int maxlines=50;

		fgets (line, 255, file);
		gtk_text_buffer_set_text (text_buffer, line, -1);

		gtk_text_buffer_get_iter_at_line (text_buffer, &iter, i);
			while(!feof (file) && i<maxlines){
				fgets (line, 255, file);
				gtk_text_buffer_insert (text_buffer, &iter, line, -1);
				i++;
			}

		if(!feof (file))gtk_text_buffer_insert (text_buffer, &iter, "...", -1);

		fclose(file);
	}
}

void tmp_circumvent_delete_event (GtkWidget *widget, gpointer user_data)
{
	GtkWidget *box 		= g_object_get_data (G_OBJECT (widget), "data_box");
	GtkWidget *parent 	= g_object_get_data (G_OBJECT (widget), "data_parent");
	gboolean *flag 		= g_object_get_data (G_OBJECT (widget), "data_flag");

	GtkWidget *window = gtk_widget_get_parent(box);

	gtk_widget_reparent(box, parent);
	gtk_widget_destroy(window);

	*flag = !(*flag);
}


void gui_detach_box_from_parent (GtkWidget *box, GtkWidget *parent, gboolean *flag, gchar *window_title, gint x, gint y)
{
	/*
	 * This function detaches the box from its parent. If the flag=FALSE, than
	 * it creates a new window and packs the box inside the window, otherwise
	 * it packs the box in its original place inside the main window.
	 */

	GtkWidget *window;
	gchar *glade_pixmap_file = g_build_filename (PHOEBE_GLADE_PIXMAP_DIR, "ico.png", NULL);

	if(*flag){
		window = gtk_widget_get_parent (box);

		gtk_widget_reparent(box, parent);
		gtk_widget_destroy(window);
	}
	else{
		window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		g_object_set_data (G_OBJECT (window), "data_box", 		(gpointer) box);
		g_object_set_data (G_OBJECT (window), "data_parent", 	(gpointer) parent);
		g_object_set_data (G_OBJECT (window), "data_flag",		(gpointer) flag);

		gtk_window_set_icon (GTK_WINDOW(window), gdk_pixbuf_new_from_file(glade_pixmap_file, NULL));
		gtk_window_set_title (GTK_WINDOW (window), window_title);
		gtk_widget_reparent(box, window);
		gtk_widget_set_size_request (window, x, y);
		gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
		g_signal_connect (GTK_WIDGET(window), "delete-event", G_CALLBACK (tmp_circumvent_delete_event), NULL);
		gtk_widget_show_all (window);
	}
	*flag = !(*flag);
}

int gui_open_parameter_file()
{
	GtkWidget *dialog;
	gchar *glade_pixmap_file = g_build_filename (PHOEBE_GLADE_PIXMAP_DIR, "ico.png", NULL);
	int status = 0;

	dialog = gtk_file_chooser_dialog_new ("Open PHOEBE parameter file",
										  GTK_WINDOW(gui_widget_lookup("phoebe_window")->gtk),
										  GTK_FILE_CHOOSER_ACTION_OPEN,
										  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
										  GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
										  NULL);

	if(PHOEBE_DIRFLAG)
		gtk_file_chooser_set_current_folder((GtkFileChooser*)dialog, PHOEBE_DIRNAME);
	else{
		gchar *dir;
		phoebe_config_entry_get("PHOEBE_DATA_DIR", &dir);
		gtk_file_chooser_set_current_folder((GtkFileChooser*)dialog, dir);
		g_free(dir);
	}

    gtk_window_set_icon (GTK_WINDOW(dialog), gdk_pixbuf_new_from_file(glade_pixmap_file, NULL));

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT){
		char *filename;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		status = phoebe_open_parameter_file(filename);

		PHOEBE_FILEFLAG = TRUE;
		PHOEBE_FILENAME = strdup(filename);
		g_free (filename);

		PHOEBE_DIRFLAG = TRUE;
		PHOEBE_DIRNAME = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (dialog));
	}

	gtk_widget_destroy (dialog);

	return status;
}

gchar *gui_get_filename_with_overwrite_confirmation(GtkWidget *dialog, char *gui_confirmation_title)
{
	/* Returns a valid filename or NULL if the user canceled 
	   Replaces functionality of gtk_file_chooser_set_do_overwrite_confirmation avaliable in Gtk 2.8 (PHOEBE only requires 2.6)
	*/
	gchar *filename;

	while (TRUE) {
		if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT){
			filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
			if (!phoebe_filename_exists(filename))
				/* file does not exist */
				return filename;
			if(gui_warning(gui_confirmation_title, "This file already exists. Do you want to replace it?")){
				/* file may be overwritten */
				return filename;
			}
			/* user doesn't want to overwrite, display the dialog again */
			g_free (filename);
		}
		else return NULL;
	}
}

int gui_save_parameter_file()
{
	GtkWidget *dialog;
	gchar *glade_pixmap_file = g_build_filename (PHOEBE_GLADE_PIXMAP_DIR, "ico.png", NULL);
	int status = 0;

	dialog = gtk_file_chooser_dialog_new ("Save PHOEBE parameter file",
										  GTK_WINDOW(gui_widget_lookup("phoebe_window")->gtk),
										  GTK_FILE_CHOOSER_ACTION_SAVE,
										  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
										  GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
										  NULL);

	if(PHOEBE_DIRFLAG)
		gtk_file_chooser_set_current_folder((GtkFileChooser*)dialog, PHOEBE_DIRNAME);
	else{
		gchar *dir;
		phoebe_config_entry_get("PHOEBE_DATA_DIR", &dir);
		gtk_file_chooser_set_current_folder((GtkFileChooser*)dialog, dir);
		g_free (dir);
	}

	/* gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE); */
    gtk_window_set_icon (GTK_WINDOW(dialog), gdk_pixbuf_new_from_file(glade_pixmap_file, NULL));

	gchar *filename = gui_get_filename_with_overwrite_confirmation(dialog, "Save PHOEBE parameter file");
	if (filename){
		status = phoebe_save_parameter_file(filename);

		PHOEBE_FILEFLAG = TRUE;
		PHOEBE_FILENAME = strdup(filename);
		g_free (filename);

		PHOEBE_DIRFLAG = TRUE;
		PHOEBE_DIRNAME = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (dialog));
	}

	gtk_widget_destroy (dialog);

	return status;
}

int gui_show_configuration_dialog()
{
	int status = 0;

	gchar     *glade_xml_file					= g_build_filename     	(PHOEBE_GLADE_XML_DIR, "phoebe_settings.glade", NULL);
	gchar     *glade_pixmap_file				= g_build_filename     	(PHOEBE_GLADE_PIXMAP_DIR, "ico.png", NULL);

	GladeXML  *phoebe_settings_xml				= glade_xml_new			(glade_xml_file, NULL, NULL);

	GtkWidget *phoebe_settings_dialog			= glade_xml_get_widget	(phoebe_settings_xml, "phoebe_settings_dialog");
	GtkWidget *basedir_filechooserbutton		= glade_xml_get_widget	(phoebe_settings_xml, "phoebe_settings_configuration_basedir_filechooserbutton");
	GtkWidget *srcdir_filechooserbutton			= glade_xml_get_widget	(phoebe_settings_xml, "phoebe_settings_configuration_srcdir_filechooserbutton");
	GtkWidget *defaultsdir_filechooserbutton	= glade_xml_get_widget	(phoebe_settings_xml, "phoebe_settings_configuration_defaultsdir_filechooserbutton");
	GtkWidget *workingdir_filechooserbutton		= glade_xml_get_widget	(phoebe_settings_xml, "phoebe_settings_configuration_workingdir_filechooserbutton");
	GtkWidget *datadir_filechooserbutton		= glade_xml_get_widget	(phoebe_settings_xml, "phoebe_settings_configuration_datadir_filechooserbutton");
	GtkWidget *ptfdir_filechooserbutton			= glade_xml_get_widget	(phoebe_settings_xml, "phoebe_settings_configuration_ptfdir_filechooserbutton");

	GtkWidget *vh_checkbutton					= glade_xml_get_widget	(phoebe_settings_xml, "phoebe_settings_vh_checkbutton");
	GtkWidget *vh_lddir_filechooserbutton		= glade_xml_get_widget	(phoebe_settings_xml, "phoebe_settings_vh_lddir_filechooserbutton");

	GtkWidget *kurucz_checkbutton				= glade_xml_get_widget	(phoebe_settings_xml, "phoebe_settings_kurucz_checkbutton");
	GtkWidget *kurucz_filechooserbutton			= glade_xml_get_widget	(phoebe_settings_xml, "phoebe_settings_kurucz_filechooserbutton");

	GtkWidget *confirm_on_overwrite_checkbutton = glade_xml_get_widget (phoebe_settings_xml, "phoebe_settings_confirmation_save_checkbutton");

	gchar 		*dir;
	gboolean	toggle;
	gint 		result;

	g_object_unref (phoebe_settings_xml);

	phoebe_config_entry_get ("PHOEBE_BASE_DIR", &dir);
	gtk_file_chooser_set_filename((GtkFileChooser*)basedir_filechooserbutton, dir);
	phoebe_config_entry_get ("PHOEBE_SOURCE_DIR", &dir);
	gtk_file_chooser_set_filename((GtkFileChooser*)srcdir_filechooserbutton, dir);
	phoebe_config_entry_get ("PHOEBE_DEFAULTS_DIR", &dir);
	gtk_file_chooser_set_filename((GtkFileChooser*)defaultsdir_filechooserbutton, dir);
	phoebe_config_entry_get ("PHOEBE_TEMP_DIR", &dir);
	gtk_file_chooser_set_filename((GtkFileChooser*)workingdir_filechooserbutton, dir);
	phoebe_config_entry_get ("PHOEBE_DATA_DIR", &dir);
	gtk_file_chooser_set_filename((GtkFileChooser*)datadir_filechooserbutton, dir);
	phoebe_config_entry_get ("PHOEBE_PTF_DIR", &dir);
	gtk_file_chooser_set_filename((GtkFileChooser*)ptfdir_filechooserbutton, dir);

	g_signal_connect(G_OBJECT(vh_checkbutton), "toggled", G_CALLBACK(on_phoebe_settings_checkbutton_toggled), (gpointer)vh_lddir_filechooserbutton);
	g_signal_connect(G_OBJECT(kurucz_checkbutton), "toggled", G_CALLBACK(on_phoebe_settings_checkbutton_toggled), (gpointer)kurucz_filechooserbutton);
	g_signal_connect(G_OBJECT(confirm_on_overwrite_checkbutton), "toggled", G_CALLBACK(on_phoebe_settings_confirmation_save_checkbutton_toggled), NULL);

	gtk_window_set_icon (GTK_WINDOW (phoebe_settings_dialog), gdk_pixbuf_new_from_file (glade_pixmap_file, NULL));
	gtk_window_set_title (GTK_WINDOW(phoebe_settings_dialog), "PHOEBE - Settings");

	phoebe_config_entry_get ("PHOEBE_LD_SWITCH", &toggle);
	if (toggle){
			phoebe_config_entry_get ("PHOEBE_LD_DIR", &dir);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(vh_checkbutton), TRUE);
			gtk_widget_set_sensitive (vh_lddir_filechooserbutton, TRUE);
			gtk_file_chooser_set_filename((GtkFileChooser*)vh_lddir_filechooserbutton, dir);
	}

	phoebe_config_entry_get ("PHOEBE_KURUCZ_SWITCH", &toggle);
	if (toggle){
			phoebe_config_entry_get ("PHOEBE_KURUCZ_DIR", &dir);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(kurucz_checkbutton), TRUE);
			gtk_widget_set_sensitive (kurucz_filechooserbutton, TRUE);
			gtk_file_chooser_set_filename((GtkFileChooser*)kurucz_filechooserbutton, dir);
	}

	phoebe_config_entry_get ("GUI_CONFIRM_ON_OVERWRITE", &toggle);
	if (toggle)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (confirm_on_overwrite_checkbutton), 1);
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (confirm_on_overwrite_checkbutton), 0);

	result = gtk_dialog_run ((GtkDialog*)phoebe_settings_dialog);
	switch (result){
		case GTK_RESPONSE_OK:{
			phoebe_config_entry_set ("PHOEBE_BASE_DIR", 	gtk_file_chooser_get_filename ((GtkFileChooser*)basedir_filechooserbutton));
			phoebe_config_entry_set ("PHOEBE_SOURCE_DIR",	gtk_file_chooser_get_filename ((GtkFileChooser*)srcdir_filechooserbutton));
			phoebe_config_entry_set ("PHOEBE_DEFAULTS_DIR", gtk_file_chooser_get_filename ((GtkFileChooser*)defaultsdir_filechooserbutton));
			phoebe_config_entry_set ("PHOEBE_TEMP_DIR", 	gtk_file_chooser_get_filename ((GtkFileChooser*)workingdir_filechooserbutton));
			phoebe_config_entry_set ("PHOEBE_DATA_DIR", 	gtk_file_chooser_get_filename ((GtkFileChooser*)datadir_filechooserbutton));
			phoebe_config_entry_set ("PHOEBE_PTF_DIR", 		gtk_file_chooser_get_filename ((GtkFileChooser*)ptfdir_filechooserbutton));

			phoebe_config_entry_get ("PHOEBE_LD_SWITCH", &toggle);
			if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(vh_checkbutton))){
				phoebe_config_entry_set ("PHOEBE_LD_SWITCH",	TRUE);
				phoebe_config_entry_set ("PHOEBE_LD_DIR",		gtk_file_chooser_get_filename ((GtkFileChooser*)vh_lddir_filechooserbutton));
				phoebe_load_ld_tables();
			}
			else if (!toggle)
				phoebe_config_entry_set ("PHOEBE_LD_SWITCH",	FALSE);

			phoebe_config_entry_get ("PHOEBE_KURUCZ_SWITCH", &toggle);
			if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(kurucz_checkbutton))){
				phoebe_config_entry_set ("PHOEBE_KURUCZ_SWITCH",	TRUE);
				phoebe_config_entry_set ("PHOEBE_KURUCZ_DIR",		gtk_file_chooser_get_filename ((GtkFileChooser*)kurucz_filechooserbutton));
			}
			else if (!toggle)
				phoebe_config_entry_set ("PHOEBE_KURUCZ_SWITCH",	FALSE);
		}
        break;

		case GTK_RESPONSE_YES:{
			phoebe_config_entry_set ("PHOEBE_BASE_DIR", 	gtk_file_chooser_get_filename ((GtkFileChooser*)basedir_filechooserbutton));
			phoebe_config_entry_set ("PHOEBE_SOURCE_DIR",	gtk_file_chooser_get_filename ((GtkFileChooser*)srcdir_filechooserbutton));
			phoebe_config_entry_set ("PHOEBE_DEFAULTS_DIR", gtk_file_chooser_get_filename ((GtkFileChooser*)defaultsdir_filechooserbutton));
			phoebe_config_entry_set ("PHOEBE_TEMP_DIR", 	gtk_file_chooser_get_filename ((GtkFileChooser*)workingdir_filechooserbutton));
			phoebe_config_entry_set ("PHOEBE_DATA_DIR", 	gtk_file_chooser_get_filename ((GtkFileChooser*)datadir_filechooserbutton));
			phoebe_config_entry_set ("PHOEBE_PTF_DIR", 		gtk_file_chooser_get_filename ((GtkFileChooser*)ptfdir_filechooserbutton));

			phoebe_config_entry_get ("PHOEBE_LD_SWITCH", &toggle);
			if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(vh_checkbutton))){
				phoebe_config_entry_set ("PHOEBE_LD_SWITCH",	TRUE);
				phoebe_config_entry_set ("PHOEBE_LD_DIR",		gtk_file_chooser_get_filename ((GtkFileChooser*)vh_lddir_filechooserbutton));
				phoebe_load_ld_tables();
			}
			else if (toggle)
				phoebe_config_entry_set ("PHOEBE_LD_SWITCH",	FALSE);

			phoebe_config_entry_get ("PHOEBE_KURUCZ_SWITCH", &toggle);
			if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(kurucz_checkbutton))){
				phoebe_config_entry_set ("PHOEBE_KURUCZ_SWITCH",	TRUE);
				phoebe_config_entry_set ("PHOEBE_KURUCZ_DIR",		gtk_file_chooser_get_filename ((GtkFileChooser*)kurucz_filechooserbutton));
			}
			else if (toggle)
				phoebe_config_entry_set ("PHOEBE_KURUCZ_SWITCH",	FALSE);

			if (!PHOEBE_HOME_DIR || !phoebe_filename_is_directory (PHOEBE_HOME_DIR)) {
				char homedir[255], confdir[255];

				sprintf (homedir, "%s/.phoebe-%s", USER_HOME_DIR, PACKAGE_VERSION);
				sprintf (confdir, "%s/phoebe.config", homedir);

				PHOEBE_HOME_DIR = strdup (homedir);
				PHOEBE_CONFIG   = strdup (confdir);

#ifdef __MINGW32__
				mkdir (PHOEBE_HOME_DIR);
#else
				mkdir (PHOEBE_HOME_DIR, 0755);
#endif
			}

			phoebe_config_save (PHOEBE_CONFIG);
		}
		break;

		case GTK_RESPONSE_CANCEL:
		break;
	}

	gtk_widget_destroy (phoebe_settings_dialog);

	return status;
}

int gui_warning(char* title, char* message)
{
	GtkWidget *dialog;
	int answer = 0;

	dialog = gtk_message_dialog_new ( GTK_WINDOW(gui_widget_lookup("phoebe_window")->gtk),
									  GTK_DIALOG_DESTROY_WITH_PARENT,
									  GTK_MESSAGE_WARNING,
									  GTK_BUTTONS_NONE,
									  message);

	gtk_dialog_add_buttons(GTK_DIALOG (dialog), GTK_STOCK_NO, GTK_RESPONSE_REJECT,
										  		GTK_STOCK_YES, GTK_RESPONSE_ACCEPT,
										  		NULL);

	gtk_window_set_title(GTK_WINDOW (dialog), title);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		answer = 1;

	gtk_widget_destroy (dialog);

	return answer;
}

int gui_question(char* title, char* message)
{
	GtkWidget *dialog;
	int answer = 0;

	dialog = gtk_message_dialog_new ( GTK_WINDOW(gui_widget_lookup("phoebe_window")->gtk),
									  GTK_DIALOG_DESTROY_WITH_PARENT,
									  GTK_MESSAGE_QUESTION,
									  GTK_BUTTONS_NONE,
									  message);

	gtk_dialog_add_buttons(GTK_DIALOG (dialog), GTK_STOCK_NO, GTK_RESPONSE_REJECT,
										  		GTK_STOCK_YES, GTK_RESPONSE_ACCEPT,
										  		NULL);

	gtk_window_set_title(GTK_WINDOW (dialog), title);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		answer = 1;

	gtk_widget_destroy (dialog);

	return answer;
}

int gui_notice(char* title, char* message)
{
	GtkWidget *dialog;
	int answer = 0;

	dialog = gtk_message_dialog_new ( GTK_WINDOW(gui_widget_lookup("phoebe_window")->gtk),
									  GTK_DIALOG_DESTROY_WITH_PARENT,
									  GTK_MESSAGE_INFO,
									  GTK_BUTTONS_NONE,
									  message);

	gtk_dialog_add_buttons(GTK_DIALOG (dialog), GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
										  		NULL);

	gtk_window_set_title(GTK_WINDOW (dialog), title);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		answer = 1;

	gtk_widget_destroy (dialog);

	return answer;
}

int gui_error(char* title, char* message)
{
	GtkWidget *dialog;
	int answer = 0;

	dialog = gtk_message_dialog_new ( GTK_WINDOW(gui_widget_lookup("phoebe_window")->gtk),
									  GTK_DIALOG_DESTROY_WITH_PARENT,
									  GTK_MESSAGE_ERROR,
									  GTK_BUTTONS_NONE,
									  message);

	gtk_dialog_add_buttons(GTK_DIALOG (dialog), GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
										  		NULL);

	gtk_window_set_title(GTK_WINDOW (dialog), title);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		answer = 1;

	gtk_widget_destroy (dialog);

	return answer;
}
