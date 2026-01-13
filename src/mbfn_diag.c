/**
 * @file mbfn_diag.c
 * @brief Implementation of Modbus diagnostic function handlers
 * @author Jonas Almås
 */

/*
 * Copyright (c) 2025 Siemens Energy AS
 */

#include "mbfn_diag.h"
#include "endian.h"
#include <string.h>

static void reset_comm_counters(struct mbinst_s *inst)
{
    inst->state.comm_event_counter = 0u;

    inst->state.bus_msg_counter = 0u;
    inst->state.bus_comm_err_counter = 0u;
    inst->state.exception_counter = 0u;
    inst->state.msg_counter = 0u;
    inst->state.no_resp_counter = 0u;
    inst->state.nak_counter = 0u;
    inst->state.busy_counter = 0u;
    inst->state.bus_char_overrun_counter = 0u;
}

static enum mbstatus_e loopback(
    const uint8_t *req,
    size_t req_len,
    struct mbpdu_buf_s *res)
{
    (void)memcpy(res->p, req, req_len);
    res->size = req_len;
    return MB_OK;
}

static enum mbstatus_e restart_comms_opt(
    struct mbinst_s *inst,
    const uint8_t *req,
    size_t req_len,
    struct mbpdu_buf_s *res)
{
    uint16_t val;

    if (req_len != 5u) return MB_ILLEGAL_DATA_VAL;

    val = betou16(req+3u);
    if ((val!=0x0000u) && (val!=0xFF00u)) return MB_ILLEGAL_DATA_VAL;

    if (inst->serial.request_restart!=NULL) {
        inst->serial.request_restart();
    }
    inst->state.is_listen_only = 0;
    reset_comm_counters(inst);

    if (val==0xFF00u) {
        inst->state.event_log_write_pos = 0;
        inst->state.event_log_count = 0;
    } else {
        mb_add_comm_event(inst, MB_COMM_EVENT_COMM_RESTART);
    }

    u16tobe(val, res->p+3u);
    res->size += 2u;

    return MB_OK;
}

static enum mbstatus_e read_diagnostic_reg(
    const struct mbinst_s *inst,
    const uint8_t *req,
    size_t req_len,
    struct mbpdu_buf_s *res)
{
    if (req_len != 5u) return MB_ILLEGAL_DATA_VAL;
    if (betou16(req+3u) != 0u) return MB_ILLEGAL_DATA_VAL;

    if (inst->serial.read_diagnostics_cb!=NULL) {
        u16tobe(inst->serial.read_diagnostics_cb(), res->p+3u);
    } else {
        u16tobe(0u, res->p+3u);
    }
    res->size += 2u;

    return MB_OK;
}

static enum mbstatus_e change_ascii_delimiter(
    struct mbinst_s *inst,
    const uint8_t *req,
    size_t req_len,
    struct mbpdu_buf_s *res)
{
    if (req_len != 5u) return MB_ILLEGAL_DATA_VAL;
    if (req[3] > 127u) return MB_ILLEGAL_DATA_VAL;
    if (req[4] != 0u) return MB_ILLEGAL_DATA_VAL;

    inst->state.ascii_delimiter = req[3];

    res->p[3] = req[3];
    res->p[4] = 0u;
    res->size += 2u;

    return MB_OK;
}

static enum mbstatus_e force_listen_only(
    struct mbinst_s *inst,
    const uint8_t *req,
    size_t req_len)
{
    if (req_len != 5u) return MB_ILLEGAL_DATA_VAL;
    if (betou16(req+3u) != 0u) return MB_ILLEGAL_DATA_VAL;

    inst->state.is_listen_only = 1;
    mb_add_comm_event(inst, MB_COMM_EVENT_ENTERED_LISTEN_ONLY);

    return MB_OK;
}

static enum mbstatus_e clear_counts_n_diag_reg(
    struct mbinst_s *inst,
    const uint8_t *req,
    size_t req_len,
    struct mbpdu_buf_s *res)
{
    if (req_len != 5u) return MB_ILLEGAL_DATA_VAL;
    if (betou16(req+3u) != 0u) return MB_ILLEGAL_DATA_VAL;

    reset_comm_counters(inst);
    if (inst->serial.reset_diagnostics_cb!=NULL) {
        inst->serial.reset_diagnostics_cb();
    }

    res->p[3] = 0u;
    res->p[4] = 0u;
    res->size += 2u;

    return MB_OK;
}

static enum mbstatus_e read_counter(
    uint16_t counter_value,
    const uint8_t *req,
    size_t req_len,
    struct mbpdu_buf_s *res)
{
    if (req_len != 5u) return MB_ILLEGAL_DATA_VAL;
    if (betou16(req+3u) != 0u) return MB_ILLEGAL_DATA_VAL;

    u16tobe(counter_value, res->p+3u);
    res->size += 2u;

    return MB_OK;
}

static enum mbstatus_e clr_overrun(
    struct mbinst_s *inst,
    const uint8_t *req,
    size_t req_len,
    struct mbpdu_buf_s *res)
{
    if (req_len != 5u) return MB_ILLEGAL_DATA_VAL;
    if (betou16(req+3u) != 0u) return MB_ILLEGAL_DATA_VAL;

    inst->state.bus_char_overrun_counter = 0u;

    res->p[3] = 0u;
    res->p[4] = 0u;
    res->size += 2u;

    return MB_OK;
}

extern enum mbstatus_e mbfn_diag(
    struct mbinst_s *inst,
    const uint8_t *req,
    size_t req_len,
    struct mbpdu_buf_s *res)
{
    if ((inst==NULL) || (req==NULL) || (res==NULL)) return MB_DEV_FAIL;

    if (req_len < 3u) return MB_ILLEGAL_DATA_VAL;

    res->p[0] = req[0];
    res->p[1] = req[1];
    res->p[2] = req[2];
    res->size = 3u;

    switch (betou16(req+1u)) {
    case MBFC_DIAG_LOOPBACK: return loopback(req, req_len, res);
    case MBFC_DIAG_RESTART_COMMS_OPT: return restart_comms_opt(inst, req, req_len, res);
    case MBFC_DIAG_REG: return read_diagnostic_reg(inst, req, req_len, res);
    case MBFC_DIAG_ASCII_DELIM: return change_ascii_delimiter(inst, req, req_len, res);
    case MBFC_DIAG_FORCE_LISTEN: return force_listen_only(inst, req, req_len);
    case MBFC_DIAG_CLR_CNTS_N_DIAG_REG: return clear_counts_n_diag_reg(inst, req, req_len, res);
    case MBFC_DIAG_BUS_MSG_COUNT: return read_counter(inst->state.bus_msg_counter, req, req_len, res);
    case MBFC_DIAG_BUS_COMM_ERR_COUNT: return read_counter(inst->state.bus_comm_err_counter, req, req_len, res);
    case MBFC_DIAG_BUS_EXCEPTION_COUNT: return read_counter(inst->state.exception_counter, req, req_len, res);
    case MBFC_DIAG_MSG_COUNT: return read_counter(inst->state.msg_counter, req, req_len, res);
    case MBFC_DIAG_NO_RESP_MSG_COUNT: return read_counter(inst->state.no_resp_counter, req, req_len, res);
    case MBFC_DIAG_NAK_COUNT: return read_counter(inst->state.nak_counter, req, req_len, res);
    case MBFC_DIAG_BUSY_COUNT: return read_counter(inst->state.busy_counter, req, req_len, res);
    case MBFC_DIAG_BUS_OVERRUN_COUNT: return read_counter(inst->state.bus_char_overrun_counter, req, req_len, res);
    case MBFC_DIAG_CLR_OVERRUN: return clr_overrun(inst, req, req_len, res);
    default: return MB_ILLEGAL_FN;
    }
}

extern enum mbstatus_e mbfn_comm_event_counter(
    const struct mbinst_s *inst,
    const uint8_t *req,
    size_t req_len,
    struct mbpdu_buf_s *res)
{
    if ((inst==NULL) || (req==NULL) || (res==NULL)) return MB_DEV_FAIL;
    if (req_len != 1u) return MB_ILLEGAL_DATA_VAL;

    u16tobe(inst->state.status, res->p+1u);
    u16tobe(inst->state.comm_event_counter, res->p+3u);
    res->size = 5u;

    return MB_OK;
}

extern enum mbstatus_e mbfn_comm_event_log(
    const struct mbinst_s *inst,
    const uint8_t *req,
    size_t req_len,
    struct mbpdu_buf_s *res)
{
    int i, ix;

    if ((inst==NULL) || (req==NULL) || (res==NULL)) return MB_DEV_FAIL;
    if (req_len != 1u) return MB_ILLEGAL_DATA_VAL;

    res->p[1] = 6u + (uint8_t)inst->state.event_log_count;
    u16tobe(inst->state.status, res->p+2u);
    u16tobe(inst->state.comm_event_counter, res->p+4u);
    u16tobe(inst->state.bus_msg_counter, res->p+6u);
    res->size = 8u;

    for (i=0; i<inst->state.event_log_count; ++i) {
        ix = (inst->state.event_log_write_pos + MB_COMM_EVENT_LOG_LEN - 1 - i) % MB_COMM_EVENT_LOG_LEN;
        res->p[res->size++] = inst->state.event_log[ix];
    }

    return MB_OK;
}
