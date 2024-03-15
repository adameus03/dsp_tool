#include "signal.h"

/**
 * @attention This code lacks documentation on memory handling for now, though you can try to figure it out
 * In the program, those functions are othen used while passing buffer ownership, so thay actually won't call `real_signal_file_payload_create` and `real_signal_file_payload_destroy` in that case
*/

typedef struct {
    union {
        signal_info_t info;
        uint64_t raw[3];
    } header;
    double* pData;
} real_signal_file_payload_t;

real_signal_file_payload_t real_signal_file_payload_create (real_signal_t* pSignal);
void real_signal_file_payload_destroy (real_signal_file_payload_t* pPayload);

real_signal_t fetch_rsignal (real_signal_file_payload_t* pPayload);

void fio_write_rpayload (real_signal_file_payload_t* pPayload, const char* filePath);

void fio_write_rpayload_human_readable (real_signal_file_payload_t* pPayload, const char* filePath);

real_signal_file_payload_t fio_read_rpayload (const char* filePath);