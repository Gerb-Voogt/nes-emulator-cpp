#pragma once
// Functions for unit testing

// TODO: Write unit tests for testing the different addressing modes
// TODO: Write unit tests for ADC
// TODO: Write unit tests for AND
// TODO: Write unit tests for ASL
// TODO: Write unit tests for BCC
// TODO: Write unit tests for BCS
// TODO: Write unit tests for INY


// adc
int test_adc();
int test_adc_status_updates();

// lda
int test_lda_immediate_load_state();
int test_lda_zero_flag();

// tax
int test_tax_load_state();
int test_tax_zero_flag();

// inx
int test_inx();
int test_inx_overflow();

// iny
int test_iny();
int test_iny_overflow();
