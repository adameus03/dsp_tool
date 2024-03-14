#include "controller.h"
#include <gtk/gtk.h>
#include <locale.h>
#include "model/generator.h"
#include "model/combiner.h"
#include "model/gnuplot.h"

#define NUM_PARAMS 10
#define NUM_SIGNALS 12
#define MAX_PARAMS_PER_SIGNAL 5

#define MIN_NUM_HISTOGRAM_INTERVALS 1
#define MAX_NUM_HISTOGRAM_INTERVALS 20
#define DEFAULT_NUM_HISTOGRAM_INTERVALS 10

struct ApplicationControls {
    GtkWidget* window;

    GtkWidget* comboBoxText_op;
    GtkWidget* button_perform;
    GtkWidget* button_swap;

    GtkWidget* comboBoxText_Astype;
    GtkWidget* labels_Apname[MAX_PARAMS_PER_SIGNAL];
    GtkWidget* entries_Apval[MAX_PARAMS_PER_SIGNAL];

    GtkWidget* comboBoxText_Bstype;
    GtkWidget* labels_Bpname[MAX_PARAMS_PER_SIGNAL];

    GtkWidget* entries_Bpval[MAX_PARAMS_PER_SIGNAL];

    GtkWidget* fileChooserButton_ASave;
    GtkWidget* button_Asave_bin;
    GtkWidget* button_Asave_txt;
    GtkWidget* fileChooserButton_ALoad;
    GtkWidget* button_Aload;

    GtkWidget* fileChooserButton_BLoad;
    GtkWidget* button_Bload;

    GtkWidget* entry_Asf;
    GtkWidget* entry_Bsf;

    GtkWidget* imageA1;
    GtkWidget* imageA2;
    GtkWidget* imageB1;
    GtkWidget* imageB2;

    GtkWidget* scaleA;
    GtkWidget* scaleB;
} widgets;

struct ApplicationControlHelpers {
    GtkAdjustment* adjustment1;   ///////////////////////////
                                 // For the scale widgets //
    GtkAdjustment* adjustment2; ///////////////////////////

    uint8_t op_idx; // for comboBoxText_op
} widget_helpers;

struct ApplicationBuilders {
    GtkBuilder* viewBuilder;
} builders;

struct Signals {
    real_signal_t signalA;
    real_signal_t signalB;
} signals;

// #define EXTRACT_WIDGET(applicationControlsVarName, applicationBuildersVarName, widgetName, builderName) applicationControlsVarName.widgetName = GTK_WIDGET(gtk_builder_get_object(applicationBuildersVarName.builderName, #widgetName))
// static const unsigned char custom_signal_idx = NUM_SIGNALS;

static char* signal_def_param_names[NUM_PARAMS] = {
    "Amplitude (A)",
    "Start time (t_1)",
    "Duration (d)",
    "Period (T)",
    "Duty cycle (k_w)",

    "Step time (t_s)",
    "First sample no (n_1)",
    "Spike sample no (n_s)", //"l",
    "Sample freq for discrete signal (f)",
    "Spike probability (p)"
};

static uint8_t param_affinity[NUM_SIGNALS][MAX_PARAMS_PER_SIGNAL + 1] = {
    {0, 1, 2, 0xff},
    {0, 1, 2, 0xff},
    {0, 3, 1, 2, 0xff},
    {0, 3, 1, 2, 0xff},
    {0, 3, 1, 2, 0xff},
    {0, 3, 1, 2, 4, 0xff},
    {0, 3, 1, 2, 4, 0xff},
    {0, 3, 1, 2, 4, 0xff},
    {0, 1, 2, 5, 0xff},
    {0, 7, 6, 8, 0xff},
    {0, 1, 2, 8, 9, 0xff},
    {0xff}
};

/**
 * @note The `ppcParamNames` buffer should be allocated by the user and contain at least `MAX_PARAMS_PER_SIGNAL` pointers to char
 * @returns Number of signal parameters
*/
uint8_t construct_param_names(uint8_t signal_idx, char** ppcParamNames) {
    uint8_t* affinity = param_affinity[signal_idx];
    uint8_t k = 0;
    while (affinity[k] != 0xff) {
        ppcParamNames[k] = signal_def_param_names[affinity[k]];
        k++;
    }
    return k;
}

typedef enum {
    SIGNAL_A,
    SIGNAL_B
} signal_selector_t;

void set_param_names(uint8_t signal_idx, signal_selector_t selector) {
    char* paramNames[MAX_PARAMS_PER_SIGNAL];
    uint8_t num_params = construct_param_names(signal_idx, paramNames);

    if (signal_idx >= NUM_SIGNALS) {
        g_error("Invalid signal index");
    }
    else {
        if (selector == SIGNAL_A) {
            for (uint8_t i = 0; i < num_params; i++) {
                gtk_label_set_text (GTK_LABEL(widgets.labels_Apname[i]), paramNames[i]);
                gtk_widget_set_visible(widgets.labels_Apname[i], TRUE);
                gtk_widget_set_visible(widgets.entries_Apval[i], TRUE);
            }
            for (uint8_t i = num_params; i < MAX_PARAMS_PER_SIGNAL; i++) {
                gtk_widget_set_visible(widgets.labels_Apname[i], FALSE);
                gtk_widget_set_visible(widgets.entries_Apval[i], FALSE);
            }

        } else { // SIGNAL_B
            for (uint8_t i = 0; i < num_params; i++) {
                gtk_label_set_text (GTK_LABEL(widgets.labels_Bpname[i]), paramNames[i]);
                gtk_widget_set_visible(widgets.labels_Bpname[i], TRUE);
                gtk_widget_set_visible(widgets.entries_Bpval[i], TRUE);
            }
            for (uint8_t i = num_params; i < MAX_PARAMS_PER_SIGNAL; i++) {
                gtk_widget_set_visible(widgets.labels_Bpname[i], FALSE);
                gtk_widget_set_visible(widgets.entries_Bpval[i], FALSE);
            }
        }
    }
}

uint8_t get_signal_idx_a() {
    return gtk_combo_box_get_active(GTK_COMBO_BOX(widgets.comboBoxText_Astype));
}

uint8_t get_signal_idx_b() {
    return gtk_combo_box_get_active(GTK_COMBO_BOX(widgets.comboBoxText_Bstype));
}

double get_sampling_frequency_a() {
    const gchar* asf_text = gtk_entry_get_text(GTK_ENTRY(widgets.entry_Asf));
    return atof(asf_text);
}

double get_sampling_frequency_b() {
    const gchar* asf_text = gtk_entry_get_text(GTK_ENTRY(widgets.entry_Bsf));
    return atof(asf_text);
}

double get_param1val_a() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Apval[0]))); }
double get_param2val_a() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Apval[1]))); }
double get_param3val_a() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Apval[2]))); }
double get_param4val_a() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Apval[3]))); }
double get_param5val_a() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Apval[4]))); }

double get_param1val_b() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Bpval[0]))); }
double get_param2val_b() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Bpval[1]))); }
double get_param3val_b() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Bpval[2]))); }
double get_param4val_b() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Bpval[3]))); }
double get_param5val_b() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Bpval[4]))); }

uint64_t get_adjustment_val_a() { return (uint64_t)gtk_adjustment_get_value(GTK_ADJUSTMENT(widget_helpers.adjustment1)); }
uint64_t get_adjustment_val_b() { return (uint64_t)gtk_adjustment_get_value(GTK_ADJUSTMENT(widget_helpers.adjustment2)); }


void load_signal_A() {
    uint8_t signal_idx = get_signal_idx_a();

    if ((signals.signalA.pValues != NULL) && (signal_idx != (NUM_SIGNALS - 1))) {
        real_signal_free_values(&signals.signalA);
    }
    
    
    generator_info_t info = { .sampling_frequency = get_sampling_frequency_a() };
    switch (signal_idx) {
        case 0:
            g_error("Not implemented");
            break;
        case 1:
            g_error("Not implemented");
            break;
        case 2:
            signals.signalA = generate_sine(info, get_param1val_a(), get_param2val_a(), get_param3val_a(), get_param4val_a());
            break;
        case 3:
            g_error("Not implemented");
            break;
        case 4:
            g_error("Not implemented");
            break;
        case 5:
            g_error("Not implemented");
            break;
        case 6:
            g_error("Not implemented");
            break;
        case 7:
            g_error("Not implemented");
            break;
        case 8:
            signals.signalA = generate_heaviside(info, get_param1val_a(), get_param2val_a(), get_param3val_a(), get_param4val_a());
            break;
        case 9:
            g_error("Not implemented");
            break;
        case 10:
            g_error("Not implemented");
            break;
        case 11:
            // Keep the custom signal
            signals.signalA.info.sampling_frequency = info.sampling_frequency;
            break;
    }
}

void load_signal_B() {
    uint8_t signal_idx = get_signal_idx_b();

    if ((signals.signalB.pValues != NULL) && (signal_idx != (NUM_SIGNALS - 1))) {
        real_signal_free_values(&signals.signalB);
    }
    
    generator_info_t info = { .sampling_frequency = get_sampling_frequency_b() };
    switch (signal_idx) {
        case 0:
            g_error("Not implemented");
            break;
        case 1:
            g_error("Not implemented");
            break;
        case 2:
            signals.signalB = generate_sine(info, get_param1val_b(), get_param2val_b(), get_param3val_b(), get_param4val_b());
            break;
        case 3:
            g_error("Not implemented");
            break;
        case 4:
            g_error("Not implemented");
            break;
        case 5:
            g_error("Not implemented");
            break;
        case 6:
            g_error("Not implemented");
            break;
        case 7:
            g_error("Not implemented");
            break;
        case 8:
            signals.signalB = generate_heaviside(info, get_param1val_b(), get_param2val_b(), get_param3val_b(), get_param4val_b());
            break;
        case 9:
            g_error("Not implemented");
            break;
        case 10:
            g_error("Not implemented");
            break;
        case 11:
            // Keep the custom signal
            signals.signalB.info.sampling_frequency = info.sampling_frequency;
            break;
    }
}

void draw_plot_A() {
    gnuplot_prepare_real_signal_plot(&signals.signalA, GNUPLOT_SCRIPT_PATH_PLOT_A);
    gtk_image_set_from_file(GTK_IMAGE(widgets.imageA1), GNUPLOT_OUTFILE_PATH);
}

void draw_plot_B() {
    gnuplot_prepare_real_signal_plot(&signals.signalB, GNUPLOT_SCRIPT_PATH_PLOT_B);
    gtk_image_set_from_file(GTK_IMAGE(widgets.imageB1), GNUPLOT_OUTFILE_PATH);
}

void draw_histogram_A() {
    gnuplot_prepare_real_signal_histogram(&signals.signalA, get_adjustment_val_a(), "Signal A histogram", GNUPLOT_SCRIPT_PATH_HISTOGRAM);
    gtk_image_set_from_file(GTK_IMAGE(widgets.imageA2), GNUPLOT_OUTFILE_PATH);
}

void draw_histogram_B() {
    gnuplot_prepare_real_signal_histogram(&signals.signalB, get_adjustment_val_b(), "Signal B histogram", GNUPLOT_SCRIPT_PATH_HISTOGRAM);
    gtk_image_set_from_file(GTK_IMAGE(widgets.imageB2), GNUPLOT_OUTFILE_PATH);
}

void update_A_plots() {
    //fprintf(stdout, "Info: Loading signal A\n");
    load_signal_A();
    //fprintf(stdout, "Info: Drawing plot A\n");
    draw_plot_A();
    //fprintf(stdout, "Info: Drawing histogram A\n");
    draw_histogram_A();
}

void update_B_plots() {
    load_signal_B();
    draw_plot_B();
    draw_histogram_B();
}

void init_scales() {
    gtk_adjustment_set_lower (widget_helpers.adjustment1, (gdouble)MIN_NUM_HISTOGRAM_INTERVALS);
    gtk_adjustment_set_upper (widget_helpers.adjustment1, (gdouble)MAX_NUM_HISTOGRAM_INTERVALS);
    gtk_adjustment_set_lower (widget_helpers.adjustment2, (gdouble)MIN_NUM_HISTOGRAM_INTERVALS);
    gtk_adjustment_set_upper (widget_helpers.adjustment2, (gdouble)MAX_NUM_HISTOGRAM_INTERVALS);
    gtk_adjustment_set_value (widget_helpers.adjustment1, (gdouble)DEFAULT_NUM_HISTOGRAM_INTERVALS);
    gtk_adjustment_set_value (widget_helpers.adjustment2, (gdouble)DEFAULT_NUM_HISTOGRAM_INTERVALS);
}

int controller_run(int* psArgc, char*** pppcArgv) {
    gtk_init(psArgc, pppcArgv);
    setlocale(LC_NUMERIC, "C");

    builders.viewBuilder = gtk_builder_new_from_file("view.xml");
    widgets.window = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "window"));
    
    g_signal_connect (widgets.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    widgets.comboBoxText_op = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "comboBoxText_op"));
    widgets.button_perform = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "button_perform"));
    widgets.button_swap = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "button_swap"));
    widgets.comboBoxText_Astype = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "comboBoxText_Astype"));
    widgets.labels_Apname[0] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Ap1name"));
    widgets.labels_Apname[1] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Ap2name"));
    widgets.labels_Apname[2] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Ap3name"));
    widgets.labels_Apname[3] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Ap4name"));
    widgets.labels_Apname[4] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Ap5name"));
    widgets.entries_Apval[0] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Ap1val"));
    widgets.entries_Apval[1] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Ap2val"));
    widgets.entries_Apval[2] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Ap3val"));
    widgets.entries_Apval[3] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Ap4val"));
    widgets.entries_Apval[4] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Ap5val"));
    widgets.comboBoxText_Bstype = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "comboBoxText_Bstype"));
    widgets.labels_Bpname[0] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Bp1name"));
    widgets.labels_Bpname[1] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Bp2name"));
    widgets.labels_Bpname[2] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Bp3name"));
    widgets.labels_Bpname[3] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Bp4name"));
    widgets.labels_Bpname[4] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_Bp5name"));
    widgets.entries_Bpval[0] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Bp1val"));
    widgets.entries_Bpval[1] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Bp2val"));
    widgets.entries_Bpval[2] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Bp3val"));
    widgets.entries_Bpval[3] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Bp4val"));
    widgets.entries_Bpval[4] = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Bp5val"));
    widgets.fileChooserButton_ASave = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "fileChooserButton_ASave"));
    widgets.button_Asave_bin = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "button_Asave_bin"));
    widgets.button_Asave_txt = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "button_Asave_txt"));
    widgets.fileChooserButton_ALoad = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "fileChooserButton_ALoad"));
    widgets.button_Aload = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "button_Aload"));
    widgets.fileChooserButton_BLoad = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "fileChooserButton_BLoad"));
    widgets.button_Bload = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "button_Bload"));
    widgets.entry_Asf = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Asf"));
    widgets.entry_Bsf = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Bsf"));
    widgets.imageA1 = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "imageA1"));
    widgets.imageA2 = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "imageA2"));
    widgets.imageB1 = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "imageB1"));
    widgets.imageB2 = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "imageB2"));
    widgets.scaleA = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "scaleA"));
    widgets.scaleB = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "scaleB"));

    widget_helpers.adjustment1 = GTK_ADJUSTMENT(gtk_builder_get_object(builders.viewBuilder, "adjustment1"));
    widget_helpers.adjustment2 = GTK_ADJUSTMENT(gtk_builder_get_object(builders.viewBuilder, "adjustment2"));
    widget_helpers.op_idx = 0U;
    
    signals.signalA = (real_signal_t) { .info = { .sampling_frequency = 0 }, .pValues = NULL };
    signals.signalB = (real_signal_t) { .info = { .sampling_frequency = 0 }, .pValues = NULL };

    set_param_names(get_signal_idx_a(), SIGNAL_A);
    set_param_names(get_signal_idx_b(), SIGNAL_B);
    init_scales();

    /*load_signal_A();
    load_signal_B();
    draw_plot_A();
    draw_plot_B();*/
    update_A_plots();
    update_B_plots();

    gtk_builder_connect_signals(builders.viewBuilder, NULL);

    gtk_widget_show(widgets.window);
    gtk_main();

    if (signals.signalA.pValues != NULL) {
        real_signal_free_values(&signals.signalA);
    }
    if (signals.signalB.pValues != NULL) {
        real_signal_free_values(&signals.signalB);
    }
    gnuplot_cleanup();
    
    return EXIT_SUCCESS;
}

void on_comboBoxText_op_changed(GtkComboBox* c, gpointer user_data) {
    widget_helpers.op_idx = gtk_combo_box_get_active(c);
}

void on_button_perform_clicked(GtkButton* b) {
    g_message("Signal combination requested");
    switch (widget_helpers.op_idx) {
        case 0U:
            add_signal (&signals.signalA, &signals.signalB);
            break;
        case 1U:
            substract_signal (&signals.signalA, &signals.signalB);
            break;
        case 2U:
            multiply_signal (&signals.signalA, &signals.signalB);
            break;
        case 3U:
            divide_signal (&signals.signalA, &signals.signalB);
            break;
        default:
            g_error ("Unknown signal combination requested");
            break;
    }
    // Set signal type as custom (the additional type)
    gtk_combo_box_set_active(GTK_COMBO_BOX (widgets.comboBoxText_Astype), (gint)(NUM_SIGNALS - 1)); 
    set_param_names (NUM_SIGNALS - 1, SIGNAL_A);
    update_A_plots();
}

void on_button_swap_clicked(GtkButton* b) {
    g_message("Signal swap requested");
    real_signal_t temp = signals.signalA;
    signals.signalA = signals.signalB;
    signals.signalB = temp;

    uint32_t signalA_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets.comboBoxText_Astype));
    uint32_t signalB_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets.comboBoxText_Bstype));
    gtk_combo_box_set_active(GTK_COMBO_BOX (widgets.comboBoxText_Astype), signalB_idx);
    gtk_combo_box_set_active(GTK_COMBO_BOX (widgets.comboBoxText_Bstype), signalA_idx);
    set_param_names(signalB_idx, SIGNAL_A);
    set_param_names(signalA_idx, SIGNAL_B);

    /*double p1va = get_param1val_a();
    double p2va = get_param2val_a();
    double p3va = get_param3val_a();
    double p4va = get_param4val_a();
    double p5va = get_param5val_a();
    double p1vb = get_param1val_b();
    double p2vb = get_param2val_b();
    double p3vb = get_param3val_b();
    double p4vb = get_param4val_b();
    double p5vb = get_param5val_b();

    char p1vaStr[20]; char p2vaStr[20]; char p3vaStr[20]; char p4vaStr[20]; char p5vaStr[20];
    char p1vbStr[20]; char p2vbStr[20]; char p3vbStr[20]; char p4vbStr[20]; char p5vbStr[20];
    snprintf(p1vaStr, 20, "%f", p1va); snprintf(p2vaStr, 20, "%f", p2va); snprintf(p3vaStr, 20, "%f", p3va); snprintf(p4vaStr, 20, "%f", p4va); snprintf(p5vaStr, 20, "%f", p5va);
    snprintf(p1vbStr, 20, "%f", p1vb); snprintf(p2vbStr, 20, "%f", p2vb); snprintf(p3vbStr, 20, "%f", p3vb); snprintf(p4vbStr, 20, "%f", p4vb); snprintf(p5vbStr, 20, "%f", p5vb);*/

    char* _p1vaStr = (char*)gtk_entry_get_text (GTK_ENTRY(widgets.entries_Apval[0]));
    char* _p2vaStr = (char*)gtk_entry_get_text (GTK_ENTRY(widgets.entries_Apval[1]));
    char* _p3vaStr = (char*)gtk_entry_get_text (GTK_ENTRY(widgets.entries_Apval[2]));
    char* _p4vaStr = (char*)gtk_entry_get_text (GTK_ENTRY(widgets.entries_Apval[3]));
    char* _p5vaStr = (char*)gtk_entry_get_text (GTK_ENTRY(widgets.entries_Apval[4]));
    char* _p1vbStr = (char*)gtk_entry_get_text (GTK_ENTRY(widgets.entries_Bpval[0]));
    char* _p2vbStr = (char*)gtk_entry_get_text (GTK_ENTRY(widgets.entries_Bpval[1]));
    char* _p3vbStr = (char*)gtk_entry_get_text (GTK_ENTRY(widgets.entries_Bpval[2]));
    char* _p4vbStr = (char*)gtk_entry_get_text (GTK_ENTRY(widgets.entries_Bpval[3]));
    char* _p5vbStr = (char*)gtk_entry_get_text (GTK_ENTRY(widgets.entries_Bpval[4]));
    char p1vaStr[20]; char p2vaStr[20]; char p3vaStr[20]; char p4vaStr[20]; char p5vaStr[20];
    char p1vbStr[20]; char p2vbStr[20]; char p3vbStr[20]; char p4vbStr[20]; char p5vbStr[20];
    strcpy(p1vaStr, _p1vaStr);strcpy(p2vaStr, _p2vaStr);strcpy(p3vaStr, _p3vaStr);strcpy(p4vaStr, _p4vaStr);strcpy(p5vaStr, _p5vaStr);
    strcpy(p1vbStr, _p1vbStr);strcpy(p2vbStr, _p2vbStr);strcpy(p3vbStr, _p3vbStr);strcpy(p4vbStr, _p4vbStr);strcpy(p5vbStr, _p5vbStr);

    gtk_entry_set_text (GTK_ENTRY(widgets.entries_Apval[0]), (const gchar*)p1vbStr);
    gtk_entry_set_text (GTK_ENTRY(widgets.entries_Apval[1]), (const gchar*)p2vbStr);
    gtk_entry_set_text (GTK_ENTRY(widgets.entries_Apval[2]), (const gchar*)p3vbStr);
    gtk_entry_set_text (GTK_ENTRY(widgets.entries_Apval[3]), (const gchar*)p4vbStr);
    gtk_entry_set_text (GTK_ENTRY(widgets.entries_Apval[4]), (const gchar*)p5vbStr);
    gtk_entry_set_text (GTK_ENTRY(widgets.entries_Bpval[0]), (const gchar*)p1vaStr);
    gtk_entry_set_text (GTK_ENTRY(widgets.entries_Bpval[1]), (const gchar*)p2vaStr);
    gtk_entry_set_text (GTK_ENTRY(widgets.entries_Bpval[2]), (const gchar*)p3vaStr);
    gtk_entry_set_text (GTK_ENTRY(widgets.entries_Bpval[3]), (const gchar*)p4vaStr);
    gtk_entry_set_text (GTK_ENTRY(widgets.entries_Bpval[4]), (const gchar*)p5vaStr);

    double adjA = (double)gtk_adjustment_get_value(GTK_ADJUSTMENT(widget_helpers.adjustment1));
    double adjB = (double)gtk_adjustment_get_value(GTK_ADJUSTMENT(widget_helpers.adjustment2));
    gtk_adjustment_set_value (GTK_ADJUSTMENT(widget_helpers.adjustment1), (gdouble)adjB);
    gtk_adjustment_set_value (GTK_ADJUSTMENT(widget_helpers.adjustment2), (gdouble)adjA);
    
    update_A_plots();
    update_B_plots();
}

void on_comboBoxText_Astype_changed(GtkComboBox* c, gpointer user_data) {
    uint32_t signal_idx = gtk_combo_box_get_active(c);
    set_param_names(signal_idx, SIGNAL_A);
    update_A_plots();
}

void on_comboBoxText_Bstype_changed(GtkComboBox* c, gpointer user_data) {
    uint32_t signal_idx = gtk_combo_box_get_active(c);
    set_param_names(signal_idx, SIGNAL_B);
    update_B_plots();
}


void on_entry_Ap1val_changed(GtkEntry* e) {
    update_A_plots();
}

void on_entry_Ap2val_changed(GtkEntry* e) {
    update_A_plots();
}

void on_entry_Ap3val_changed(GtkEntry* e) {
    update_A_plots();
}

void on_entry_Ap4val_changed(GtkEntry* e) {
    update_A_plots();
}

void on_entry_Ap5val_changed(GtkEntry* e) {
    update_A_plots();
}

void on_entry_Bp1val_changed(GtkEntry* e) {
    update_B_plots();
}

void on_entry_Bp2val_changed(GtkEntry* e) {
    update_B_plots();
}

void on_entry_Bp3val_changed(GtkEntry* e) {
    update_B_plots();
}

void on_entry_Bp4val_changed(GtkEntry* e) {
    update_B_plots();
}

void on_entry_Bp5val_changed(GtkEntry* e) {
    update_B_plots();
}

void on_button_Asave_bin_clicked(GtkButton* b) {
    g_error("Not implemented");
}

void on_button_Asave_txt_clicked(GtkButton* b) {
    g_error("Not implemented");
}

void on_button_Aload_clicked(GtkButton* b) {
    g_error("Not implemented");
}

void on_button_Bload_clicked(GtkButton* b) {
    g_error("Not implemented");
}

void on_entry_Asf_changed(GtkEntry* e) {
    update_A_plots();
}

void on_entry_Bsf_changed(GtkEntry* e) {
    update_B_plots();
}

void on_scaleA_value_changed(GtkScale* s) {
    update_A_plots();
}

void on_scaleB_value_changed(GtkScale* s) {
    update_B_plots();
}

