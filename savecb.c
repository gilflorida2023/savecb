/*
 * savecb.c: A C program to save clipboard content to a file.
 * It uses the GTK 3 toolkit for clipboard access and file dialogs.
 *
 * To compile and run:
 * 1. Ensure you have the GTK 3 development headers installed.
 * On Debian-based systems like LMDE, run:
 * sudo apt-get install libgtk-3-dev
 * 2. Run the `make` command in the same directory as the source file and Makefile.
 * 3. Run the executable: ./savecb
 */

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <string.h>

/*
 * Function to save text from the clipboard to a file.
 * It uses a GtkFileChooserNative for a native save dialog.
 */
static void save_text(GtkClipboard *clipboard, const gchar *text) {
    // Create a native file chooser dialog for saving.
    GtkFileChooserNative *dialog = gtk_file_chooser_native_new(
        "Save Text File",
        NULL, // No parent window
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Save",
        "_Cancel"
    );

    // Set a default filename for convenience.
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "clipboard_text.txt");

    // Add a filter to restrict files to text.
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Text Files (*.txt)");
    gtk_file_filter_add_pattern(filter, "*.txt");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    // Run the dialog and check the user's response.
    if (gtk_native_dialog_run(GTK_NATIVE_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (filename) {
            GError *error = NULL;
            // Write the text content to the selected file.
            if (g_file_set_contents(filename, text, -1, &error)) {
                g_print("Text successfully saved to: %s\n", filename);
            } else {
                g_printerr("Error saving text file: %s\n", error->message);
                g_error_free(error);
            }
            g_free(filename);
        }
    } else {
        g_print("Text save canceled.\n");
    }

    // Clean up the dialog and quit the main loop.
    g_object_unref(dialog);
    gtk_main_quit();
}

/*
 * Function to save an image from the clipboard to a file.
 * It uses a GtkFileChooserNative and GdkPixbuf for image handling.
 */
static void save_image(GtkClipboard *clipboard, GdkPixbuf *pixbuf) {
    // Create a native file chooser dialog for saving.
    GtkFileChooserNative *dialog = gtk_file_chooser_native_new(
        "Save Image File",
        NULL, // No parent window
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Save",
        "_Cancel"
    );

    // Add filters for common image formats.
    GtkFileFilter *png_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(png_filter, "PNG Image (*.png)");
    gtk_file_filter_add_pattern(png_filter, "*.png");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), png_filter);

    GtkFileFilter *jpeg_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(jpeg_filter, "JPEG Image (*.jpg)");
    gtk_file_filter_add_pattern(jpeg_filter, "*.jpg");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), jpeg_filter);

    // Set a default filename based on a guess.
    gchar *filename = g_strdup("clipboard_image.png");
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), filename);
    g_free(filename);

    if (gtk_native_dialog_run(GTK_NATIVE_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (filename) {
            GError *error = NULL;
            // Determine the format based on the file extension.
            const gchar *format = "png"; // Default format
            if (g_str_has_suffix(filename, ".jpg") || g_str_has_suffix(filename, ".jpeg")) {
                format = "jpeg";
            }

            if (gdk_pixbuf_save(pixbuf, filename, format, &error, NULL)) {
                g_print("Image successfully saved to: %s\n", filename);
            } else {
                g_printerr("Error saving image: %s\n", error->message);
                g_error_free(error);
            }
            g_free(filename);
        }
    } else {
        g_print("Image save canceled.\n");
    }

    g_object_unref(dialog);
    gtk_main_quit();
}

// Callback for when clipboard content is received.
static void on_clipboard_received_content(GtkClipboard *clipboard, GtkSelectionData *selection_data, gpointer user_data) {
    if (gtk_selection_data_get_length(selection_data) == 0) {
        g_print("Clipboard is empty or contains an unsupported format.\n");
        gtk_main_quit();
        return;
    }

    // Check if the content is an image.
    GdkPixbuf *pixbuf = gtk_selection_data_get_pixbuf(selection_data);
    if (pixbuf) {
        g_print("Image data detected. Opening save dialog...\n");
        save_image(clipboard, pixbuf);
        g_object_unref(pixbuf);
    } else {
        // If not an image, check for text.
        const gchar *text = gtk_selection_data_get_text(selection_data);
        if (text) {
            g_print("Text data detected. Opening save dialog...\n");
            save_text(clipboard, text);
        } else {
            g_print("Clipboard is empty or contains an unsupported format.\n");
            gtk_main_quit();
        }
    }
}

// New callback to check for available targets and then request the content.
static void on_clipboard_received_targets(GtkClipboard *clipboard, GdkAtom *targets, gint n_targets, gpointer user_data) {
    GdkAtom target_atom = NULL;

    // First, check for an image target
    for (gint i = 0; i < n_targets; i++) {
        const gchar *target_name = gdk_atom_name(targets[i]);
        if (g_str_has_prefix(target_name, "image/")) {
            target_atom = targets[i];
            break;
        }
    }

    // If no image target was found, check for a text target
    if (target_atom == NULL) {
        for (gint i = 0; i < n_targets; i++) {
            const gchar *target_name = gdk_atom_name(targets[i]);
            if (g_strcmp0(target_name, "text/plain") == 0 || g_strcmp0(target_name, "UTF8_STRING") == 0) {
                target_atom = targets[i];
                break;
            }
        }
    }

    if (target_atom != NULL) {
        // Request the content of the target that was found.
        gtk_clipboard_request_contents(clipboard, target_atom, on_clipboard_received_content, NULL);
    } else {
        // Print all detected formats
        g_print("Clipboard is empty or contains an unsupported format.\n");
        g_print("Found targets: ");
        for (gint i = 0; i < n_targets; i++) {
            const gchar *target_name = gdk_atom_name(targets[i]);
            if (i > 0) {
                g_print(", ");
            }
            g_print("%s", target_name);
        }
        g_print("\n");
        gtk_main_quit();
    }
}


int main(int argc, char *argv[]) {
    // Initialize GTK.
    gtk_init(&argc, &argv);

    // Get the default clipboard.
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

    // First, request a list of available targets. This will trigger the new callback.
    gtk_clipboard_request_targets(clipboard, on_clipboard_received_targets, NULL);

    // Start the GTK main loop, which handles all events, including the dialog.
    gtk_main();

    return 0;
}

