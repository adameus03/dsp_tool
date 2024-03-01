#include "controller.h"
#include <gtk/gtk.h>

struct ApplicationControls {
    GtkWidget* window;

    GtkWidget* comboBoxText_op;
    GtkWidget* button_perform;

    GtkWidget* comboBoxText_Astype;
    GtkWidget* label_Ap1name;
    GtkWidget* label_Ap2name;
    GtkWidget* label_Ap3name;
    GtkWidget* label_Ap4name;
    GtkWidget* label_Ap5name;
    GtkWidget* entry_Ap1val;
    GtkWidget* entry_Ap2val;
    GtkWidget* entry_Ap3val;
    GtkWidget* entry_Ap4val;
    GtkWidget* entry_Ap5val;

    GtkWidget* comboBoxText_Bstype;
    GtkWidget* label_Bp1name;
    GtkWidget* label_Bp2name;
    GtkWidget* label_Bp3name;
    GtkWidget* label_Bp4name;
    GtkWidget* label_Bp5name;
    GtkWidget* entry_Bp1val;
    GtkWidget* entry_Bp2val;
    GtkWidget* entry_Bp3val;
    GtkWidget* entry_Bp4val;
    GtkWidget* entry_Bp5val;

    GtkWidget* fileChooserButton_ASave;
    GtkWidget* button_Asave_bin;
    GtkWidget* button_Asave_txt;
    GtkWidget* fileChooserButton_ALoad;
    GtkWidget* button_Aload;

    GtkWidget* fileChooserButton_BLoad;
    GtkWidget* button_Bload;

    GtkWidget* entry_Asf;
    GtkWidget* entry_Bsf;
} widgets;

struct ApplicationBuilders {
    GtkBuilder* viewBuilder;
} builders;

// #define EXTRACT_WIDGET(applicationControlsVarName, applicationBuildersVarName, widgetName, builderName) applicationControlsVarName.widgetName = GTK_WIDGET(gtk_builder_get_object(applicationBuildersVarName.builderName, #widgetName))

#define NUM_PARAMS 10
#define NUM_SIGNALS 12

static const char* signal_def_param_names[10] = {
    "Amplitude (A)",
    "Start time (t_1)",
    "Duration (d)",
    "Period (T)",
    "Duty cycle (k_w)",

    "Step time (t_s)",
    "First sample no (n_1)",
    "Spike sample no (n_s)", //"l",
    "f",
    "p"
};

//static const unsigned char custom_signal_idx = NUM_SIGNALS;
static const char* param_affinity[NUM_SIGNALS] = {
    "012",
    "012",
    "0312",
    "0312",
    "0312",
    "0312",
    "03124",
    "03124",
    "0125",
    "0768",
    "01289"
    ""
};

void construct_param_names(uint8_t signal_idx) {
    g_error("Not implemented");
}

void set_param_names(uint8_t signal_idx) {
    if (signal_idx >= NUM_SIGNALS) {
        g_error("Invalid signal index");
    }
    else {
        gtk_label_set_text (GTK_LABEL(widgets.label_Ap1name), "Amplitude");
    }
}



int controller_run(int* psArgc, char*** pppcArgv) {
    gtk_init(psArgc, pppcArgv);
    builders.viewBuilder = gtk_builder_new_from_file("view.xml");
    widgets.window = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "window"));
    
    g_signal_connect (widgets.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_builder_connect_signals(builders.viewBuilder, NULL);

    widgets.comboBoxText_op = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "comboBoxText_op"));
    widgets.button_perform = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "button_perform"));
    widgets.comboBoxText_Astype = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "comboBoxText_Astype"));
    widgets.label_Ap1name = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Ap1name"));
    widgets.label_Ap2name = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Ap2name"));
    widgets.label_Ap3name = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Ap3name"));
    widgets.label_Ap4name = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Ap4name"));
    widgets.label_Ap5name = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Ap5name"));
    widgets.entry_Ap1val = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Ap1val"));
    widgets.entry_Ap2val = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Ap2val"));
    widgets.entry_Ap3val = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Ap3val"));
    widgets.entry_Ap4val = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Ap4val"));
    widgets.entry_Ap5val = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Ap5val"));
    widgets.comboBoxText_Bstype = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "comboBoxText_Bstype"));
    widgets.label_Bp1name = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Bp1name"));
    widgets.label_Bp2name = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Bp2name"));
    widgets.label_Bp3name = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Bp3name"));
    widgets.label_Bp4name = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Bp4name"));
    widgets.label_Bp5name = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Bp5name"));
    widgets.entry_Bp1val = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Bp1val"));
    widgets.entry_Bp2val = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Bp2val"));
    widgets.entry_Bp3val = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Bp3val"));
    widgets.entry_Bp4val = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Bp4val"));
    widgets.entry_Bp5val = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Bp5val"));
    widgets.fileChooserButton_ASave = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "fileChooserButton_ASave"));
    widgets.button_Asave_bin = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "button_Asave_bin"));
    widgets.button_Asave_txt = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "button_Asave_txt"));
    widgets.fileChooserButton_ALoad = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "fileChooserButton_ALoad"));
    widgets.button_Aload = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "button_Aload"));
    widgets.fileChooserButton_BLoad = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "fileChooserButton_BLoad"));
    widgets.button_Bload = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "button_Bload"));
    widgets.entry_Asf = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Asf"));
    widgets.entry_Bsf = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Bsf"));
    
    gtk_widget_show(widgets.window);
    gtk_main();
    
    return EXIT_SUCCESS;
}

void on_comboBoxText_op_changed(GtkComboBoxText* c) {
    
}

void on_button_perform_clicked(GtkButton* b) {
    /* test */
    printf("ON BUttoN PERFORM CLICK\n");
    gtk_label_set_text (GTK_LABEL(widgets.label_Ap1name), "Amplitude");
}

void on_comboBoxText_Astype_changed(GtkComboBoxText* c) {
    
}

void on_comboBoxText_Bstype_changed(GtkComboBoxText* c) {
    
}

void on_entry_Ap1val_insert_at_cursor(GtkEntry* e) {

}

void on_entry_Ap2val_insert_at_cursor(GtkEntry* e) {
    
}

void on_entry_Ap3val_insert_at_cursor(GtkEntry* e) {
    
}

void on_entry_Ap4val_insert_at_cursor(GtkEntry* e) {
    
}

void on_entry_Ap5val_insert_at_cursor(GtkEntry* e) {
    
}

void on_entry_Bp1val_insert_at_cursor(GtkEntry* e) {

}

void on_entry_Bp2val_insert_at_cursor(GtkEntry* e) {
    
}

void on_entry_Bp3val_insert_at_cursor(GtkEntry* e) {
    
}

void on_entry_Bp4val_insert_at_cursor(GtkEntry* e) {
    
}

void on_entry_Bp5val_insert_at_cursor(GtkEntry* e) {
    
}

void on_button_Asave_bin_clicked(GtkButton* b) {
    
}

void on_button_Asave_txt_clicked(GtkButton* b) {

}

void on_button_Aload_clicked(GtkButton* b) {
    
}

void on_button_Bload_clicked(GtkButton* b) {
    
}

void on_entry_Asf_insert_at_cursor(GtkEntry* e) {
    
}

void on_entry_Bsf_insert_at_cursor(GtkEntry* e) {

}


