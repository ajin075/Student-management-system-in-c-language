#include <gtk/gtk.h>
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>

GtkWidget *entry_name;
GtkWidget *entry_dept;
GtkWidget *entry_marks;
GtkWidget *treeview;

// Function to view students
void on_view_students(GtkButton *button, gpointer user_data) {
    GtkListStore *store;
    GtkTreeIter iter;
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "localhost", "cuser", "Cuser@1234", "college", 0, NULL, 0)) {
        g_print("MySQL Connection Failed: %s\n", mysql_error(conn));
        return;
    }

    if (mysql_query(conn, "SELECT name, dept, marks FROM students")) {
        g_print("Query Failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return;
    }

    res = mysql_store_result(conn);

    while ((row = mysql_fetch_row(res)) != NULL) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, row[0],       // name
                           1, row[1],       // dept
                           2, atoi(row[2]), // marks
                           -1);
    }

    mysql_free_result(res);
    mysql_close(conn);

    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
    g_object_unref(store);
}

// Function to add a student
void on_add_student(GtkButton *button, gpointer user_data) {
    const gchar *name = gtk_entry_get_text(GTK_ENTRY(entry_name));
    const gchar *dept = gtk_entry_get_text(GTK_ENTRY(entry_dept));
    const gchar *marks_str = gtk_entry_get_text(GTK_ENTRY(entry_marks));

    int marks = atoi(marks_str);

    if (name[0] == '\0' || dept[0] == '\0' || marks < 0) {
        g_print("Invalid input!\n");
        return;
    }

    MYSQL *conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "localhost", "cuser", "Cuser@1234", "college", 0, NULL, 0)) {
        g_print("MySQL Connection Failed: %s\n", mysql_error(conn));
        return;
    }

    char query[256];
    snprintf(query, sizeof(query),
             "INSERT INTO students(name, dept, marks) VALUES('%s', '%s', %d)",
             name, dept, marks);

    if (mysql_query(conn, query)) {
        g_print("Insert Failed: %s\n", mysql_error(conn));
    } else {
        g_print("Student Added!\n");
    }

    mysql_close(conn);

    on_view_students(NULL, treeview); // Refresh the list
}

// Main GUI
int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *fixed;
    GtkWidget *button_add, *button_view;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkWidget *image;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Student Management");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 450);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Fixed container for overlay
    fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window), fixed);

    // Background image
    image = gtk_image_new_from_file("college.jpg");
    gtk_fixed_put(GTK_FIXED(fixed), image, 0, 0);

    // Entries
    entry_name = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_name), "Name");
    gtk_widget_set_size_request(entry_name, 150, 30);
    gtk_fixed_put(GTK_FIXED(fixed), entry_name, 20, 20);

    entry_dept = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_dept), "Dept");
    gtk_widget_set_size_request(entry_dept, 120, 30);
    gtk_fixed_put(GTK_FIXED(fixed), entry_dept, 190, 20);

    entry_marks = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_marks), "Marks");
    gtk_widget_set_size_request(entry_marks, 80, 30);
    gtk_fixed_put(GTK_FIXED(fixed), entry_marks, 330, 20);

    // Buttons
    button_add = gtk_button_new_with_label("Add Student");
    gtk_widget_set_size_request(button_add, 120, 30);
    gtk_fixed_put(GTK_FIXED(fixed), button_add, 20, 60);
    g_signal_connect(button_add, "clicked", G_CALLBACK(on_add_student), NULL);

    button_view = gtk_button_new_with_label("View Students");
    gtk_widget_set_size_request(button_view, 120, 30);
    gtk_fixed_put(GTK_FIXED(fixed), button_view, 160, 60);
    g_signal_connect(button_view, "clicked", G_CALLBACK(on_view_students), NULL);

    // TreeView
    treeview = gtk_tree_view_new();
    gtk_widget_set_size_request(treeview, 560, 300);
    gtk_fixed_put(GTK_FIXED(fixed), treeview, 20, 110);

    // Make TreeView transparent
    gtk_widget_set_name(treeview, "treeview");
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "#treeview { background-color: rgba(0,0,0,0); }",
        -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(treeview);
    gtk_style_context_add_provider(context,
                                   GTK_STYLE_PROVIDER(provider),
                                   GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);

    // Columns
    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "foreground", "blue", NULL); // Set text color to white
    column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "foreground", "blue", NULL);
    column = gtk_tree_view_column_new_with_attributes("Dept", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);


    renderer = gtk_cell_renderer_text_new();
     g_object_set(renderer, "foreground", "blue", NULL);
    column = gtk_tree_view_column_new_with_attributes("Marks", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

