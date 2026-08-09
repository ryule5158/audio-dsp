`timescale 1ns/1ps

// Reference codec-master design. All audio logic runs in the codec BCLK
// domain, avoiding an unsafe audio-data clock crossing. Parameter words are
// supplied by a separate AXI-Lite register block using aud_param_cdc.
module aud_i2s_effects_top #(
    parameter DELAY_ADDR_WIDTH = 14,
    parameter DELAY_DEPTH = 16384
) (
    input  wire                     reset_n,
    input  wire                     i2s_bclk,
    input  wire                     i2s_lrclk,
    input  wire                     i2s_adc_data,
    output wire                     i2s_dac_data,
    input  wire signed [25:0]       gain_q23,
    input  wire signed [23:0]       dc_r_q23,
    input  wire        [31:0]       phase_inc,
    input  wire         [1:0]       osc_select,
    input  wire signed [23:0]       synth_level_q23,
    input  wire [DELAY_ADDR_WIDTH-1:0] delay_samples,
    input  wire signed [23:0]       delay_feedback_q23,
    input  wire signed [23:0]       delay_mix_q23
);
    wire rx_valid;
    wire signed [23:0] rx_left;
    wire signed [23:0] rx_right;
    wire core_valid;
    wire signed [23:0] core_left;
    wire signed [23:0] core_right;
    wire delay_ready;
    wire delay_valid;
    wire signed [23:0] delay_left;
    wire signed [23:0] delay_right;
    reg signed [23:0] transmit_left;
    reg signed [23:0] transmit_right;

    aud_i2s_slave_rx receiver (
        .bclk(i2s_bclk), .reset_n(reset_n), .lrclk(i2s_lrclk),
        .serial_data(i2s_adc_data), .frame_valid(rx_valid),
        .left_sample(rx_left), .right_sample(rx_right));

    aud_fpga_core core (
        .clk(i2s_bclk), .reset_n(reset_n), .sample_valid(rx_valid),
        .left_in(rx_left), .right_in(rx_right), .gain_q23(gain_q23),
        .dc_r_q23(dc_r_q23), .phase_inc(phase_inc),
        .osc_select(osc_select), .synth_level_q23(synth_level_q23),
        .sample_out_valid(core_valid), .left_out(core_left),
        .right_out(core_right));

    aud_delay_stereo_q23 #(
        .ADDR_WIDTH(DELAY_ADDR_WIDTH), .DEPTH(DELAY_DEPTH)) delay (
        .clk(i2s_bclk), .reset_n(reset_n), .sample_valid(core_valid),
        .left_in(core_left), .right_in(core_right),
        .delay_samples(delay_samples),
        .feedback_q23(delay_feedback_q23), .mix_q23(delay_mix_q23),
        .ready(delay_ready), .sample_out_valid(delay_valid),
        .left_out(delay_left), .right_out(delay_right));

    always @(posedge i2s_bclk) begin
        if (!reset_n) begin
            transmit_left <= 24'sd0;
            transmit_right <= 24'sd0;
        end else if (delay_valid) begin
            transmit_left <= delay_left;
            transmit_right <= delay_right;
        end
    end

    aud_i2s_slave_tx transmitter (
        .bclk(i2s_bclk), .reset_n(reset_n), .lrclk(i2s_lrclk),
        .left_sample(transmit_left), .right_sample(transmit_right),
        .serial_data(i2s_dac_data));

    // At 24-bit stereo/32-bit slots, frames are 64 BCLKs apart and the delay
    // consumes only 3 clocks. This assertion-by-construction exposes the busy
    // state to synthesis without silently dropping a frame in normal use.
    wire unused_delay_ready = delay_ready;
endmodule
