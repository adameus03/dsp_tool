#include "controller_transform.h"
#include <gtk/gtk.h>
#include "gui_tweaks.h"

static struct ApplicationControls {
    GtkWidget* window;

    GtkWidget* checkButton_reverseTransform;
    GtkWidget* comboBoxText_transformType;
    GtkWidget* comboBoxText_computationMode;
    GtkWidget* label_param1;
    GtkWidget* entry_param1val;
    GtkWidget* button_close;
    GtkWidget* button_executeTransform;
    GtkWidget* label_runtime;
    GtkWidget* button_abort;
} widgets;

static struct ApplicationControlHelpers {
    controller_transform_callback_fn configureCb;
    controller_transform_abort_callback_fn abortCb;

    uint8_t transformType_idx; // for comboBoxText_transformType // [TODO] ditch? (replace with local variable). Oh, I think it is handy, we can keep it
} widget_helpers;

static struct ApplicationBuilders {
    GtkBuilder* viewTransformBuilder;
} builders;

static bool __is_transformation_issued = false;

static void set_param_names(uint8_t transformTypeIdx) {
    switch (transformTypeIdx) {
        case 0U:
            gtk_widget_set_visible(widgets.label_param1, FALSE);
            gtk_widget_set_visible(widgets.entry_param1val, FALSE);
            break;
        case 1U:
            gtk_label_set_text(GTK_LABEL(widgets.label_param1), "m");
            gtk_widget_set_visible(widgets.label_param1, TRUE);
            gtk_widget_set_visible(widgets.entry_param1val, TRUE);
            break;
        default:
            g_error("Invalid transformTypeIdx");
            break;
    }
}

static uint8_t get_transform_type_idx() {
    return gtk_combo_box_get_active(GTK_COMBO_BOX(widgets.comboBoxText_transformType));
}

static uint8_t get_computation_mode_idx() {
    return gtk_combo_box_get_active(GTK_COMBO_BOX(widgets.comboBoxText_computationMode));
}

static int get_param1val() { return atoi(gtk_entry_get_text(GTK_ENTRY(widgets.entry_param1val))); }

/**
 * @returns true if the checkbox is enabled, false otherwise
 */
static bool get_reverseTransform_enabled() {
    return (bool)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.checkButton_reverseTransform));
}

static void issue_abort_transformation() {
    widget_helpers.abortCb();
    __is_transformation_issued = false;
}

static void on_window_destroyed() {
    g_message("Transform window destroyed.");
    if (__is_transformation_issued) {
        g_message("The transformation was abandoned. Issuing abort...");
        issue_abort_transformation();
    } else {
        transform_common_config_t rejectionTransformConfig = {
            .transformType = -1
        };
        widget_helpers.configureCb(rejectionTransformConfig, false);
    }
}

void controller_transform_run(controller_transform_callback_fn onConfiguredCb, controller_transform_abort_callback_fn onAbortedCb) {
    builders.viewTransformBuilder = gtk_builder_new_from_file("view_transform.xml");
    widgets.window = GTK_WIDGET(gtk_builder_get_object(builders.viewTransformBuilder, "window"));
    
    widgets.checkButton_reverseTransform = GTK_WIDGET(gtk_builder_get_object(builders.viewTransformBuilder, "checkButton_reverseTransform"));
    widgets.comboBoxText_transformType = GTK_WIDGET(gtk_builder_get_object(builders.viewTransformBuilder, "comboBoxText_transformType"));
    widgets.comboBoxText_computationMode = GTK_WIDGET(gtk_builder_get_object(builders.viewTransformBuilder, "comboBoxText_computationMode"));
    widgets.label_param1 = GTK_WIDGET(gtk_builder_get_object(builders.viewTransformBuilder, "label_param1"));
    widgets.entry_param1val = GTK_WIDGET(gtk_builder_get_object(builders.viewTransformBuilder, "entry_param1val"));
    widgets.button_close = GTK_WIDGET(gtk_builder_get_object(builders.viewTransformBuilder, "button_close"));
    widgets.button_executeTransform = GTK_WIDGET(gtk_builder_get_object(builders.viewTransformBuilder, "button_executeTransform"));
    widgets.label_runtime = GTK_WIDGET(gtk_builder_get_object(builders.viewTransformBuilder, "label_runtime"));
    widgets.button_abort = GTK_WIDGET(gtk_builder_get_object(builders.viewTransformBuilder, "button_abort"));

    widget_helpers.configureCb = onConfiguredCb;
    widget_helpers.abortCb = onAbortedCb;
    widget_helpers.transformType_idx = 0U;

    set_param_names(get_transform_type_idx());

    gtk_builder_connect_signals(builders.viewTransformBuilder, NULL);

    gtk_widget_set_visible(widgets.button_abort, FALSE);
    gtk_widget_show(widgets.window);

    g_signal_connect (widgets.window, "destroy", G_CALLBACK(on_window_destroyed), NULL);
}

void controller_transform_set_progress(double fraction) {
    g_message("controller_transform_set_progress called"); // [TODO] Implement
    if (fraction >= 1.0) {
        g_message("It seems like the transform computation has finished.");
        __is_transformation_issued = false;
        gtk_widget_set_visible(widgets.button_abort, FALSE);
        gtk_widget_set_visible(widgets.button_executeTransform, FALSE);
        enable_button(GTK_BUTTON(widgets.button_close));
    }
}


void on_checkButton_reverseTransform_toggled(GtkToggleButton* t) {
    // Do nothing actually
} 

void on_comboBoxText_transformType_changed(GtkComboBox* c, gpointer user_data) {
    widget_helpers.transformType_idx = gtk_combo_box_get_active(c);
    set_param_names(widget_helpers.transformType_idx);
}

void on_button_abort_clicked(GtkButton* b) {
    issue_abort_transformation();
    gtk_widget_set_visible(widgets.button_abort, FALSE);
    enable_button(GTK_BUTTON(widgets.button_executeTransform));
    enable_button(GTK_BUTTON(widgets.button_close));
}

static void __controller_transform_config_set_computation_mode(transform_common_config_t* pConfig, uint8_t computationModeIdx) {
    switch (computationModeIdx) {
        case 0U:
            pConfig->computationMode = TRANSFORM_COMPUTATION_MODE_NAIVE;
            break;
        case 1U:
            pConfig->computationMode = TRANSFORM_COMPUTATION_MOVE_FAST;
            break;
        default:
            g_error("Unknown value detected for computationModeIdx");
            exit(EXIT_FAILURE);
            break;
    }
}

static void __controller_tranform_config_set_direction(transform_common_config_t* pConfig, bool isForward) {
    if (isForward) {
        pConfig->direction = TRANSFORM_DIRECTION_FORWARD;
    } else {
        pConfig->direction = TRANSFORM_DIRECTION_REVERSE;
    }
}

void on_button_executeTransform_clicked(GtkButton* b) {
    if (widget_helpers.configureCb != NULL) {
        transform_common_config_t transformConfig = {};
        __controller_transform_config_set_computation_mode(&transformConfig, get_computation_mode_idx());
        __controller_tranform_config_set_direction(&transformConfig, !get_reverseTransform_enabled());

        switch (widget_helpers.transformType_idx) {
            case 0U:
                transformConfig.transformType = TRANSFORM_TYPE_DFT;
                transformConfig.dftConfig = (dft_config_t) {};
                break;
            case 1U:
                transformConfig.transformType = TRANSFORM_TYPE_WALSH_HADAMARD;
                transformConfig.whConfig = (walsh_hadamard_config_t) {
                    .m = (uint64_t)get_param1val()
                };
                break;
            default:
                g_error("Invalid widget_helpers.transformType_idx detected");
                exit(EXIT_FAILURE);
                break;
        }
        widget_helpers.configureCb(transformConfig, true);
    }
    __is_transformation_issued = true;
    gtk_widget_set_visible(widgets.button_abort, TRUE);
    disable_button(GTK_BUTTON(widgets.button_executeTransform));
    disable_button(GTK_BUTTON(widgets.button_close));
}

void on_button_close_clicked(GtkButton* b) {
    gtk_window_close(GTK_WINDOW(widgets.window));
}