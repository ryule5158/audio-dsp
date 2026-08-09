`timescale 1ns/1ps

// Toggle-handshake transfer from a PS/AXI clock domain to the audio BCLK
// domain. The source must hold param_bus stable until ack_toggle returns.
module aud_param_cdc #(
    parameter WIDTH = 128
) (
    input  wire                 audio_clk,
    input  wire                 audio_reset_n,
    input  wire                 frame_boundary,
    input  wire [WIDTH-1:0]     param_bus_async,
    input  wire                 update_toggle_async,
    output reg  [WIDTH-1:0]     param_bus_audio,
    output reg                  ack_toggle_audio
);
    (* ASYNC_REG = "TRUE" *) reg update_sync_1;
    (* ASYNC_REG = "TRUE" *) reg update_sync_2;

    always @(posedge audio_clk) begin
        if (!audio_reset_n) begin
            update_sync_1 <= 1'b0;
            update_sync_2 <= 1'b0;
            param_bus_audio <= {WIDTH{1'b0}};
            ack_toggle_audio <= 1'b0;
        end else begin
            update_sync_1 <= update_toggle_async;
            update_sync_2 <= update_sync_1;
            if (frame_boundary && update_sync_2 != ack_toggle_audio) begin
                param_bus_audio <= param_bus_async;
                ack_toggle_audio <= update_sync_2;
            end
        end
    end
endmodule
