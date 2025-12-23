# Testing

Unit testing and validation tools for implementations.

## Testing Module Overview

The library includes a testing module (`mbtest.h`) to help validate your register configurations during development. This module provides functions to check common configuration errors that could lead to runtime issues.

Include `mbtest.h` and call these functions to validate your configuration.

All functions return `1` on success, `0` on failure. The optional `issue_addr` parameter will contain the problematic register address when a test fails.

## Available Test Functions

```c
/**
 * @brief Check that all coils are in ascending order
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_coils_asc(
    const struct mbcoil_desc_s *coils,
    size_t n_coils,
    uint16_t *issue_addr);

/**
 * @brief Check if all coil accesses are valid
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_coils_valid_access(
    const struct mbcoil_desc_s *coils,
    size_t n_coils,
    uint16_t *issue_addr);

/**
 * @brief Check if all coil bit indices are within valid range [0-7]
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_coils_valid_bit_index(
    const struct mbcoil_desc_s *coils,
    size_t n_coils,
    uint16_t *issue_addr);

/**
 * @brief Check that no coils have duplicate addresses
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_coils_no_duplicates(
    const struct mbcoil_desc_s *coils,
    size_t n_coils,
    uint16_t *issue_addr);

/**
 * @brief Comprehensive coil validation - runs all coil tests
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_coils_validate_all(
    const struct mbcoil_desc_s *coils,
    size_t n_coils,
    uint16_t *issue_addr);

/**
 * @brief Check that all registers are in ascending order
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_regs_asc(
    const struct mbreg_desc_s *regs,
    size_t n_regs,
    uint16_t *issue_addr);

/**
 * @brief Check that all register sizes are valid
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_regs_valid_size(
    const struct mbreg_desc_s *regs,
    size_t n_regs,
    uint16_t *issue_addr);

/**
 * @brief Check that no registers overlap
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_regs_dont_overlap(
    const struct mbreg_desc_s *regs,
    size_t n_regs,
    uint16_t *issue_addr);

    /**
 * @brief Check if all register data types are valid
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_regs_valid_data_type(
    const struct mbreg_desc_s *regs,
    size_t n_regs,
    uint16_t *issue_addr);

/**
 * @brief Check if all register accesses are valid
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_regs_valid_access(
    const struct mbreg_desc_s *regs,
    size_t n_regs,
    uint16_t *issue_addr);

/**
 * @brief Check if all register block accesses are valid
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_regs_valid_block_access(
    const struct mbreg_desc_s *regs,
    size_t n_regs,
    uint16_t *issue_addr);

/**
 * @brief Comprehensive register validation - runs all register tests
 * @param issue_addr Optional - e.g. printf("0x%04x\n", issue_addr);
 * @return 1 success, 0 failure
 */
extern int mbtest_regs_validate_all(
    const struct mbreg_desc_s *regs,
    size_t n_regs,
    uint16_t *issue_addr);

/**
 * @brief Check that all file numbers are ascending and that there no no duplicates
 * @param issue_file_no Optional - e.g. printf("0x%04x\n", issue_file_no);
 * @return 1 success, 0 failure
 *
 * @note Assumes files are in ascending order
 */
extern int mbtest_files_asc_and_unique(
    const struct mbfile_desc_s *files,
    size_t n_files,
    uint16_t *issue_file_no);
```
