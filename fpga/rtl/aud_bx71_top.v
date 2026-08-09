`timescale 1ns/1ps

// BX71 integration shell. The AXI4-Lite slave can be connected to Zynq PS GP0;
// this standalone reference clocks it from the verified U18 50 MHz oscillator.
// External I2S pin locations remain board-specific.
module aud_bx71_top (
    input  wire             pl_clk_50m,
    input  wire             reset_n,
    input  wire             i2s_bclk,
    input  wire             i2s_lrclk,
    input  wire             i2s_adc_data,
    output wire             i2s_dac_data,
    input  wire [5:0]       s_axi_awaddr,
    input  wire             s_axi_awvalid,
    output wire             s_axi_awready,
    input  wire [31:0]      s_axi_wdata,
    input  wire [3:0]       s_axi_wstrb,
    input  wire             s_axi_wvalid,
    output wire             s_axi_wready,
    output wire [1:0]       s_axi_bresp,
    output wire             s_axi_bvalid,
    input  wire             s_axi_bready,
    input  wire [5:0]       s_axi_araddr,
    input  wire             s_axi_arvalid,
    output wire             s_axi_arready,
    output wire [31:0]      s_axi_rdata,
    output wire [1:0]       s_axi_rresp,
    output wire             s_axi_rvalid,
    input  wire             s_axi_rready
);
    reg lrclk_previous;
    reg frame_boundary;
    wire [175:0] audio_param_bus;
    wire ack_toggle_audio;
    (* ASYNC_REG = "TRUE" *) reg ack_sync_1;
    (* ASYNC_REG = "TRUE" *) reg ack_sync_2;
    wire [175:0] control_param_bus;
    wire control_update_toggle;

    aud_axi_lite_regs registers (
        .clk(pl_clk_50m), .reset_n(reset_n),
        .s_axi_awaddr(s_axi_awaddr), .s_axi_awvalid(s_axi_awvalid),
        .s_axi_awready(s_axi_awready), .s_axi_wdata(s_axi_wdata),
        .s_axi_wstrb(s_axi_wstrb), .s_axi_wvalid(s_axi_wvalid),
        .s_axi_wready(s_axi_wready), .s_axi_bresp(s_axi_bresp),
        .s_axi_bvalid(s_axi_bvalid), .s_axi_bready(s_axi_bready),
        .s_axi_araddr(s_axi_araddr), .s_axi_arvalid(s_axi_arvalid),
        .s_axi_arready(s_axi_arready), .s_axi_rdata(s_axi_rdata),
        .s_axi_rresp(s_axi_rresp), .s_axi_rvalid(s_axi_rvalid),
        .s_axi_rready(s_axi_rready), .ack_toggle(ack_sync_2),
        .param_bus(control_param_bus),
        .update_toggle(control_update_toggle));

    always @(posedge i2s_bclk) begin
        if (!reset_n) begin
            lrclk_previous <= 1'b0;
            frame_boundary <= 1'b0;
        end else begin
            frame_boundary <= lrclk_previous && !i2s_lrclk;
            lrclk_previous <= i2s_lrclk;
        end
    end

    aud_param_cdc #(.WIDTH(176)) parameter_crossing (
        .audio_clk(i2s_bclk), .audio_reset_n(reset_n),
        .frame_boundary(frame_boundary),
        .param_bus_async(control_param_bus),
        .update_toggle_async(control_update_toggle),
        .param_bus_audio(audio_param_bus),
        .ack_toggle_audio(ack_toggle_audio));

    always @(posedge pl_clk_50m) begin
        if (!reset_n) begin
            ack_sync_1 <= 1'b0;
            ack_sync_2 <= 1'b0;
        end else begin
            ack_sync_1 <= ack_toggle_audio;
            ack_sync_2 <= ack_sync_1;
        end
    end

    aud_i2s_effects_top effects (
        .reset_n(reset_n), .i2s_bclk(i2s_bclk), .i2s_lrclk(i2s_lrclk),
        .i2s_adc_data(i2s_adc_data), .i2s_dac_data(i2s_dac_data),
        .gain_q23(audio_param_bus[25:0]),
        .dc_r_q23(audio_param_bus[49:26]),
        .phase_inc(audio_param_bus[81:50]),
        .osc_select(audio_param_bus[83:82]),
        .synth_level_q23(audio_param_bus[107:84]),
        .delay_samples(audio_param_bus[121:108]),
        .delay_feedback_q23(audio_param_bus[145:122]),
        .delay_mix_q23(audio_param_bus[169:146]));
endmodule
