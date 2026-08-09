`timescale 1ns/1ps

// Stereo BRAM delay. DEPTH must be a power of two. One frame takes 3 clocks.
module aud_delay_stereo_q23 #(
    parameter ADDR_WIDTH = 14,
    parameter DEPTH = 16384
) (
    input  wire                     clk,
    input  wire                     reset_n,
    input  wire                     sample_valid,
    input  wire signed [23:0]       left_in,
    input  wire signed [23:0]       right_in,
    input  wire        [ADDR_WIDTH-1:0] delay_samples,
    input  wire signed [23:0]       feedback_q23,
    input  wire signed [23:0]       mix_q23,
    output wire                     ready,
    output reg                      sample_out_valid,
    output reg signed [23:0]        left_out,
    output reg signed [23:0]        right_out
);
    localparam STATE_IDLE = 2'd0;
    localparam STATE_READ = 2'd1;
    localparam STATE_MIX  = 2'd2;

    (* ram_style = "block" *) reg signed [23:0] memory_left [0:DEPTH-1];
    (* ram_style = "block" *) reg signed [23:0] memory_right[0:DEPTH-1];
    reg [1:0] state;
    reg [ADDR_WIDTH-1:0] write_address;
    reg [ADDR_WIDTH-1:0] read_address;
    reg [ADDR_WIDTH:0] written_samples;
    reg read_has_history;
    reg signed [23:0] dry_left;
    reg signed [23:0] dry_right;
    reg signed [23:0] memory_read_left;
    reg signed [23:0] memory_read_right;
    wire [ADDR_WIDTH-1:0] bounded_delay =
        (delay_samples == 0) ? {{(ADDR_WIDTH-1){1'b0}}, 1'b1} :
        delay_samples;

    `include "aud_fixed_math.vh"

    assign ready = (state == STATE_IDLE);

    wire signed [23:0] wet_left = read_has_history
        ? memory_read_left : 24'sd0;
    wire signed [23:0] wet_right = read_has_history
        ? memory_read_right : 24'sd0;
    wire memory_write_enable = reset_n && (state == STATE_MIX);
    wire signed [23:0] memory_write_left = aud_add_sat(
        dry_left, aud_mul_q23(wet_left, feedback_q23));
    wire signed [23:0] memory_write_right = aud_add_sat(
        dry_right, aud_mul_q23(wet_right, feedback_q23));

    // Vivado 2018.3 simple-dual-port synchronous RAM inference template.
    always @(posedge clk) begin
        memory_read_left <= memory_left[read_address];
        if (memory_write_enable)
            memory_left[write_address] <= memory_write_left;
    end

    always @(posedge clk) begin
        memory_read_right <= memory_right[read_address];
        if (memory_write_enable)
            memory_right[write_address] <= memory_write_right;
    end

    always @(posedge clk) begin
        if (!reset_n) begin
            state <= STATE_IDLE;
            write_address <= {ADDR_WIDTH{1'b0}};
            read_address <= {ADDR_WIDTH{1'b0}};
            written_samples <= {(ADDR_WIDTH+1){1'b0}};
            read_has_history <= 1'b0;
            dry_left <= 24'sd0;
            dry_right <= 24'sd0;
            left_out <= 24'sd0;
            right_out <= 24'sd0;
            sample_out_valid <= 1'b0;
        end else begin
            sample_out_valid <= 1'b0;
            case (state)
                STATE_IDLE: begin
                    if (sample_valid) begin
                        dry_left <= left_in;
                        dry_right <= right_in;
                        read_address <= write_address - bounded_delay;
                        read_has_history <=
                            written_samples >= {1'b0, bounded_delay};
                        state <= STATE_READ;
                    end
                end
                STATE_READ: begin
                    state <= STATE_MIX;
                end
                STATE_MIX: begin
                    left_out <= aud_mix_q23(dry_left, wet_left, mix_q23);
                    right_out <= aud_mix_q23(dry_right, wet_right, mix_q23);
                    write_address <= write_address + 1'b1;
                    if (written_samples < DEPTH)
                        written_samples <= written_samples + 1'b1;
                    sample_out_valid <= 1'b1;
                    state <= STATE_IDLE;
                end
                default: state <= STATE_IDLE;
            endcase
        end
    end
endmodule
