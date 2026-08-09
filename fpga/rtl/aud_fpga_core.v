`timescale 1ns/1ps

// One-sample-per-clock-capable stereo DSP core. Samples and gains are Q1.23.
module aud_fpga_core (
    input  wire                     clk,
    input  wire                     reset_n,
    input  wire                     sample_valid,
    input  wire signed [23:0]       left_in,
    input  wire signed [23:0]       right_in,
    input  wire signed [25:0]       gain_q23,
    input  wire signed [23:0]       dc_r_q23,
    input  wire        [31:0]       phase_inc,
    input  wire         [1:0]       osc_select,
    input  wire signed [23:0]       synth_level_q23,
    output reg                      sample_out_valid,
    output reg signed [23:0]        left_out,
    output reg signed [23:0]        right_out
);
    reg signed [23:0] left_x1;
    reg signed [23:0] right_x1;
    reg signed [23:0] left_y1;
    reg signed [23:0] right_y1;
    reg        [31:0] phase;
    reg        [22:0] lfsr;

    `include "aud_fixed_math.vh"

    // Q3.23 gain provides approximately -4x..+4x while audio stays Q1.23.
    function signed [23:0] aud_apply_gain_q23;
        input signed [23:0] sample;
        input signed [25:0] gain;
        reg signed [49:0] product;
        reg signed [55:0] scaled;
        begin
            product = sample * gain;
            scaled = product >>> 23;
            aud_apply_gain_q23 = aud_sat24(scaled);
        end
    endfunction

    function signed [23:0] aud_dc_step;
        input signed [23:0] x;
        input signed [23:0] x1;
        input signed [23:0] y1;
        input signed [23:0] r;
        reg signed [24:0] difference;
        reg signed [23:0] decay;
        reg signed [55:0] result;
        begin
            difference = x - x1;
            decay = aud_mul_q23(r, y1);
            result = difference;
            result = result + decay;
            aud_dc_step = aud_sat24(result);
        end
    endfunction

    wire signed [23:0] left_dc = aud_dc_step(left_in, left_x1,
                                             left_y1, dc_r_q23);
    wire signed [23:0] right_dc = aud_dc_step(right_in, right_x1,
                                              right_y1, dc_r_q23);
    wire signed [23:0] left_gain = aud_apply_gain_q23(left_dc, gain_q23);
    wire signed [23:0] right_gain = aud_apply_gain_q23(right_dc, gain_q23);

    wire [23:0] triangle_unsigned = phase[31]
        ? ~phase[30:7] : phase[30:7];
    wire signed [23:0] osc_saw = $signed(phase[31:8] ^ 24'h800000);
    wire signed [23:0] osc_triangle =
        $signed(triangle_unsigned ^ 24'h800000);
    wire signed [23:0] osc_square = phase[31]
        ? 24'sh7fffff : -24'sd8388608;
    wire signed [23:0] osc_noise = {lfsr[22], lfsr};
    reg  signed [23:0] oscillator;

    always @* begin
        case (osc_select)
            2'd0: oscillator = osc_saw;
            2'd1: oscillator = osc_triangle;
            2'd2: oscillator = osc_square;
            default: oscillator = osc_noise;
        endcase
    end

    wire signed [23:0] synth_sample =
        aud_mul_q23(oscillator, synth_level_q23);
    wire signed [23:0] left_mixed = aud_add_sat(left_gain, synth_sample);
    wire signed [23:0] right_mixed = aud_add_sat(right_gain, synth_sample);

    always @(posedge clk) begin
        if (!reset_n) begin
            left_x1 <= 24'sd0;
            right_x1 <= 24'sd0;
            left_y1 <= 24'sd0;
            right_y1 <= 24'sd0;
            phase <= 32'd0;
            lfsr <= 23'h5a17c3;
            sample_out_valid <= 1'b0;
            left_out <= 24'sd0;
            right_out <= 24'sd0;
        end else begin
            sample_out_valid <= 1'b0;
            if (sample_valid) begin
                left_x1 <= left_in;
                right_x1 <= right_in;
                left_y1 <= left_dc;
                right_y1 <= right_dc;
                phase <= phase + phase_inc;
                lfsr <= {lfsr[21:0], lfsr[22] ^ lfsr[17]};
                left_out <= left_mixed;
                right_out <= right_mixed;
                sample_out_valid <= 1'b1;
            end
        end
    end
endmodule
