`timescale 1ns/1ps

// Standard I2S slave transmitter: 24 significant bits in 32-bit slots.
// SD changes on BCLK falling edges for capture by the codec on rising edges.
module aud_i2s_slave_tx (
    input  wire                     bclk,
    input  wire                     reset_n,
    input  wire                     lrclk,
    input  wire signed [23:0]       left_sample,
    input  wire signed [23:0]       right_sample,
    output reg                      serial_data
);
    reg lrclk_previous;
    reg [5:0] bit_count;
    reg [23:0] active_sample;

    always @(negedge bclk) begin
        if (!reset_n) begin
            lrclk_previous <= 1'b0;
            bit_count <= 6'd0;
            active_sample <= 24'd0;
            serial_data <= 1'b0;
        end else if (lrclk != lrclk_previous) begin
            lrclk_previous <= lrclk;
            bit_count <= 6'd0;
            active_sample <= lrclk ? right_sample : left_sample;
            serial_data <= 1'b0; // I2S one-bit delay
        end else if (bit_count < 6'd24) begin
            serial_data <= active_sample[23 - bit_count];
            bit_count <= bit_count + 1'b1;
        end else begin
            serial_data <= 1'b0;
        end
    end
endmodule
