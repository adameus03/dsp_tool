#include "controller.h"
#include <gtk/gtk.h>
#include <locale.h>
#include "gui_tweaks.h"
#include "model/generator.h"
#include "model/combiner.h"
#include "model/aggregator.h"
#include "model/gnuplot.h"
#include "model/signal_fio.h"

#include "model/converters/adc.h"
#include "model/converters/dac.h"

#include "model/measures/similarity.h"

#include "controller_timeshift.h"
#include "controller_fir.h"

#define NUM_PARAMS 10
#define NUM_SIGNALS 12
#define MAX_PARAMS_PER_SIGNAL 5

#define MIN_NUM_HISTOGRAM_INTERVALS 1
#define MAX_NUM_HISTOGRAM_INTERVALS /*20*/100
#define DEFAULT_NUM_HISTOGRAM_INTERVALS 10

static struct ApplicationControls {
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

    GtkWidget* labelAmsv;
    GtkWidget* labelAmsav;
    GtkWidget* labelAmsp;
    GtkWidget* labelAsv;
    GtkWidget* labelArms;
    GtkWidget* labelBmsv;
    GtkWidget* labelBmsav;
    GtkWidget* labelBmsp;
    GtkWidget* labelBsv;
    GtkWidget* labelBrms;

    GtkWidget* checkButton_AsReconstruct;
    GtkWidget* comboBoxText_AsReconstructionMethod;
    GtkWidget* entry_Asqt;
    GtkWidget* checkButton_BsReconstruct;
    GtkWidget* comboBoxText_BsReconstructionMethod;
    GtkWidget* entry_Bsqt;

    GtkWidget* label_AsNeighCoeff;
    GtkWidget* entry_AsNeighCoeffVal;
    GtkWidget* label_BsNeighCoeff;
    GtkWidget* entry_BsNeighCoeffVal;

    GtkWidget* labelAsmse;
    GtkWidget* labelAssnr;
    GtkWidget* labelAspsnr;
    GtkWidget* labelAsmd;
    GtkWidget* labelAqmse;
    GtkWidget* labelAqsnr;
    GtkWidget* labelAqpsnr;
    GtkWidget* labelAqmd;
    GtkWidget* labelBsmse;
    GtkWidget* labelBssnr;
    GtkWidget* labelBspsnr;
    GtkWidget* labelBsmd;
    GtkWidget* labelBqmse;
    GtkWidget* labelBqsnr;
    GtkWidget* labelBqpsnr;
    GtkWidget* labelBqmd;

    GtkWidget* labelAenob;
    GtkWidget* labelBenob;

    GtkWidget* button_cpy;
    GtkWidget* button_collapseTDomains;

    GtkWidget* button_Atimeshift;
    GtkWidget* button_Afir;
    GtkWidget* button_Btimeshift;
    GtkWidget* button_Bfir;

    GtkWidget* labelAargmax;
    GtkWidget* labelAcdist;
    GtkWidget* labelBargmax;
    GtkWidget* labelBcdist;
} widgets;

static struct ApplicationControlHelpers {
    GtkAdjustment* adjustment1;   ///////////////////////////
                                 // For the scale widgets //
    GtkAdjustment* adjustment2; ///////////////////////////

    uint8_t op_idx; // for comboBoxText_op
    char* a_load_filename;
    char* b_load_filename;
} widget_helpers;

static struct ApplicationBuilders {
    GtkBuilder* viewBuilder;
} builders;

static struct Signals {
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
    "Spike sample no (n_s)",
    //"Sample freq for discrete signal (f)",
    "Spike probability (p)",
    "Number of samples (l)"
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
    {0, 7, 6, 9, 0xff},
    {0, 1, 2, 8, 0xff},
    {0xff}
};

/**
 * @note The `ppcParamNames` buffer should be allocated by the user and contain at least `MAX_PARAMS_PER_SIGNAL` pointers to char
 * @returns Number of signal parameters
*/
static uint8_t construct_param_names(uint8_t signal_idx, char** ppcParamNames) {
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

typedef enum {
    SIMILARITY_MEASURE_DESTINY_SAMPLING,
    SIMILIARITY_MEASURE_DESTINY_QUANTIZATION
} similiarity_measure_destiny_selector_t;

static void set_param_names(uint8_t signal_idx, signal_selector_t selector) {
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

static void set_sReconstruct(gboolean b, signal_selector_t sel) {
    if (sel == SIGNAL_A) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.checkButton_AsReconstruct), b);
    } else {//SIGNAL_B
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.checkButton_BsReconstruct), b);
    }
}

static void set_sReconstructionMethod(uint8_t u, signal_selector_t sel) {
    if (sel == SIGNAL_A) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(widgets.comboBoxText_AsReconstructionMethod), (gint)u);
    } else {//SIGNAL_B
        gtk_combo_box_set_active(GTK_COMBO_BOX(widgets.comboBoxText_BsReconstructionMethod), (gint)u);
    }
}

static void set_sNeighCoeffVal(gchar* value, signal_selector_t sel) {
    if (sel == SIGNAL_A) {
        gtk_entry_set_text(GTK_ENTRY(widgets.entry_AsNeighCoeffVal), value);
    } else {//SIGNAL_B
        gtk_entry_set_text(GTK_ENTRY(widgets.entry_BsNeighCoeffVal), value);
    }
}

static gboolean get_sReconstruct(signal_selector_t sel) {
    if (sel == SIGNAL_A) {
        return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.checkButton_AsReconstruct));
    } else {//SIGNAL_B
        return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.checkButton_BsReconstruct));
    }
}

static gint get_sReconstructionMethod(signal_selector_t sel) {
    if (sel == SIGNAL_A) {
        return gtk_combo_box_get_active(GTK_COMBO_BOX(widgets.comboBoxText_AsReconstructionMethod));
    } else {//SIGNAL_B
        return gtk_combo_box_get_active(GTK_COMBO_BOX(widgets.comboBoxText_BsReconstructionMethod));
    }
}

static const gchar* get_sNeighCoeffVal(signal_selector_t sel) {
    if (sel == SIGNAL_A) {
        return gtk_entry_get_text(GTK_ENTRY(widgets.entry_AsNeighCoeffVal)); 
    } else {//SIGNAL_B
        return gtk_entry_get_text(GTK_ENTRY(widgets.entry_BsNeighCoeffVal));
    }
}

static uint8_t get_signal_idx_a() {
    return gtk_combo_box_get_active(GTK_COMBO_BOX(widgets.comboBoxText_Astype));
}

static uint8_t get_signal_idx_b() {
    return gtk_combo_box_get_active(GTK_COMBO_BOX(widgets.comboBoxText_Bstype));
}

static double get_sampling_frequency_a() {
    const gchar* asf_text = gtk_entry_get_text(GTK_ENTRY(widgets.entry_Asf));
    return atof(asf_text);
}

static double get_sampling_frequency_b() {
    const gchar* asf_text = gtk_entry_get_text(GTK_ENTRY(widgets.entry_Bsf));
    return atof(asf_text);
}

static double get_quantization_threshold_a() {
    const gchar* asqt_text = gtk_entry_get_text(GTK_ENTRY(widgets.entry_Asqt));
    return atof(asqt_text);
}

static double get_quantization_threshold_b() {
    const gchar* bsqt_text = gtk_entry_get_text(GTK_ENTRY(widgets.entry_Bsqt));
    return atof(bsqt_text);
}

static uint8_t get_reconstruction_method_idx_a() {
    return gtk_combo_box_get_active(GTK_COMBO_BOX(widgets.comboBoxText_AsReconstructionMethod));   
}

static uint8_t get_reconstruction_method_idx_b() {
    return gtk_combo_box_get_active(GTK_COMBO_BOX(widgets.comboBoxText_BsReconstructionMethod));
}

static uint64_t get_sinc_neigh_coeff_val_a() {
    const gchar* neigh_coeff_text_a = gtk_entry_get_text(GTK_ENTRY(widgets.entry_AsNeighCoeffVal));
    return (uint64_t)atoll(neigh_coeff_text_a);
}

static uint64_t get_sinc_neigh_coeff_val_b() {
    const gchar* neigh_coeff_text_b = gtk_entry_get_text(GTK_ENTRY(widgets.entry_BsNeighCoeffVal));
    return (uint64_t)atoll(neigh_coeff_text_b);
}

/**
 * @returns 1 if reconstruction mode for signal A is enabled (checkbox checked), 0 otherwise 
*/
static uint8_t get_reconstruction_mode_enabled_a() {
    gboolean checkbox_a__val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.checkButton_AsReconstruct));
    if (checkbox_a__val == TRUE) {
        return 1U;
    } else {
        return 0U;
    }
}

/**
 * @returns 1 if reconstruction mode for signal B is enabled (checkbox checked), 0 otherwise 
*/
static uint8_t get_reconstruction_mode_enabled_b() {
    gboolean checkbox_b__val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets.checkButton_BsReconstruct));
    if (checkbox_b__val == TRUE) {
        return 1U;
    } else {
        return 0U;
    }
}





static double get_param1val_a() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Apval[0]))); }
static double get_param2val_a() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Apval[1]))); }
static double get_param3val_a() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Apval[2]))); }
static double get_param4val_a() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Apval[3]))); }
static double get_param5val_a() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Apval[4]))); }

static double get_param1val_b() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Bpval[0]))); }
static double get_param2val_b() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Bpval[1]))); }
static double get_param3val_b() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Bpval[2]))); }
static double get_param4val_b() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Bpval[3]))); }
static double get_param5val_b() { return atof(gtk_entry_get_text(GTK_ENTRY(widgets.entries_Bpval[4]))); }

static uint64_t get_adjustment_val_a() { return (uint64_t)gtk_adjustment_get_value(GTK_ADJUSTMENT(widget_helpers.adjustment1)); }
static uint64_t get_adjustment_val_b() { return (uint64_t)gtk_adjustment_get_value(GTK_ADJUSTMENT(widget_helpers.adjustment2)); }


static void __evaluate_similarity_measures(real_signal_t* pSignalOriginal,
                                    real_signal_t* pSignalImitated,
                                    similiarity_measure_destiny_selector_t measureDestinySelector,
                                    signal_selector_t signalSelector) {
    double mse = signal_mean_squared_error(pSignalImitated, pSignalOriginal);
    double enob;
    double snr = signal_to_noise(pSignalImitated, pSignalOriginal, &enob);
    double psnr = peak_signal_to_noise(pSignalImitated, pSignalOriginal);
    double md = signal_max_difference(pSignalImitated, pSignalOriginal);

    char mseStr[20]; char snrStr[20]; char psnrStr[20]; char mdStr[20];
    snprintf(mseStr, 20, "%f", mse); snprintf(snrStr, 20, "%f", snr); snprintf(psnrStr, 20, "%f", psnr); snprintf(mdStr, 20, "%f", md);
    
    if (signalSelector == SIGNAL_A) {
        if (measureDestinySelector == SIMILARITY_MEASURE_DESTINY_SAMPLING) {
            gtk_label_set_text(GTK_LABEL(widgets.labelAsmse), (const gchar*)mseStr);
            gtk_label_set_text(GTK_LABEL(widgets.labelAssnr), (const gchar*)snrStr);
            gtk_label_set_text(GTK_LABEL(widgets.labelAspsnr), (const gchar*)psnrStr);
            gtk_label_set_text(GTK_LABEL(widgets.labelAsmd), (const gchar*)mdStr);
        } else { // SIMILARITY_MEASURE_DESTINY_QUANTIZATION
            gtk_label_set_text(GTK_LABEL(widgets.labelAqmse), (const gchar*)mseStr);
            gtk_label_set_text(GTK_LABEL(widgets.labelAqsnr), (const gchar*)snrStr);
            gtk_label_set_text(GTK_LABEL(widgets.labelAqpsnr), (const gchar*)psnrStr);
            gtk_label_set_text(GTK_LABEL(widgets.labelAqmd), (const gchar*)mdStr);

            char aenobStr[20]; snprintf(aenobStr, 20, "%f", enob);
            gtk_label_set_text(GTK_LABEL(widgets.labelAenob), (const gchar*)aenobStr);
        }
    } else { // SIGNAL_B
        if (measureDestinySelector == SIMILARITY_MEASURE_DESTINY_SAMPLING) {
            gtk_label_set_text(GTK_LABEL(widgets.labelBsmse), (const gchar*)mseStr);
            gtk_label_set_text(GTK_LABEL(widgets.labelBssnr), (const gchar*)snrStr);
            gtk_label_set_text(GTK_LABEL(widgets.labelBspsnr), (const gchar*)psnrStr);
            gtk_label_set_text(GTK_LABEL(widgets.labelBsmd), (const gchar*)mdStr);
        } else { // SIMILARITY_MEASURE_DESTINY_QUANTIZATION
            gtk_label_set_text(GTK_LABEL(widgets.labelBqmse), (const gchar*)mseStr);
            gtk_label_set_text(GTK_LABEL(widgets.labelBqsnr), (const gchar*)snrStr);
            gtk_label_set_text(GTK_LABEL(widgets.labelBqpsnr), (const gchar*)psnrStr);
            gtk_label_set_text(GTK_LABEL(widgets.labelBqmd), (const gchar*)mdStr);

            char benobStr[20]; snprintf(benobStr, 20, "%f", enob);
            gtk_label_set_text(GTK_LABEL(widgets.labelBenob), (const gchar*)benobStr);
        }
    }
}

static void evaluate_similarity_measures(signal_selector_t originalSignalSelector, 
                                  similiarity_measure_destiny_selector_t measureDestinySelector,
                                  real_signal_t* pSignalImitated) {
    real_signal_t* pSignalOriginal = originalSignalSelector == SIGNAL_A ? &signals.signalA : &signals.signalB;
    __evaluate_similarity_measures(pSignalOriginal, pSignalImitated, measureDestinySelector, originalSignalSelector);
    
}

static void quantization_handler_A() {
    double quant_threshold = get_quantization_threshold_a();
    if (quant_threshold <= 0.0) {
        g_message("Zero quantization threshold for signal A. Skipping quantization.");
        return;
    } else {
        adc_caps_t adcCaps = {
            .quantization_threshold = quant_threshold
        };

        real_signal_t signalACopy = {
            .info = signals.signalA.info
        };
        real_signal_alloc_values(&signalACopy);
        memcpy(signalACopy.pValues, signals.signalA.pValues, sizeof(double) * signals.signalA.info.num_samples);

        adc_quantize_real_signal(&adcCaps, &signals.signalA);

        //evaluate_similarity_measures(SIGNAL_A, SIMILIARITY_MEASURE_DESTINY_QUANTIZATION, )
        __evaluate_similarity_measures(&signalACopy, &signals.signalA, SIMILIARITY_MEASURE_DESTINY_QUANTIZATION, SIGNAL_A);
    }
}

static void quantization_handler_B() {
    double quant_threshold = get_quantization_threshold_b();
    if (quant_threshold <= 0.0) {
        g_message("Zero quantization threshold for signal B. Skipping quantization.");
        return;
    } else {
        adc_caps_t adcCaps = {
            .quantization_threshold = quant_threshold
        };

        real_signal_t signalBCopy = {
            .info = signals.signalB.info
        };
        real_signal_alloc_values(&signalBCopy);
        memcpy(signalBCopy.pValues, signals.signalB.pValues, sizeof(double) * signals.signalB.info.num_samples);
        adc_quantize_real_signal(&adcCaps, &signals.signalB);

        __evaluate_similarity_measures(&signalBCopy, &signals.signalB, SIMILIARITY_MEASURE_DESTINY_QUANTIZATION, SIGNAL_B);
    }
}

static void load_signal_A() {
    uint8_t signal_idx = get_signal_idx_a();

    if ((signals.signalA.pValues != NULL) && (signal_idx != (NUM_SIGNALS - 1))) {
        real_signal_free_values(&signals.signalA);
    }
    
    
    generator_info_t info = { .sampling_frequency = get_sampling_frequency_a() };
    switch (signal_idx) {
        case 0:
            signals.signalA = generate_uniform_noise(info, get_param1val_a(), get_param2val_a(), get_param3val_a());
            break;
        case 1:
            signals.signalA = generate_gaussian_noise(info, get_param1val_a(), get_param2val_a(), get_param3val_a());
            break;
        case 2:
            signals.signalA = generate_sine(info, get_param1val_a(), get_param2val_a(), get_param3val_a(), get_param4val_a());
            break;
        case 3:
            signals.signalA = generate_half_wave_rectified_sine(info, get_param1val_a(), get_param2val_a(), get_param3val_a(), get_param4val_a());
            break;
        case 4:
            signals.signalA = generate_full_wave_rectified_sine(info, get_param1val_a(), get_param2val_a(), get_param3val_a(), get_param4val_a());
            break;
        case 5:
            signals.signalA = generate_rectangular(info, get_param1val_a(), get_param2val_a(), get_param3val_a(), get_param4val_a(), get_param5val_a());
            break;
        case 6:
            signals.signalA = generate_symmetric_rectangular(info, get_param1val_a(), get_param2val_a(), get_param3val_a(), get_param4val_a(), get_param5val_a());
            break;
        case 7:
            signals.signalA = generate_triangle(info, get_param1val_a(), get_param2val_a(), get_param3val_a(), get_param4val_a(), get_param5val_a());
            break;
        case 8:
            signals.signalA = generate_heaviside(info, get_param1val_a(), get_param2val_a(), get_param3val_a(), get_param4val_a());
            break;
        case 9:
            signals.signalA = generate_kronecker_delta(info, get_param1val_a(), get_param2val_a(), get_param3val_a(), get_param4val_a());
            break;
        case 10:
            signals.signalA = generate_impulse_noise(info, get_param1val_a(), get_param2val_a(), get_param3val_a(), get_param4val_a());
            break;
        case 11:
            // Keep the custom signal
            signals.signalA.info.sampling_frequency = info.sampling_frequency;
            break;
    }

    // Disable/enable sampling freqency modification if needed
    // can focus+opacity+editable
    if (signal_idx == (NUM_SIGNALS - 1)) {
        disable_entry (GTK_ENTRY(widgets.entry_Asf));
    } else {
        enable_entry (GTK_ENTRY(widgets.entry_Asf));
    }

    // Handle quantization
    quantization_handler_A();
}

static void load_signal_B() {
    uint8_t signal_idx = get_signal_idx_b();

    if ((signals.signalB.pValues != NULL) && (signal_idx != (NUM_SIGNALS - 1))) {
        real_signal_free_values(&signals.signalB);
    }
    
    generator_info_t info = { .sampling_frequency = get_sampling_frequency_b() };
    switch (signal_idx) {
        case 0:
            signals.signalB = generate_uniform_noise(info, get_param1val_b(), get_param2val_b(), get_param3val_b());
            break;
        case 1:
            signals.signalB = generate_gaussian_noise(info, get_param1val_b(), get_param2val_b(), get_param3val_b());
            break;
        case 2:
            signals.signalB = generate_sine(info, get_param1val_b(), get_param2val_b(), get_param3val_b(), get_param4val_b());
            break;
        case 3:
            signals.signalB = generate_half_wave_rectified_sine(info, get_param1val_b(), get_param2val_b(), get_param3val_b(), get_param4val_b());
            break;
        case 4:
            signals.signalB = generate_full_wave_rectified_sine(info, get_param1val_b(), get_param2val_b(), get_param3val_b(), get_param4val_b());
            break;
        case 5:
            signals.signalB = generate_rectangular(info, get_param1val_b(), get_param2val_b(), get_param3val_b(), get_param4val_b(), get_param5val_b());
            break;
        case 6:
            signals.signalB = generate_symmetric_rectangular(info, get_param1val_b(), get_param2val_b(), get_param3val_b(), get_param4val_b(), get_param5val_b());
            break;
        case 7:
            signals.signalB = generate_triangle(info, get_param1val_b(), get_param2val_b(), get_param3val_b(), get_param4val_b(), get_param5val_b());
            break;
        case 8:
            signals.signalB = generate_heaviside(info, get_param1val_b(), get_param2val_b(), get_param3val_b(), get_param4val_b());
            break;
        case 9:
            signals.signalB = generate_kronecker_delta(info, get_param1val_b(), get_param2val_b(), get_param3val_b(), get_param4val_b());
            break;
        case 10:
            signals.signalB = generate_impulse_noise(info, get_param1val_b(), get_param2val_b(), get_param3val_b(), get_param4val_b());
            break;
        case 11:
            // Keep the custom signal
            signals.signalB.info.sampling_frequency = info.sampling_frequency;
            break;
    }

    // Disable/enable sampling freqency modification if needed
    // can focus+opacity+editable
    if (signal_idx == (NUM_SIGNALS - 1)) {
        disable_entry (GTK_ENTRY(widgets.entry_Bsf));
    } else {
        enable_entry (GTK_ENTRY(widgets.entry_Bsf));
    }

    // Handle quantization
    quantization_handler_B();
}



/**
 * @todo Verify
*/
static void __replace_real_signal(real_signal_t* signalAddr, real_signal_t newSignal) {
    real_signal_free_values (signalAddr);
    signalAddr->info = newSignal.info;
    signalAddr->pValues = newSignal.pValues;
}

static void __reconstruct_signal_caps_helper(signal_selector_t signalSelector, dac_config_t* pDacConfig) {
    pseudo_dac_caps_t caps = {};
    if (signalSelector == SIGNAL_A) {
        caps.output_sampling_freq = get_sampling_frequency_a();
    } else { // SIGNAL_B
        caps.output_sampling_freq = get_sampling_frequency_b();
    }

    real_signal_t reconstructedSignal = {};
    if (signalSelector == SIGNAL_A) {
        reconstructedSignal = dac_reconstruct_real_signal (pDacConfig, &caps, &signals.signalA);
    } else { //SIGNAL_B
        reconstructedSignal = dac_reconstruct_real_signal (pDacConfig, &caps, &signals.signalB);
    }

    evaluate_similarity_measures(signalSelector, SIMILARITY_MEASURE_DESTINY_SAMPLING, &reconstructedSignal);

    __replace_real_signal (
        signalSelector == SIGNAL_A ? &signals.signalA : &signals.signalB,
        reconstructedSignal
    );
}

static void reconstruct_signal(signal_selector_t signalSelector) {
    dac_config_t dacConfig = { };
    uint8_t rtype;
    if (signalSelector == SIGNAL_A) {
        rtype = get_reconstruction_method_idx_a();
        
    } else { // SIGNAL_B
        rtype = get_reconstruction_method_idx_b();
    }

    if (rtype == 0) {
        dacConfig.reconstruction_type = DAC_RECONSTRUCTION_ZERO_ORDER_EXTRAPOLATION;
        dacConfig.pReconstruction_config = NULL;
        __reconstruct_signal_caps_helper(signalSelector, &dacConfig);
    } else if (rtype == 1) {
        dacConfig.reconstruction_type = DAC_RECONSTRUCTION_SINC;
        sinc_reconstruction_config_t sincReconstructionConfig = {
            .symmetric_num_neighbours = signalSelector == SIGNAL_A ? get_sinc_neigh_coeff_val_a() : get_sinc_neigh_coeff_val_b()
        };
        dacConfig.pReconstruction_config = &sincReconstructionConfig;
        __reconstruct_signal_caps_helper(signalSelector, &dacConfig);
    } else {
        g_error("Invalid reconstruction type detected!");
    }
}

static void draw_plot_A() {
    gnuplot_prepare_real_signal_plot(&signals.signalA, GNUPLOT_SCRIPT_PATH_PLOT_A);
    gtk_image_set_from_file(GTK_IMAGE(widgets.imageA1), GNUPLOT_OUTFILE_PATH);
}

static void draw_plot_B() {
    gnuplot_prepare_real_signal_plot(&signals.signalB, GNUPLOT_SCRIPT_PATH_PLOT_B);
    gtk_image_set_from_file(GTK_IMAGE(widgets.imageB1), GNUPLOT_OUTFILE_PATH);
}

static void draw_histogram_A() {
    gnuplot_prepare_real_signal_histogram(&signals.signalA, get_adjustment_val_a(), "Signal A histogram", GNUPLOT_SCRIPT_PATH_HISTOGRAM);
    gtk_image_set_from_file(GTK_IMAGE(widgets.imageA2), GNUPLOT_OUTFILE_PATH);
}

static void draw_histogram_B() {
    gnuplot_prepare_real_signal_histogram(&signals.signalB, get_adjustment_val_b(), "Signal B histogram", GNUPLOT_SCRIPT_PATH_HISTOGRAM);
    gtk_image_set_from_file(GTK_IMAGE(widgets.imageB2), GNUPLOT_OUTFILE_PATH);
}

static void evaluate_A_aggregates() {
    double amsv = mean_signal_value(&signals.signalA);
    double amsav = mean_signal_absolute_value(&signals.signalA);
    double amsp = mean_signal_power(&signals.signalA);
    double asv = signal_variance(&signals.signalA);
    double arms = signal_RMS(&signals.signalA);

    char amsvStr[20]; char amsavStr[20]; char amspStr[20]; char asvStr[20]; char armsStr[20];
    snprintf(amsvStr, 20, "%f", amsv); snprintf(amsavStr, 20, "%f", amsav); snprintf(amspStr, 20, "%f", amsp); snprintf(asvStr, 20, "%f", asv); snprintf(armsStr, 20, "%f", arms);
    
    gtk_label_set_text(GTK_LABEL(widgets.labelAmsv), (const gchar*)amsvStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAmsav), (const gchar*)amsavStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAmsp), (const gchar*)amspStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAsv), (const gchar*)asvStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelArms), (const gchar*)armsStr);
}

static void evaluate_B_aggregates() {
    double bmsv = mean_signal_value(&signals.signalB);
    double bmsav = mean_signal_absolute_value(&signals.signalB);
    double bmsp = mean_signal_power(&signals.signalB);
    double bsv = signal_variance(&signals.signalB);
    double brms = signal_RMS(&signals.signalB);

    char bmsvStr[20]; char bmsavStr[20]; char bmspStr[20]; char bsvStr[20]; char brmsStr[20];
    snprintf(bmsvStr, 20, "%f", bmsv); snprintf(bmsavStr, 20, "%f", bmsav); snprintf(bmspStr, 20, "%f", bmsp); snprintf(bsvStr, 20, "%f", bsv); snprintf(brmsStr, 20, "%f", brms);

    gtk_label_set_text(GTK_LABEL(widgets.labelBmsv), (const gchar*)bmsvStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBmsav), (const gchar*)bmsavStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBmsp), (const gchar*)bmspStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBsv), (const gchar*)bsvStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBrms), (const gchar*)brmsStr);
}

static void update_A_plots_no_sigload() {
    //fprintf(stdout, "Info: Drawing plot A\n");
    draw_plot_A();
    //fprintf(stdout, "Info: Drawing histogram A\n");
    draw_histogram_A();

    evaluate_A_aggregates();
}

static void update_B_plots_no_sigload() {
    draw_plot_B();
    draw_histogram_B();
    evaluate_B_aggregates();
}

static void update_A_plots() {
    //fprintf(stdout, "Info: Loading signal A\n");
    load_signal_A();
    update_A_plots_no_sigload();
}

static void update_B_plots() {
    load_signal_B();
    update_B_plots_no_sigload();   
}

static void init_scales() {
    gtk_adjustment_set_lower (widget_helpers.adjustment1, (gdouble)MIN_NUM_HISTOGRAM_INTERVALS);
    gtk_adjustment_set_upper (widget_helpers.adjustment1, (gdouble)MAX_NUM_HISTOGRAM_INTERVALS);
    gtk_adjustment_set_lower (widget_helpers.adjustment2, (gdouble)MIN_NUM_HISTOGRAM_INTERVALS);
    gtk_adjustment_set_upper (widget_helpers.adjustment2, (gdouble)MAX_NUM_HISTOGRAM_INTERVALS);
    gtk_adjustment_set_value (widget_helpers.adjustment1, (gdouble)DEFAULT_NUM_HISTOGRAM_INTERVALS);
    gtk_adjustment_set_value (widget_helpers.adjustment2, (gdouble)DEFAULT_NUM_HISTOGRAM_INTERVALS);

    disable_scroll(widgets.scaleA);
    disable_scroll(widgets.scaleB);
}

/**
 * Makes the reconstruction GUI hidden by default
*/
static void init_reconstruction_gui() {
    //disable_combo_box(GTK_COMBO_BOX(widgets.comboBoxText_AsReconstructionMethod));
    gtk_widget_set_visible(widgets.comboBoxText_AsReconstructionMethod, FALSE);
    gtk_widget_set_visible(widgets.label_AsNeighCoeff, FALSE);
    gtk_widget_set_visible(widgets.entry_AsNeighCoeffVal, FALSE);
    gtk_widget_set_visible(widgets.comboBoxText_BsReconstructionMethod, FALSE);
    gtk_widget_set_visible(widgets.label_BsNeighCoeff, FALSE);
    gtk_widget_set_visible(widgets.entry_BsNeighCoeffVal, FALSE);
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
    widget_helpers.a_load_filename = NULL;
    widget_helpers.b_load_filename = NULL;
    
    signals.signalA = (real_signal_t) { .info = { .sampling_frequency = 0 }, .pValues = NULL };
    signals.signalB = (real_signal_t) { .info = { .sampling_frequency = 0 }, .pValues = NULL };

    widgets.labelAmsv = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelAmsv"));
    widgets.labelAmsav = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelAmsav"));
    widgets.labelAmsp = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelAmsp"));
    widgets.labelAsv =  GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelAsv"));
    widgets.labelArms =  GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelArms"));
    widgets.labelBmsv = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelBmsv"));
    widgets.labelBmsav = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelBmsav"));
    widgets.labelBmsp = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelBmsp"));
    widgets.labelBsv =  GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelBsv"));
    widgets.labelBrms =  GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelBrms"));

    widgets.checkButton_AsReconstruct = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "checkButton_AsReconstruct"));
    widgets.comboBoxText_AsReconstructionMethod = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "comboBoxText_AsReconstructionMethod"));
    widgets.entry_Asqt = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Asqt"));
    widgets.checkButton_BsReconstruct = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "checkButton_BsReconstruct"));
    widgets.comboBoxText_BsReconstructionMethod = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "comboBoxText_BsReconstructionMethod"));
    widgets.entry_Bsqt = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_Bsqt"));

    widgets.label_AsNeighCoeff = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_AsNeighCoeff"));
    widgets.entry_AsNeighCoeffVal = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_AsNeighCoeffVal"));
    widgets.label_BsNeighCoeff = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "label_BsNeighCoeff"));
    widgets.entry_BsNeighCoeffVal = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "entry_BsNeighCoeffVal"));

    widgets.labelAsmse = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelAsmse"));
    widgets.labelAssnr = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelAssnr"));
    widgets.labelAspsnr = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelAspsnr"));
    widgets.labelAsmd = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelAsmd"));
    widgets.labelAqmse = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelAqmse"));
    widgets.labelAqsnr = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelAqsnr"));
    widgets.labelAqpsnr = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelAqpsnr"));
    widgets.labelAqmd = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelAqmd"));
    widgets.labelBsmse = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelBsmse"));
    widgets.labelBssnr = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelBssnr"));
    widgets.labelBspsnr = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelBspsnr"));
    widgets.labelBsmd = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelBsmd"));
    widgets.labelBqmse = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelBqmse"));
    widgets.labelBqsnr = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelBqsnr"));
    widgets.labelBqpsnr = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelBqpsnr"));
    widgets.labelBqmd = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelBqmd"));

    widgets.labelAenob = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelAenob"));
    widgets.labelBenob = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelBenob"));

    widgets.button_cpy = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "button_cpy"));
    widgets.button_collapseTDomains = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "button_collapseTDomains"));

    widgets.button_Atimeshift = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "button_Atimeshift"));
    widgets.button_Afir = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "button_Afir"));
    widgets.button_Btimeshift = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "button_Btimeshift"));
    widgets.button_Bfir = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "button_Bfir"));

    widgets.labelAargmax = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelAargmax"));
    widgets.labelAcdist = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelAcdist"));
    widgets.labelBargmax = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelBargmax"));
    widgets.labelBcdist = GTK_WIDGET(gtk_builder_get_object(builders.viewBuilder, "labelBcdist"));


    set_param_names(get_signal_idx_a(), SIGNAL_A);
    set_param_names(get_signal_idx_b(), SIGNAL_B);
    init_scales();
    
    init_reconstruction_gui();

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

static void shift_signal_a(double timeshiftVal) {
    pseudo_enable_window(GTK_WINDOW(widgets.window));
    if (timeshiftVal != 0.0) {
        g_message("Signal A shift by %f requested.", timeshiftVal);
        real_signal_timeshift(&signals.signalA, timeshiftVal);

        // Set signal type as custom (the additional type)
        gtk_combo_box_set_active(GTK_COMBO_BOX (widgets.comboBoxText_Astype), (gint)(NUM_SIGNALS - 1)); 
        set_param_names (NUM_SIGNALS - 1, SIGNAL_A);
        update_A_plots();
    }
}

static void shift_signal_b(double timeshiftVal) {
    pseudo_enable_window(GTK_WINDOW(widgets.window));
    if (timeshiftVal != 0.0) {
        g_message("Signal B shift by %f requested.", timeshiftVal);
        real_signal_timeshift(&signals.signalB, timeshiftVal);

        // Set signal type as custom (the additional type)
        gtk_combo_box_set_active(GTK_COMBO_BOX (widgets.comboBoxText_Bstype), (gint)(NUM_SIGNALS - 1)); 
        set_param_names (NUM_SIGNALS - 1, SIGNAL_B);
        update_B_plots();
    }
}

static void filter_signal_a(fir_common_config_t config) {
    pseudo_enable_window(GTK_WINDOW(widgets.window));
    if (config.filterType != -1) {
        g_message("Signal A: FIR filter requested.");
        fir_common_config_print(&config);
        real_signal_t s = { 
            .info = {
                .num_samples = signals.signalA.info.num_samples,
                .sampling_frequency = signals.signalA.info.sampling_frequency,
                .start_time = signals.signalA.info.start_time
            }
        };
        real_signal_alloc_values(&s);
        switch (config.filterType) {
            case FIR_FILTER_TYPE_LOWPASS:
                fir_filter_real_signal_lowpass(&signals.signalA, &s, &config.oneSidedConfig);
                break;
            case FIR_FILTER_TYPE_HIGHPASS:
                fir_filter_real_signal_highpass(&signals.signalA, &s, &config.oneSidedConfig);
                break;
            case FIR_FILTER_TYPE_BANDPASS:
                fir_filter_real_signal_bandpass(&signals.signalA, &s, &config.doubleSidedConfig);
                break;
            default:
                g_error("Invalid config.filterType");
                break;
        }
        real_signal_free_values(&signals.signalA);
        signals.signalA.pValues = s.pValues;
    }
}

static void filter_signal_b(fir_common_config_t config) {
    pseudo_enable_window(GTK_WINDOW(widgets.window));
    if (config.filterType != -1) {
        g_message("Signal B: FIR filter requested.");
        fir_common_config_print(&config);
        real_signal_t s = { 
            .info = {
                .num_samples = signals.signalB.info.num_samples,
                .sampling_frequency = signals.signalB.info.sampling_frequency,
                .start_time = signals.signalB.info.start_time
            }
        };
        real_signal_alloc_values(&s);
        switch (config.filterType) {
            case FIR_FILTER_TYPE_LOWPASS:
                fir_filter_real_signal_lowpass(&signals.signalB, &s, &config.oneSidedConfig);
                break;
            case FIR_FILTER_TYPE_HIGHPASS:
                fir_filter_real_signal_highpass(&signals.signalB, &s, &config.oneSidedConfig);
                break;
            case FIR_FILTER_TYPE_BANDPASS:
                fir_filter_real_signal_bandpass(&signals.signalB, &s, &config.doubleSidedConfig);
                break;
            default:
                g_error("Invalid config.filterType");
                break;
        }
        real_signal_free_values(&signals.signalB);
        signals.signalB.pValues = s.pValues;
    }
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
        case 4U:
            convolve_signal (&signals.signalA, &signals.signalB);
            break;
        case 5U:
            cross_correlate_signal_1 (&signals.signalA, &signals.signalB);
            break;
        case 6U:
            cross_correlate_signal_2 (&signals.signalA, &signals.signalB);
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
    char* _asfStr = (char*)gtk_entry_get_text (GTK_ENTRY(widgets.entry_Asf));
    char* _bsfStr = (char*)gtk_entry_get_text (GTK_ENTRY(widgets.entry_Bsf));

    char p1vaStr[20]; char p2vaStr[20]; char p3vaStr[20]; char p4vaStr[20]; char p5vaStr[20];
    char p1vbStr[20]; char p2vbStr[20]; char p3vbStr[20]; char p4vbStr[20]; char p5vbStr[20];
    char asfStr[20]; char bsfStr[20];
    strcpy(p1vaStr, _p1vaStr);strcpy(p2vaStr, _p2vaStr);strcpy(p3vaStr, _p3vaStr);strcpy(p4vaStr, _p4vaStr);strcpy(p5vaStr, _p5vaStr);
    strcpy(p1vbStr, _p1vbStr);strcpy(p2vbStr, _p2vbStr);strcpy(p3vbStr, _p3vbStr);strcpy(p4vbStr, _p4vbStr);strcpy(p5vbStr, _p5vbStr);
    strcpy(asfStr, _asfStr);strcpy(bsfStr, _bsfStr);

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
    gtk_entry_set_text (GTK_ENTRY(widgets.entry_Asf), (const gchar*)bsfStr);
    gtk_entry_set_text (GTK_ENTRY(widgets.entry_Bsf), (const gchar*)asfStr);



    double adjA = (double)gtk_adjustment_get_value(GTK_ADJUSTMENT(widget_helpers.adjustment1));
    double adjB = (double)gtk_adjustment_get_value(GTK_ADJUSTMENT(widget_helpers.adjustment2));
    gtk_adjustment_set_value (GTK_ADJUSTMENT(widget_helpers.adjustment1), (gdouble)adjB);
    gtk_adjustment_set_value (GTK_ADJUSTMENT(widget_helpers.adjustment2), (gdouble)adjA);
    


    char* _amsvStr = (char*)gtk_label_get_text (GTK_LABEL(widgets.labelAmsv));
    char* _amsavStr = (char*)gtk_label_get_text (GTK_LABEL(widgets.labelAmsav));
    char* _amspStr = (char*)gtk_label_get_text (GTK_LABEL(widgets.labelAmsp));
    char* _asvStr = (char*)gtk_label_get_text (GTK_LABEL(widgets.labelAsv));
    char* _armsStr = (char*)gtk_label_get_text (GTK_LABEL(widgets.labelArms));
    char* _asmseStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAsmse));
    char* _assnrStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAssnr));
    char* _aspsnrStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAspsnr));
    char* _asmdStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAsmd));
    char* _aqmseStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAqmse));
    char* _aqsnrStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAqsnr));
    char* _aqpsnrStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAqpsnr));
    char* _aqmdStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAqmd));
    char* _aenobStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAenob));
    char* _aargmaxStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAargmax));
    char* _acdistStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAcdist));
    char* _bmsvStr = (char*)gtk_label_get_text (GTK_LABEL(widgets.labelBmsv));
    char* _bmsavStr = (char*)gtk_label_get_text (GTK_LABEL(widgets.labelBmsav));
    char* _bmspStr = (char*)gtk_label_get_text (GTK_LABEL(widgets.labelBmsp));
    char* _bsvStr = (char*)gtk_label_get_text (GTK_LABEL(widgets.labelBsv));
    char* _brmsStr = (char*)gtk_label_get_text (GTK_LABEL(widgets.labelBrms));
    char* _bsmseStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelBsmse));
    char* _bssnrStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelBssnr));
    char* _bspsnrStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelBspsnr));
    char* _bsmdStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelBsmd));
    char* _bqmseStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelBqmse));
    char* _bqsnrStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelBqsnr));
    char* _bqpsnrStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelBqpsnr));
    char* _bqmdStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelBqmd));
    char* _benobStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelBenob));
    char* _bargmaxStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelBargmax));
    char* _bcdistStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelBcdist));

    char amsvStr[20]; char amsavStr[20]; char amspStr[20]; char asvStr[20]; char armsStr[20]; char bmsvStr[20]; char bmsavStr[20]; char bmspStr[20]; char bsvStr[20]; char brmsStr[20];
    char asmseStr[20]; char assnrStr[20]; char aspsnrStr[20]; char asmdStr[20]; char bsmseStr[20]; char bssnrStr[20]; char bspsnrStr[20]; char bsmdStr[20];
    char aqmseStr[20]; char aqsnrStr[20]; char aqpsnrStr[20]; char aqmdStr[20]; char bqmseStr[20]; char bqsnrStr[20]; char bqpsnrStr[20]; char bqmdStr[20];
    char aenobStr[20]; char benobStr[20];
    char aargmaxStr[20]; char acdistStr[20]; char bargmaxStr[20]; char bcdistStr[20];
    strcpy(amsvStr, _amsvStr);strcpy(amsavStr, _amsavStr);strcpy(amspStr, _amspStr);strcpy(asvStr, _asvStr);strcpy(armsStr, _armsStr); strcpy(bmsvStr, _bmsvStr);strcpy(bmsavStr, _bmsavStr);strcpy(bmspStr, _bmspStr);strcpy(bsvStr, _bsvStr);strcpy(brmsStr, _brmsStr);
    strcpy(asmseStr, _asmseStr);strcpy(assnrStr, _assnrStr);strcpy(aspsnrStr, _aspsnrStr);strcpy(asmdStr, _asmdStr); strcpy(bsmseStr, _bsmseStr);strcpy(bssnrStr, _bssnrStr);strcpy(bspsnrStr, _bspsnrStr);strcpy(bsmdStr, _bsmdStr);
    strcpy(aqmseStr, _aqmseStr);strcpy(aqsnrStr, _aqsnrStr);strcpy(aqpsnrStr, _aqpsnrStr);strcpy(aqmdStr, _aqmdStr); strcpy(bqmseStr, _bqmseStr);strcpy(bqsnrStr, _bqsnrStr);strcpy(bqpsnrStr, _bqpsnrStr);strcpy(bqmdStr, _bqmdStr);
    strcpy(aenobStr, _aenobStr); strcpy(benobStr, _benobStr);
    strcpy(aargmaxStr, _aargmaxStr);strcpy(acdistStr, _acdistStr); strcpy(bargmaxStr, _bargmaxStr);strcpy(bcdistStr, _bcdistStr);

    gtk_label_set_text(GTK_LABEL(widgets.labelBmsv), (const gchar*)amsvStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBmsav), (const gchar*)amsavStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBmsp), (const gchar*)amspStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBsv), (const gchar*)asvStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBrms), (const gchar*)armsStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAmsv), (const gchar*)bmsvStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAmsav), (const gchar*)bmsavStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAmsp), (const gchar*)bmspStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAsv), (const gchar*)bsvStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelArms), (const gchar*)brmsStr);

    gtk_label_set_text(GTK_LABEL(widgets.labelBsmse), (const gchar*)asmseStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBssnr), (const gchar*)assnrStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBspsnr), (const gchar*)aspsnrStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBsmd), (const gchar*)asmdStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAsmse), (const gchar*)bsmseStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAssnr), (const gchar*)bssnrStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAspsnr), (const gchar*)bspsnrStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAsmd), (const gchar*)bsmdStr);

    gtk_label_set_text(GTK_LABEL(widgets.labelBqmse), (const gchar*)aqmseStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBqsnr), (const gchar*)aqsnrStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBqpsnr), (const gchar*)aqpsnrStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBqmd), (const gchar*)aqmdStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAqmse), (const gchar*)bqmseStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAqsnr), (const gchar*)bqsnrStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAqpsnr), (const gchar*)bqpsnrStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAqmd), (const gchar*)bqmdStr);

    gtk_label_set_text(GTK_LABEL(widgets.labelBenob), (const gchar*)aenobStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAenob), (const gchar*)benobStr);

    gtk_label_set_text(GTK_LABEL(widgets.labelBargmax), (const gchar*)aargmaxStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBcdist), (const gchar*)acdistStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAargmax), (const gchar*)bargmaxStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAcdist), (const gchar*)bcdistStr);

    gtk_label_set_text(GTK_LABEL(widgets.labelBargmax), (const gchar*)aargmaxStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBcdist), (const gchar*)acdistStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAargmax), (const gchar*)bargmaxStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelAcdist), (const gchar*)bcdistStr);


    char* _asqtStr = (char*)gtk_entry_get_text(GTK_ENTRY(widgets.entry_Asqt));
    char* _bsqtStr = (char*)gtk_entry_get_text(GTK_ENTRY(widgets.entry_Bsqt));
    char asqtStr[20]; char bsqtStr[20];
    strcpy(asqtStr, _asqtStr); strcpy(bsqtStr, _bsqtStr);
    
    gtk_entry_set_text(GTK_ENTRY(widgets.entry_Bsqt), (const gchar*)asqtStr);
    gtk_entry_set_text(GTK_ENTRY(widgets.entry_Asqt), (const gchar*)bsqtStr);

    gboolean asReconstruct = get_sReconstruct(SIGNAL_A);
    gboolean bsReconstruct = get_sReconstruct(SIGNAL_B);
    set_sReconstruct(bsReconstruct, SIGNAL_A);
    set_sReconstruct(asReconstruct, SIGNAL_B);
    gint asReconstructionMethod = get_sReconstructionMethod(SIGNAL_A);
    gint bsReconstructionMethod = get_sReconstructionMethod(SIGNAL_B);
    set_sReconstructionMethod(bsReconstructionMethod, SIGNAL_A);
    set_sReconstructionMethod(asReconstructionMethod, SIGNAL_B);
    
    const gchar* _asNeighCoeffValStr = get_sNeighCoeffVal(SIGNAL_A);
    const gchar* _bsNeighCoeffValStr = get_sNeighCoeffVal(SIGNAL_B);
    gchar asNeighCoeffValStr[20]; gchar bsNeighCoeffValStr[20];
    strcpy(asNeighCoeffValStr, _asNeighCoeffValStr); strcpy(bsNeighCoeffValStr, _bsNeighCoeffValStr);
    set_sNeighCoeffVal(bsNeighCoeffValStr, SIGNAL_A);
    set_sNeighCoeffVal(asNeighCoeffValStr, SIGNAL_B);
    
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
    //https://docs.gtk.org/gtk3/class.FileChooserDialog.html

    GtkWidget *dialog;
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;

    dialog = gtk_file_chooser_dialog_new ("Save File",
                                        GTK_WINDOW(widgets.window),
                                        action,
                                        /*_("_Cancel")*/"Cancel",
                                        GTK_RESPONSE_CANCEL,
                                        /*_("_Save")*/"Save",
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);
    chooser = GTK_FILE_CHOOSER (dialog);

    gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);

    gtk_file_chooser_set_current_name (chooser,
                                        /*_("Untitled document")*/"Untitled signal");
    

    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;

        filename = gtk_file_chooser_get_filename (chooser);
        
        fprintf(stdout, "Info: Saving signal A (binary) into file '%s'\n", filename);
        real_signal_file_payload_t payload = real_signal_file_payload_create (&signals.signalA);
        fio_write_rpayload (&payload, (const char*)filename);

        g_free (filename);
    }

    gtk_widget_destroy (dialog);
}

void on_button_Asave_txt_clicked(GtkButton* b) {
    //https://docs.gtk.org/gtk3/class.FileChooserDialog.html

    GtkWidget *dialog;
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;

    dialog = gtk_file_chooser_dialog_new ("Save File",
                                        GTK_WINDOW(widgets.window),
                                        action,
                                        /*_("_Cancel")*/"Cancel",
                                        GTK_RESPONSE_CANCEL,
                                        /*_("_Save")*/"Save",
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);
    chooser = GTK_FILE_CHOOSER (dialog);

    gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);

    gtk_file_chooser_set_current_name (chooser,
                                        /*_("Untitled document")*/"Untitled signal");
    

    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;

        filename = gtk_file_chooser_get_filename (chooser);
        
        fprintf(stdout, "Info: Saving signal A (human-readable) into file '%s'\n", filename);
        real_signal_file_payload_t payload = real_signal_file_payload_create (&signals.signalA);
        fio_write_rpayload_human_readable(&payload, (const char*)filename);

        g_free (filename);
    }

    gtk_widget_destroy (dialog);
}

void on_button_Aload_clicked(GtkButton* b) {
    if (widget_helpers.a_load_filename != NULL) {
        real_signal_file_payload_t payload = fio_read_rpayload ((const char*)widget_helpers.a_load_filename);
        real_signal_t fetchedSignal = fetch_rsignal (&payload);

        real_signal_free_values (&signals.signalA);
        signals.signalA.info.num_samples = fetchedSignal.info.num_samples;
        signals.signalA.info.sampling_frequency = fetchedSignal.info.sampling_frequency;
        signals.signalA.info.start_time = fetchedSignal.info.start_time;
        signals.signalA.pValues = fetchedSignal.pValues; //passing buffer ownership

        double asf = signals.signalA.info.sampling_frequency;
        //printf("%f / %f / %f\n", asf, signals.signalA.info.sampling_frequency, fetchedSignal.info.sampling_frequency);
        char asfStr[50]; snprintf(asfStr, 50, "%f", asf);
        gtk_entry_set_text (GTK_ENTRY(widgets.entry_Asf), (const gchar*)asfStr);

        // Set signal type as custom (the additional type)
        gtk_combo_box_set_active(GTK_COMBO_BOX (widgets.comboBoxText_Astype), (gint)(NUM_SIGNALS - 1)); 
        set_param_names (NUM_SIGNALS - 1, SIGNAL_A);

        
        update_A_plots();
    }
}

void on_button_Bload_clicked(GtkButton* b) {
    if (widget_helpers.b_load_filename != NULL) {
        real_signal_file_payload_t payload = fio_read_rpayload ((const char*)widget_helpers.b_load_filename);
        real_signal_t fetchedSignal = fetch_rsignal (&payload);

        real_signal_free_values (&signals.signalB);
        signals.signalB.info.num_samples = fetchedSignal.info.num_samples;
        signals.signalB.info.sampling_frequency = fetchedSignal.info.sampling_frequency;
        signals.signalB.info.start_time = fetchedSignal.info.start_time;
        signals.signalB.pValues = fetchedSignal.pValues; //passing buffer ownership

        double bsf = signals.signalB.info.sampling_frequency;
        //printf("%f / %f / %f\n", asf, signals.signalA.info.sampling_frequency, fetchedSignal.info.sampling_frequency);
        char bsfStr[50]; snprintf(bsfStr, 50, "%f", bsf);
        gtk_entry_set_text (GTK_ENTRY(widgets.entry_Bsf), (const gchar*)bsfStr);

        // Set signal type as custom (the additional type)
        gtk_combo_box_set_active(GTK_COMBO_BOX (widgets.comboBoxText_Bstype), (gint)(NUM_SIGNALS - 1)); 
        set_param_names (NUM_SIGNALS - 1, SIGNAL_B);
        
        update_B_plots();
    }
}

void on_entry_Asf_changed(GtkEntry* e) {
    if (get_reconstruction_mode_enabled_a() == 1U) {
        g_message("Calling reconstruct_signal for SIGNAL_A");
        //load_signal_A();
        reconstruct_signal(SIGNAL_A);
        update_A_plots_no_sigload();
    } else {
        update_A_plots();
    }
    
}

void on_entry_Bsf_changed(GtkEntry* e) {
    if (get_reconstruction_mode_enabled_b() == 1U) {
        g_message("Calling reconstruct_signal for SIGNAL_B");
        //load_signal_B();
        reconstruct_signal(SIGNAL_B);
        update_B_plots_no_sigload();
    } else {
        update_B_plots();
    }
}

void on_scaleA_value_changed(GtkScale* s) {
    update_A_plots_no_sigload();
}

void on_scaleB_value_changed(GtkScale* s) {
    update_B_plots_no_sigload();
}

void on_fileChooserButton_ALoad_file_set(GtkFileChooserButton* fcb) {
    const gchar* fileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(fcb));
    fprintf(stdout, "Info: '%s' choosen for reading signal A\n", fileName);
    widget_helpers.a_load_filename = (char*)fileName;
}

void on_fileChooserButton_BLoad_file_set(GtkFileChooserButton* fcb) {
    const gchar* fileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(fcb));
    fprintf(stdout, "Info: '%s' choosen for reading signal B\n", fileName);
    widget_helpers.b_load_filename = (char*)fileName;
}

void on_entry_Asqt_changed(GtkEntry* e) {
    //update_A_plots();
    //load_signal_A();

    //reconstruct_signal(SIGNAL_A);
    quantization_handler_A();
    update_A_plots_no_sigload();
}

void on_entry_Bsqt_changed(GtkEntry* e) {
    //update_B_plots();
    //load_signal_B();

    //reconstruct_signal(SIGNAL_B);
    quantization_handler_B();
    update_B_plots_no_sigload();
}

void on_checkButton_AsReconstruct_toggled(GtkToggleButton* t) { 
    if (get_reconstruction_mode_enabled_a() == 1U) {
        if (get_signal_idx_a() == NUM_SIGNALS - 1) {
            enable_entry (GTK_ENTRY(widgets.entry_Asf));
        }

        gtk_widget_set_visible(widgets.comboBoxText_AsReconstructionMethod, TRUE);
    
        uint8_t reconstruction_method_index = get_reconstruction_method_idx_a();
        if (reconstruction_method_index == 0) {
            // Keep other configuration widgets hidden
        }
        else if (reconstruction_method_index == 1) {
            // Show sinc reconstruction configuration widgets
            gtk_widget_set_visible(widgets.label_AsNeighCoeff, TRUE);
            gtk_widget_set_visible(widgets.entry_AsNeighCoeffVal, TRUE);
        } else {
            g_error("Invalid reconstruction method selected for signal A!");
        }
    } else {
        if (get_signal_idx_a() == NUM_SIGNALS - 1) {
            disable_entry (GTK_ENTRY(widgets.entry_Asf));
        }

        gtk_widget_set_visible(widgets.comboBoxText_AsReconstructionMethod, FALSE);
        gtk_widget_set_visible(widgets.label_AsNeighCoeff, FALSE);
        gtk_widget_set_visible(widgets.entry_AsNeighCoeffVal, FALSE);
    }
    
}

void on_checkButton_BsReconstruct_toggled(GtkToggleButton* t) {
    if (get_reconstruction_mode_enabled_b() == 1U) {
        if (get_signal_idx_b() == NUM_SIGNALS - 1) {
            enable_entry (GTK_ENTRY(widgets.entry_Bsf));
        }


        gtk_widget_set_visible(widgets.comboBoxText_BsReconstructionMethod, TRUE);
    
        uint8_t reconstruction_method_index = get_reconstruction_method_idx_b();
        if (reconstruction_method_index == 0) {
            // Keep other configuration widgets hidden
        }
        else if (reconstruction_method_index == 1) {
            // Show sinc reconstruction configuration widgets
            gtk_widget_set_visible(widgets.label_BsNeighCoeff, TRUE);
            gtk_widget_set_visible(widgets.entry_BsNeighCoeffVal, TRUE);
        } else {
            g_error("Invalid reconstruction method selected for signal B!");
        }
    } else {
        if (get_signal_idx_b() == NUM_SIGNALS - 1) {
            disable_entry (GTK_ENTRY(widgets.entry_Bsf));
        }

        gtk_widget_set_visible(widgets.comboBoxText_BsReconstructionMethod, FALSE);
        gtk_widget_set_visible(widgets.label_BsNeighCoeff, FALSE);
        gtk_widget_set_visible(widgets.entry_BsNeighCoeffVal, FALSE);
    }
    
    
}

void on_comboBoxText_AsReconstructionMethod_changed(GtkComboBox* c, gpointer user_data) {
    uint8_t reconstruction_method_index = get_reconstruction_method_idx_a();
    if (reconstruction_method_index == 0) {
        // Keep other configuration widgets hidden
        gtk_widget_set_visible(widgets.label_AsNeighCoeff, FALSE);
        gtk_widget_set_visible(widgets.entry_AsNeighCoeffVal, FALSE);
    }
    else if (reconstruction_method_index == 1) {
        // Show sinc reconstruction configuration widgets
        gtk_widget_set_visible(widgets.label_AsNeighCoeff, TRUE);
        gtk_widget_set_visible(widgets.entry_AsNeighCoeffVal, TRUE);
    } else {
        g_error("Invalid reconstruction method selected for signal A!");
    }
}

void on_comboBoxText_BsReconstructionMethod_changed(GtkComboBox* c, gpointer user_data) {
    uint8_t reconstruction_method_index = get_reconstruction_method_idx_b();
    if (reconstruction_method_index == 0) {
        // Keep other configuration widgets hidden
        gtk_widget_set_visible(widgets.label_BsNeighCoeff, FALSE);
        gtk_widget_set_visible(widgets.entry_BsNeighCoeffVal, FALSE);
    }
    else if (reconstruction_method_index == 1) {
        // Show sinc reconstruction configuration widgets
        gtk_widget_set_visible(widgets.label_BsNeighCoeff, TRUE);
        gtk_widget_set_visible(widgets.entry_BsNeighCoeffVal, TRUE);
    } else {
        g_error("Invalid reconstruction method selected for signal B!");
    }
}

void on_entry_AsNeighCoeffVal_changed(GtkEntry* e) {
    // Reconstruct signal
    reconstruct_signal(SIGNAL_A);
    
    // Update plots
    update_A_plots_no_sigload();
    
    //g_error("Not implemented");
}

void on_entry_BsNeighCoeffVal_changed(GtkEntry* e) {
    // Reconstruct signal
    reconstruct_signal(SIGNAL_B);
    
    // Update plots
    update_B_plots_no_sigload();
    
    //g_error("Not implemented");
}

void on_button_cpy_clicked(GtkButton* b) {
    real_signal_free_values(&signals.signalB);
    signals.signalB.info.num_samples = signals.signalA.info.num_samples;
    signals.signalB.info.sampling_frequency = signals.signalA.info.sampling_frequency;
    signals.signalB.info.start_time = signals.signalA.info.start_time;
    real_signal_alloc_values(&signals.signalB);
    memcpy(signals.signalB.pValues, signals.signalA.pValues, sizeof(double) * signals.signalA.info.num_samples);

    uint32_t signalA_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets.comboBoxText_Astype));
    gtk_combo_box_set_active(GTK_COMBO_BOX (widgets.comboBoxText_Bstype), signalA_idx);
    set_param_names(signalA_idx, SIGNAL_B);

    char* _p1vaStr = (char*)gtk_entry_get_text (GTK_ENTRY(widgets.entries_Apval[0]));
    char* _p2vaStr = (char*)gtk_entry_get_text (GTK_ENTRY(widgets.entries_Apval[1]));
    char* _p3vaStr = (char*)gtk_entry_get_text (GTK_ENTRY(widgets.entries_Apval[2]));
    char* _p4vaStr = (char*)gtk_entry_get_text (GTK_ENTRY(widgets.entries_Apval[3]));
    char* _p5vaStr = (char*)gtk_entry_get_text (GTK_ENTRY(widgets.entries_Apval[4]));
    char* _asfStr = (char*)gtk_entry_get_text (GTK_ENTRY(widgets.entry_Asf));

    char p1vaStr[20]; char p2vaStr[20]; char p3vaStr[20]; char p4vaStr[20]; char p5vaStr[20];
    char asfStr[20];
    strcpy(p1vaStr, _p1vaStr);strcpy(p2vaStr, _p2vaStr);strcpy(p3vaStr, _p3vaStr);strcpy(p4vaStr, _p4vaStr);strcpy(p5vaStr, _p5vaStr);
    strcpy(asfStr, _asfStr);

    gtk_entry_set_text (GTK_ENTRY(widgets.entries_Bpval[0]), (const gchar*)p1vaStr);
    gtk_entry_set_text (GTK_ENTRY(widgets.entries_Bpval[1]), (const gchar*)p2vaStr);
    gtk_entry_set_text (GTK_ENTRY(widgets.entries_Bpval[2]), (const gchar*)p3vaStr);
    gtk_entry_set_text (GTK_ENTRY(widgets.entries_Bpval[3]), (const gchar*)p4vaStr);
    gtk_entry_set_text (GTK_ENTRY(widgets.entries_Bpval[4]), (const gchar*)p5vaStr);
    gtk_entry_set_text (GTK_ENTRY(widgets.entry_Bsf), (const gchar*)asfStr);

    double adjA = (double)gtk_adjustment_get_value(GTK_ADJUSTMENT(widget_helpers.adjustment1));

    gtk_adjustment_set_value (GTK_ADJUSTMENT(widget_helpers.adjustment2), (gdouble)adjA);


    char* _amsvStr = (char*)gtk_label_get_text (GTK_LABEL(widgets.labelAmsv));
    char* _amsavStr = (char*)gtk_label_get_text (GTK_LABEL(widgets.labelAmsav));
    char* _amspStr = (char*)gtk_label_get_text (GTK_LABEL(widgets.labelAmsp));
    char* _asvStr = (char*)gtk_label_get_text (GTK_LABEL(widgets.labelAsv));
    char* _armsStr = (char*)gtk_label_get_text (GTK_LABEL(widgets.labelArms));
    char* _asmseStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAsmse));
    char* _assnrStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAssnr));
    char* _aspsnrStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAspsnr));
    char* _asmdStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAsmd));
    char* _aqmseStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAqmse));
    char* _aqsnrStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAqsnr));
    char* _aqpsnrStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAqpsnr));
    char* _aqmdStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAqmd));
    char* _aenobStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAenob));
    char* _aargmaxStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAargmax));
    char* _acdistStr = (char*)gtk_label_get_text(GTK_LABEL(widgets.labelAcdist));

    char amsvStr[20]; char amsavStr[20]; char amspStr[20]; char asvStr[20]; char armsStr[20];
    char asmseStr[20]; char assnrStr[20]; char aspsnrStr[20]; char asmdStr[20];
    char aqmseStr[20]; char aqsnrStr[20]; char aqpsnrStr[20]; char aqmdStr[20];
    char aenobStr[20];
    char aargmaxStr[20]; char acdistStr[20];
    strcpy(amsvStr, _amsvStr);strcpy(amsavStr, _amsavStr);strcpy(amspStr, _amspStr);strcpy(asvStr, _asvStr);strcpy(armsStr, _armsStr);
    strcpy(asmseStr, _asmseStr);strcpy(assnrStr, _assnrStr);strcpy(aspsnrStr, _aspsnrStr);strcpy(asmdStr, _asmdStr);
    strcpy(aqmseStr, _aqmseStr);strcpy(aqsnrStr, _aqsnrStr);strcpy(aqpsnrStr, _aqpsnrStr);strcpy(aqmdStr, _aqmdStr);
    strcpy(aenobStr, _aenobStr);
    strcpy(aargmaxStr, _aargmaxStr);strcpy(acdistStr, _acdistStr);

    gtk_label_set_text(GTK_LABEL(widgets.labelBmsv), (const gchar*)amsvStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBmsav), (const gchar*)amsavStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBmsp), (const gchar*)amspStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBsv), (const gchar*)asvStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBrms), (const gchar*)armsStr);

    gtk_label_set_text(GTK_LABEL(widgets.labelBsmse), (const gchar*)asmseStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBssnr), (const gchar*)assnrStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBspsnr), (const gchar*)aspsnrStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBsmd), (const gchar*)asmdStr);

    gtk_label_set_text(GTK_LABEL(widgets.labelBqmse), (const gchar*)aqmseStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBqsnr), (const gchar*)aqsnrStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBqpsnr), (const gchar*)aqpsnrStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBqmd), (const gchar*)aqmdStr);

    gtk_label_set_text(GTK_LABEL(widgets.labelBenob), (const gchar*)aenobStr);

    gtk_label_set_text(GTK_LABEL(widgets.labelBargmax), (const gchar*)aargmaxStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBcdist), (const gchar*)acdistStr);

    gtk_label_set_text(GTK_LABEL(widgets.labelBargmax), (const gchar*)aargmaxStr);
    gtk_label_set_text(GTK_LABEL(widgets.labelBcdist), (const gchar*)acdistStr);


    char* _asqtStr = (char*)gtk_entry_get_text(GTK_ENTRY(widgets.entry_Asqt));
    char asqtStr[20];
    strcpy(asqtStr, _asqtStr);
    
    gtk_entry_set_text(GTK_ENTRY(widgets.entry_Bsqt), (const gchar*)asqtStr);

    gboolean asReconstruct = get_sReconstruct(SIGNAL_A);
    set_sReconstruct(asReconstruct, SIGNAL_B);
    gint asReconstructionMethod = get_sReconstructionMethod(SIGNAL_A);
    set_sReconstructionMethod(asReconstructionMethod, SIGNAL_B);
    
    const gchar* _asNeighCoeffValStr = get_sNeighCoeffVal(SIGNAL_A);
    gchar asNeighCoeffValStr[20];
    strcpy(asNeighCoeffValStr, _asNeighCoeffValStr);;
    set_sNeighCoeffVal(asNeighCoeffValStr, SIGNAL_B);
    
    update_B_plots();
}

void on_button_Atimeshift_clicked(GtkButton* b) {
    pseudo_disable_window(GTK_WINDOW(widgets.window));
    int rv = controller_timeshift_run ( shift_signal_a );
    g_message("controller_timeshift_run returned [rv=%d]", rv);
    /*sem_t* pSem = controller_timeshift_run();
    if (-1 == sem_wait(pSem)) {
        g_error("sem_wait failed [errno=%d]", errno);
    }
    g_message("sem_wait returned into on_button_Atimeshift_clicked");*/
}

void on_button_Btimeshift_clicked(GtkButton* b) {
    pseudo_disable_window(GTK_WINDOW(widgets.window));
    int rv = controller_timeshift_run ( shift_signal_b );
    g_message("controller_timeshift_run returned [rv=%d]", rv);
}

void on_button_Afir_clicked(GtkButton* b) {
    pseudo_disable_window(GTK_WINDOW(widgets.window));
    int rv = controller_fir_run ( filter_signal_a );
    g_message("controller_fir_run returned [rv=%d]", rv);
}

void on_button_Bfir_clicked(GtkButton* b) {
    pseudo_disable_window(GTK_WINDOW(widgets.window));
    int rv = controller_fir_run ( filter_signal_b );
    g_message("controller_fir_run returned [rv=%d]", rv);
}

void on_button_collapseTDomains_clicked(GtkButton* b) {
    real_signal_collapse_signals_tdomains(&signals.signalA, &signals.signalB);

    // Set signal types as custom (the additional type)
    gtk_combo_box_set_active(GTK_COMBO_BOX (widgets.comboBoxText_Astype), (gint)(NUM_SIGNALS - 1));
    gtk_combo_box_set_active(GTK_COMBO_BOX (widgets.comboBoxText_Bstype), (gint)(NUM_SIGNALS - 1));

    set_param_names (NUM_SIGNALS - 1, SIGNAL_A);
    set_param_names (NUM_SIGNALS - 1, SIGNAL_B);
    update_A_plots();
    update_B_plots();
    g_message("on_button_collapseTDomains_clicked end");
}