`timescale 1ns/1ps

// Standard I2S slave receiver: 24 significant bits in 32-bit channel slots.
// LRCLK low is left, high is right. SD is sampled on BCLK rising edges.
module aud_i2s_slave_rx (
    input  wire                     bclk,
    input  wire                     reset_n,
    input  wire                     lrclk,
    input  wire                     serial_data,
    output reg                      frame_valid,
    output reg signed [23:0]        left_sample,
    output reg signed [23:0]        right_sample
);
    reg lrclk_previous;
    reg [5:0] bit_count;
    reg [23:0] shift_register;
    wire [23:0] completed_sample = {shift_register[22:0], serial_data};

    always @(posedge bclk) begin
        if (!reset_n) begin
            lrclk_previous <= 1'b0;
            bit_count <= 6'd0;
            shift_register <= 24'd0;
            frame_valid <= 1'b0;
            left_sample <= 24'sd0;
            right_sample <= 24'sd0;
        end else begin
            frame_valid <= 1'b0;
            if (lrclk != lrclk_previous) begin
                // I2S inserts one BCLK delay after each LRCLK transition.
                lrclk_previous <= lrclk;
                bit_count <= 6'd0;
                shift_register <= 24'd0;
            end else if (bit_count < 6'd24) begin
                shift_register <= completed_sample;
                bit_count <= bit_count + 1'b1;
                if (bit_count == 6'd23) begin
                    if (!lrclk) begin
                        left_sample <= completed_sample;
                    end else begin
                        right_sample <= completed_sample;
                        frame_valid <= 1'b1;
                    end
                end
            end
        end
    end
endmodule
