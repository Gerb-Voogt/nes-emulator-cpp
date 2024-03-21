#pragma once
// Functions for unit testing

// TODO: Write unit tests for testing the different addressing modes
// TODO: Write unit tests for ADC
// TODO: Write unit tests for AND
// TODO: Write unit tests for ASL
// TODO: Write unit tests for BCC
// TODO: Write unit tests for BCS
// TODO: Write unit tests for INY


// lda
void test_lda_immediate_load_state();
void test_lda_zero_flag();

// tax
void test_tax_load_state();
void test_tax_zero_flag();

// inx
void test_inx();
void test_inx_overflow();

// iny
void test_iny();
void test_iny_overflow();

