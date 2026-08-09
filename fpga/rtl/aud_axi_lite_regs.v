`timescale 1ns/1ps

// AXI4-Lite register bank for Zynq PS control. One write address and one write
// data item are buffered independently, as required by AXI4-Lite.
module aud_axi_lite_regs (
    input  wire             clk,
    input  wire             reset_n,
    input  wire [5:0]       s_axi_awaddr,
    input  wire             s_axi_awvalid,
    output wire             s_axi_awready,
    input  wire [31:0]      s_axi_wdata,
    input  wire [3:0]       s_axi_wstrb,
    input  wire             s_axi_wvalid,
    output wire             s_axi_wready,
    output reg  [1:0]       s_axi_bresp,
    output reg              s_axi_bvalid,
    input  wire             s_axi_bready,
    input  wire [5:0]       s_axi_araddr,
    input  wire             s_axi_arvalid,
    output wire             s_axi_arready,
    output reg  [31:0]      s_axi_rdata,
    output reg  [1:0]       s_axi_rresp,
    output reg              s_axi_rvalid,
    input  wire             s_axi_rready,
    input  wire             ack_toggle,
    output wire [175:0]     param_bus,
    output reg              update_toggle
);
    reg aw_pending;
    reg [5:0] awaddr_latched;
    reg w_pending;
    reg [31:0] wdata_latched;
    reg [3:0] wstrb_latched;

    reg signed [25:0] gain_q23;
    reg signed [23:0] dc_r_q23;
    reg [31:0] phase_inc;
    reg [1:0] osc_select;
    reg signed [23:0] synth_level_q23;
    reg [13:0] delay_samples;
    reg signed [23:0] delay_feedback_q23;
    reg signed [23:0] delay_mix_q23;

    wire update_pending = update_toggle != ack_toggle;
    assign s_axi_awready = !aw_pending && !s_axi_bvalid;
    assign s_axi_wready = !w_pending && !s_axi_bvalid;
    assign s_axi_arready = !s_axi_rvalid;
    assign param_bus = {6'd0, delay_mix_q23, delay_feedback_q23,
                        delay_samples, synth_level_q23, osc_select,
                        phase_inc, dc_r_q23, gain_q23};

    always @(posedge clk) begin
        if (!reset_n) begin
            aw_pending <= 1'b0;
            awaddr_latched <= 6'd0;
            w_pending <= 1'b0;
            wdata_latched <= 32'd0;
            wstrb_latched <= 4'd0;
            s_axi_bresp <= 2'b00;
            s_axi_bvalid <= 1'b0;
            gain_q23 <= 26'sd8388608;
            dc_r_q23 <= 24'sh7f5c29;
            phase_inc <= 32'd0;
            osc_select <= 2'd0;
            synth_level_q23 <= 24'sd0;
            delay_samples <= 14'd1;
            delay_feedback_q23 <= 24'sd0;
            delay_mix_q23 <= 24'sd0;
            update_toggle <= 1'b0;
        end else begin
            if (s_axi_awready && s_axi_awvalid) begin
                awaddr_latched <= s_axi_awaddr;
                aw_pending <= 1'b1;
            end
            if (s_axi_wready && s_axi_wvalid) begin
                wdata_latched <= s_axi_wdata;
                wstrb_latched <= s_axi_wstrb;
                w_pending <= 1'b1;
            end
            if (s_axi_bvalid && s_axi_bready)
                s_axi_bvalid <= 1'b0;

            if (aw_pending && w_pending && !s_axi_bvalid) begin
                aw_pending <= 1'b0;
                w_pending <= 1'b0;
                s_axi_bvalid <= 1'b1;
                s_axi_bresp <= 2'b00;
                if (|awaddr_latched[1:0]) begin
                    s_axi_bresp <= 2'b10;
                end else if (update_pending) begin
                    // Preserve the stable multi-bit CDC bus until audio ACK.
                    s_axi_bresp <= 2'b10;
                end else begin
                    case (awaddr_latched[5:2])
                        4'h0: begin
                            if (wstrb_latched[0] && wdata_latched[0])
                                update_toggle <= ~update_toggle;
                        end
                        4'h1: begin
                            if (wstrb_latched[0]) gain_q23[7:0] <= wdata_latched[7:0];
                            if (wstrb_latched[1]) gain_q23[15:8] <= wdata_latched[15:8];
                            if (wstrb_latched[2]) gain_q23[23:16] <= wdata_latched[23:16];
                            if (wstrb_latched[3]) gain_q23[25:24] <= wdata_latched[25:24];
                        end
                        4'h2: begin
                            if (wstrb_latched[0]) dc_r_q23[7:0] <= wdata_latched[7:0];
                            if (wstrb_latched[1]) dc_r_q23[15:8] <= wdata_latched[15:8];
                            if (wstrb_latched[2]) dc_r_q23[23:16] <= wdata_latched[23:16];
                        end
                        4'h3: begin
                            if (wstrb_latched[0]) phase_inc[7:0] <= wdata_latched[7:0];
                            if (wstrb_latched[1]) phase_inc[15:8] <= wdata_latched[15:8];
                            if (wstrb_latched[2]) phase_inc[23:16] <= wdata_latched[23:16];
                            if (wstrb_latched[3]) phase_inc[31:24] <= wdata_latched[31:24];
                        end
                        4'h4: if (wstrb_latched[0])
                            osc_select <= wdata_latched[1:0];
                        4'h5: begin
                            if (wstrb_latched[0]) synth_level_q23[7:0] <= wdata_latched[7:0];
                            if (wstrb_latched[1]) synth_level_q23[15:8] <= wdata_latched[15:8];
                            if (wstrb_latched[2]) synth_level_q23[23:16] <= wdata_latched[23:16];
                        end
                        4'h6: begin
                            if (wstrb_latched[0]) delay_samples[7:0] <= wdata_latched[7:0];
                            if (wstrb_latched[1]) delay_samples[13:8] <= wdata_latched[13:8];
                        end
                        4'h7: begin
                            if (wstrb_latched[0]) delay_feedback_q23[7:0] <= wdata_latched[7:0];
                            if (wstrb_latched[1]) delay_feedback_q23[15:8] <= wdata_latched[15:8];
                            if (wstrb_latched[2]) delay_feedback_q23[23:16] <= wdata_latched[23:16];
                        end
                        4'h8: begin
                            if (wstrb_latched[0]) delay_mix_q23[7:0] <= wdata_latched[7:0];
                            if (wstrb_latched[1]) delay_mix_q23[15:8] <= wdata_latched[15:8];
                            if (wstrb_latched[2]) delay_mix_q23[23:16] <= wdata_latched[23:16];
                        end
                        default: s_axi_bresp <= 2'b10;
                    endcase
                end
            end
        end
    end

    always @(posedge clk) begin
        if (!reset_n) begin
            s_axi_rdata <= 32'd0;
            s_axi_rresp <= 2'b00;
            s_axi_rvalid <= 1'b0;
        end else begin
            if (s_axi_rvalid && s_axi_rready)
                s_axi_rvalid <= 1'b0;
            if (s_axi_arready && s_axi_arvalid) begin
                s_axi_rvalid <= 1'b1;
                s_axi_rresp <= 2'b00;
                if (|s_axi_araddr[1:0]) begin
                    s_axi_rdata <= 32'd0;
                    s_axi_rresp <= 2'b10;
                end else case (s_axi_araddr[5:2])
                    4'h0: s_axi_rdata <= {29'd0, update_pending,
                                           ack_toggle, update_toggle};
                    4'h1: s_axi_rdata <= {{6{gain_q23[25]}}, gain_q23};
                    4'h2: s_axi_rdata <= {{8{dc_r_q23[23]}}, dc_r_q23};
                    4'h3: s_axi_rdata <= phase_inc;
                    4'h4: s_axi_rdata <= {30'd0, osc_select};
                    4'h5: s_axi_rdata <= {{8{synth_level_q23[23]}}, synth_level_q23};
                    4'h6: s_axi_rdata <= {18'd0, delay_samples};
                    4'h7: s_axi_rdata <= {{8{delay_feedback_q23[23]}}, delay_feedback_q23};
                    4'h8: s_axi_rdata <= {{8{delay_mix_q23[23]}}, delay_mix_q23};
                    4'h9: s_axi_rdata <= 32'h41554431; // "AUD1"
                    default: begin
                        s_axi_rdata <= 32'd0;
                        s_axi_rresp <= 2'b10;
                    end
                endcase
            end
        end
    end
endmodule
