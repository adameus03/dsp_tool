#include "signal_fio.h"
#include <stdlib.h>
#include <stdio.h>

void __real_signal_file_payload_alloc_data (real_signal_file_payload_t* pPayload) {
    pPayload->pData = (double*) malloc(pPayload->header.info.num_samples * sizeof(double));
}

void __real_signal_file_payload_free_data (real_signal_file_payload_t* pPayload) {
    free (pPayload->pData);
}

real_signal_file_payload_t real_signal_file_payload_create (real_signal_t* pSignal) {
    real_signal_file_payload_t filePayload = {
        .header.info = pSignal->info,
        .pData = 0
    };
    //__real_signal_file_payload_alloc_data (&filePayload);
    filePayload.pData = pSignal->pValues;
    return filePayload;
}
void real_signal_file_payload_destroy (real_signal_file_payload_t* pPayload) {
    __real_signal_file_payload_free_data (pPayload);
}

real_signal_t fetch_rsignal (real_signal_file_payload_t* pPayload) {
    real_signal_t realSignal = {
        .info = pPayload->header.info,
        .pValues = pPayload->pData
    };
    return realSignal;
}

void fio_write_rpayload (real_signal_file_payload_t* pPayload, const char* filePath) {
    FILE* pFile = fopen (filePath, "w");
    if (pFile == NULL) {
        fprintf(stderr, "Error: Failed to open file '%s' for writing rsignal\n", filePath);
        return;
    }
    
    size_t n = fwrite (pPayload->header.raw, sizeof(uint64_t), sizeof(pPayload->header.raw) / sizeof(uint64_t), pFile);
    n += fwrite (pPayload->pData, sizeof(double), pPayload->header.info.num_samples, pFile);

    if (fclose (pFile) != 0) {
        fprintf(stderr, "Error: Failure while closing rsignal payload output file\n");
    }

    if (n != (pPayload->header.info.num_samples + sizeof(pPayload->header.raw) / sizeof(uint64_t))) {
        fprintf(stderr, "Error: File I/O failure while writing rsignal file payload\n");
    }
}

void fio_write_rpayload_human_readable (real_signal_file_payload_t* pPayload, const char* filePath) {
    FILE* pFile = fopen (filePath, "w");
    if (pFile == NULL) {
        fprintf(stderr, "Error: Failed to open file '%s' for writing human-readable rsignal\n", filePath);
        return;
    }
    
    fprintf(pFile, "start_time sampling_frequency num_samples\n");
    fprintf(pFile, "%f %f %lu\n\n", pPayload->header.info.start_time, pPayload->header.info.sampling_frequency, pPayload->header.info.num_samples);

    fprintf(pFile, "# value\n");
    for (uint64_t i = 0; i < pPayload->header.info.num_samples; i++) {
        double* pValue = pPayload->pData + i;
        fprintf(pFile, "%lu %f\n", i, *pValue);
    }
    
    if (fclose (pFile) != 0) {
        fprintf(stderr, "Error: Failure while closing rsignal human-readable payload output file\n");
    }
}

#define return_blank_signal_payload return (real_signal_file_payload_t) { .header.info = { .num_samples = 0, .sampling_frequency = 0, .start_time = 0 }, .pData = 0 }

real_signal_file_payload_t fio_read_rpayload (const char* filePath) {
    FILE* pFile = fopen (filePath, "r");
    if (pFile == NULL) {
        fprintf(stderr, "Error: Failed to open file '%s' for reading rsignal\n", filePath);
        return_blank_signal_payload;
    }
    
    real_signal_file_payload_t payload;
    size_t n = fread (payload.header.raw, sizeof(uint64_t), sizeof(payload.header.raw)/sizeof(uint64_t), pFile);
    __real_signal_file_payload_alloc_data (&payload);
    n += fread (payload.pData, sizeof(double), payload.header.info.num_samples, pFile);

    if (fclose(pFile) != 0) {
        fprintf(stderr, "Error: Failure while closing rsignal payload input file\n");
        return_blank_signal_payload;
    }

    if (n != (payload.header.info.num_samples + sizeof(payload.header.raw) / sizeof(uint64_t))) {
        fprintf(stderr, "Error: File I/O failure while reading rsignal file payload\n");
        return_blank_signal_payload;
    }

    return payload;
}


