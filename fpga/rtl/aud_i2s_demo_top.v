`timescale 1ns/1ps

// External-codec profile with fixed safe DSP parameters.
// This profile intentionally has no PACKAGE_PIN assignments for I2S: copy the
// external board profile and fill it only after the codec and connector are
// selected.  The codec is the BCLK/LRCLK master.
module aud_i2s_demo_top (
    input  wire pl_clk_50m,
    input  wire reset_n,
    input  wire wm8960_mclk,
    input  wire i2s_bclk,
    input  wire i2s_lrclk,
    input  wire i2s_adc_data,
    output wire i2s_dac_data,
    output wire fpga_led0
);
    (* ASYNC_REG = "TRUE" *) reg [1:0] pl_reset_sync = 2'b00;
    (* ASYNC_REG = "TRUE" *) reg [1:0] mclk_reset_sync = 2'b00;
    (* ASYNC_REG = "TRUE" *) reg [1:0] bclk_reset_sync = 2'b00;
    reg [25:0] heartbeat;
    reg mclk_toggle;
    reg bclk_toggle;
    (* ASYNC_REG = "TRUE" *) reg [2:0] mclk_sync;
    (* ASYNC_REG = "TRUE" *) reg [2:0] bclk_sync;
    reg mclk_seen;
    reg bclk_seen;
    wire pl_reset_n = pl_reset_sync[1];
    wire mclk_reset_n = mclk_reset_sync[1];
    wire bclk_reset_n = bclk_reset_sync[1];

    always @(posedge pl_clk_50m)
        pl_reset_sync <= {pl_reset_sync[0], reset_n};

    always @(posedge wm8960_mclk)
        mclk_reset_sync <= {mclk_reset_sync[0], reset_n};

    always @(posedge i2s_bclk)
        bclk_reset_sync <= {bclk_reset_sync[0], reset_n};

    always @(posedge wm8960_mclk) begin
        if (!mclk_reset_n)
            mclk_toggle <= 1'b0;
        else
            mclk_toggle <= ~mclk_toggle;
    end

    always @(posedge i2s_bclk) begin
        if (!bclk_reset_n)
            bclk_toggle <= 1'b0;
        else
            bclk_toggle <= ~bclk_toggle;
    end

    always @(posedge pl_clk_50m) begin
        if (!pl_reset_n) begin
            heartbeat <= 26'd0;
            mclk_sync <= 3'b000;
            bclk_sync <= 3'b000;
            mclk_seen <= 1'b0;
            bclk_seen <= 1'b0;
        end else begin
            heartbeat <= heartbeat + 1'b1;
            mclk_sync <= {mclk_sync[1:0], mclk_toggle};
            bclk_sync <= {bclk_sync[1:0], bclk_toggle};
            if (mclk_sync[2] != mclk_sync[1])
                mclk_seen <= 1'b1;
            if (bclk_sync[2] != bclk_sync[1])
                bclk_seen <= 1'b1;
        end
    end

    assign fpga_led0 = (mclk_seen && bclk_seen) ? heartbeat[25] : heartbeat[22];

    aud_i2s_effects_top effects (
        .reset_n(bclk_reset_n),
        .i2s_bclk(i2s_bclk),
        .i2s_lrclk(i2s_lrclk),
        .i2s_adc_data(i2s_adc_data),
        .i2s_dac_data(i2s_dac_data),
        .gain_q23(26'sd8388608),
        .dc_r_q23(24'sh7f5c29),
        .phase_inc(32'd0),
        .osc_select(2'd0),
        .synth_level_q23(24'sd0),
        .delay_samples(14'd1),
        .delay_feedback_q23(24'sd0),
        .delay_mix_q23(24'sd0));
endmodule
