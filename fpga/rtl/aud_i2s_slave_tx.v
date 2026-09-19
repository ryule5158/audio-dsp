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
    reg lrclk_sampled;
    reg [5:0] bit_count;
    reg [23:0] active_sample;

    // WM8960 master mode changes LRCLK on BCLK's falling edge.  Sampling it on
    // the rising edge avoids a same-edge hold violation; the following falling
    // edge can then launch the first MSB exactly one I2S bit after the change.
    always @(posedge bclk) begin
        if (!reset_n)
            lrclk_sampled <= 1'b0;
        else
            lrclk_sampled <= lrclk;
    end

    always @(negedge bclk) begin
        if (!reset_n) begin
            lrclk_previous <= 1'b0;
            bit_count <= 6'd0;
            active_sample <= 24'd0;
            serial_data <= 1'b0;
        end else if (lrclk_sampled != lrclk_previous) begin
            lrclk_previous <= lrclk_sampled;
            bit_count <= 6'd1;
            active_sample <= lrclk_sampled ? right_sample : left_sample;
            serial_data <= lrclk_sampled ? right_sample[23] : left_sample[23];
        end else if (bit_count < 6'd24) begin
            serial_data <= active_sample[23 - bit_count];
            bit_count <= bit_count + 1'b1;
        end else begin
            serial_data <= 1'b0;
        end
    end
endmodule
