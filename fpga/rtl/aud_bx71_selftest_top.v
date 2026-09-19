`timescale 1ns/1ps

// Default BX71 hardware profile.
//
// This top level needs only the three board resources that are confirmed by
// the BX71 manual, pin spreadsheet, and vendor LED example: U18 (50 MHz PL
// clock), T19 (active-low PL key), and T10 (active-high PL LED).  It runs the
// real NCO/DC/gain/delay datapath from an internal 48 kHz sample source, so the
// generated bitstream can be loaded before an external audio codec is chosen.
module aud_bx71_selftest_top #(
    parameter integer CLOCK_HZ = 50000000,
    parameter integer SAMPLE_RATE_HZ = 48000,
    parameter integer DELAY_ADDR_WIDTH = 14,
    parameter integer DELAY_DEPTH = 16384,
    parameter integer DELAY_SAMPLES = 2400,
    parameter integer HEARTBEAT_BIT = 25,
    parameter integer ERROR_BLINK_BIT = 22
) (
    input  wire pl_clk_50m,
    input  wire reset_n,
    output wire fpga_led0
);
    (* ASYNC_REG = "TRUE" *) reg [1:0] reset_sync = 2'b00;
    reg [32:0] sample_accumulator;
    reg sample_valid;
    reg signed [23:0] stimulus_left;
    reg signed [23:0] stimulus_right;

    wire [32:0] accumulator_sum = sample_accumulator + SAMPLE_RATE_HZ;
    wire core_valid;
    wire signed [23:0] core_left;
    wire signed [23:0] core_right;
    wire delay_ready;
    wire delay_valid;
    wire signed [23:0] delay_left;
    wire signed [23:0] delay_right;

    (* MARK_DEBUG = "TRUE" *) reg error_latched;
    (* MARK_DEBUG = "TRUE" *) reg activity_latched;
    (* MARK_DEBUG = "TRUE" *) reg [31:0] accepted_count;
    (* MARK_DEBUG = "TRUE" *) reg [31:0] output_count;
    (* MARK_DEBUG = "TRUE" *) reg [31:0] health_signature;
    reg [31:0] heartbeat_counter;
    wire internal_reset_n = reset_sync[1];

    // T19 is asynchronous to the 50 MHz oscillator.  Synchronize both edges;
    // the FPGA configuration value holds the design reset until two sampled
    // high levels have arrived, without an async-reset flop feeding BRAM.
    always @(posedge pl_clk_50m)
        reset_sync <= {reset_sync[0], reset_n};

    // Fractional accumulator gives exactly 48,000 sample enables per second
    // on average without creating a derived clock.
    always @(posedge pl_clk_50m) begin
        if (!internal_reset_n) begin
            sample_accumulator <= 33'd0;
            sample_valid <= 1'b0;
            stimulus_left <= -24'sd2097152;
            stimulus_right <= 24'sd2097152;
        end else begin
            sample_valid <= 1'b0;
            if (accumulator_sum >= CLOCK_HZ) begin
                sample_accumulator <= accumulator_sum - CLOCK_HZ;
                sample_valid <= 1'b1;
                stimulus_left <= stimulus_left + 24'sd32749;
                stimulus_right <= stimulus_right - 24'sd24593;
            end else begin
                sample_accumulator <= accumulator_sum;
            end
        end
    end

    (* DONT_TOUCH = "TRUE" *) aud_fpga_core core (
        .clk(pl_clk_50m),
        .reset_n(internal_reset_n),
        .sample_valid(sample_valid),
        .left_in(stimulus_left),
        .right_in(stimulus_right),
        .gain_q23(26'sd8388608),             // 1.0 in Q3.23
        .dc_r_q23(24'sh7f5c29),              // approximately 0.995
        .phase_inc(32'd39370534),             // 440 Hz at 48 kHz
        .osc_select(2'd1),                    // triangle
        .synth_level_q23(24'sd1048576),       // 0.125 in Q1.23
        .sample_out_valid(core_valid),
        .left_out(core_left),
        .right_out(core_right));

    (* DONT_TOUCH = "TRUE" *) aud_delay_stereo_q23 #(
        .ADDR_WIDTH(DELAY_ADDR_WIDTH),
        .DEPTH(DELAY_DEPTH)) delay (
        .clk(pl_clk_50m),
        .reset_n(internal_reset_n),
        .sample_valid(core_valid),
        .left_in(core_left),
        .right_in(core_right),
        .delay_samples(DELAY_SAMPLES[DELAY_ADDR_WIDTH-1:0]),
        .feedback_q23(24'sd3145728),          // 0.375
        .mix_q23(24'sd2097152),               // 0.25
        .ready(delay_ready),
        .sample_out_valid(delay_valid),
        .left_out(delay_left),
        .right_out(delay_right));

    always @(posedge pl_clk_50m) begin
        if (!internal_reset_n) begin
            error_latched <= 1'b0;
            activity_latched <= 1'b0;
            accepted_count <= 32'd0;
            output_count <= 32'd0;
            health_signature <= 32'h1aceb00c;
            heartbeat_counter <= 32'd0;
        end else begin
            heartbeat_counter <= heartbeat_counter + 1'b1;
            if (sample_valid) begin
                accepted_count <= accepted_count + 1'b1;
                // The preceding sample has more than 1000 clocks to finish at
                // 50 MHz/48 kHz.  A mismatch therefore catches a stuck core.
                if (accepted_count > 2 && output_count != accepted_count)
                    error_latched <= 1'b1;
            end
            if (core_valid && !delay_ready)
                error_latched <= 1'b1;
            if (delay_valid) begin
                output_count <= output_count + 1'b1;
                activity_latched <= activity_latched |
                    (|delay_left) | (|delay_right);
                health_signature <= {health_signature[30:0],
                    health_signature[31] ^ health_signature[21] ^
                    delay_left[0] ^ delay_left[7] ^
                    delay_right[3] ^ delay_right[19]};
            end
        end
    end

    // Slow heartbeat means the complete datapath is active.  A fast blink
    // means the sample pipeline watchdog has latched an error.
    assign fpga_led0 = error_latched ? heartbeat_counter[ERROR_BLINK_BIT] :
                       activity_latched ? heartbeat_counter[HEARTBEAT_BIT] :
                       1'b0;
endmodule
