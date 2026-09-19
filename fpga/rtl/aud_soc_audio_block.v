`timescale 1ns/1ps

// RTL module-reference used by the optional Zynq PS block design.  Explicit
// interface metadata lets Vivado 2018.3 infer a standards-compliant AXI4-Lite
// slave without packaging generated IP into the repository.
module aud_soc_audio_block (
    (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 S_AXI_ACLK CLK" *)
    (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME S_AXI_ACLK, ASSOCIATED_BUSIF S_AXI, ASSOCIATED_RESET S_AXI_ARESETN, FREQ_HZ 50000000" *)
    input  wire        S_AXI_ACLK,
    (* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 S_AXI_ARESETN RST" *)
    (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME S_AXI_ARESETN, POLARITY ACTIVE_LOW" *)
    input  wire        S_AXI_ARESETN,

    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI AWADDR" *)
    (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME S_AXI, PROTOCOL AXI4LITE, DATA_WIDTH 32, ADDR_WIDTH 6, FREQ_HZ 50000000, READ_WRITE_MODE READ_WRITE" *)
    input  wire [5:0]  S_AXI_AWADDR,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI AWVALID" *)
    input  wire        S_AXI_AWVALID,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI AWREADY" *)
    output wire        S_AXI_AWREADY,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI WDATA" *)
    input  wire [31:0] S_AXI_WDATA,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI WSTRB" *)
    input  wire [3:0]  S_AXI_WSTRB,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI WVALID" *)
    input  wire        S_AXI_WVALID,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI WREADY" *)
    output wire        S_AXI_WREADY,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI BRESP" *)
    output wire [1:0]  S_AXI_BRESP,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI BVALID" *)
    output wire        S_AXI_BVALID,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI BREADY" *)
    input  wire        S_AXI_BREADY,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI ARADDR" *)
    input  wire [5:0]  S_AXI_ARADDR,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI ARVALID" *)
    input  wire        S_AXI_ARVALID,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI ARREADY" *)
    output wire        S_AXI_ARREADY,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI RDATA" *)
    output wire [31:0] S_AXI_RDATA,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI RRESP" *)
    output wire [1:0]  S_AXI_RRESP,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI RVALID" *)
    output wire        S_AXI_RVALID,
    (* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 S_AXI RREADY" *)
    input  wire        S_AXI_RREADY,

    (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 I2S_BCLK CLK" *)
    (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME I2S_BCLK, FREQ_HZ 3072000" *)
    input  wire        i2s_bclk,
    (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 WM8960_MCLK CLK" *)
    (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME WM8960_MCLK, FREQ_HZ 24000000" *)
    input  wire        wm8960_mclk,
    input  wire        i2s_lrclk,
    input  wire        i2s_adc_data,
    output wire        i2s_dac_data
);
    (* ASYNC_REG = "TRUE" *) reg [1:0] audio_reset_sync = 2'b00;
    (* DONT_TOUCH = "TRUE", MARK_DEBUG = "TRUE" *) reg [23:0] mclk_activity;

    always @(posedge wm8960_mclk or negedge S_AXI_ARESETN) begin
        if (!S_AXI_ARESETN)
            mclk_activity <= 24'd0;
        else
            mclk_activity <= mclk_activity + 1'b1;
    end

    // Synchronize both reset edges into BCLK.  Configuration initializes the
    // pipe low; using only synchronous flops also prevents Vivado from routing
    // an asynchronous reset onto inferred BRAM control pins (REQP-1839).
    always @(posedge i2s_bclk)
        audio_reset_sync <= {audio_reset_sync[0], S_AXI_ARESETN};

    aud_bx71_top audio (
        .pl_clk_50m(S_AXI_ACLK),
        .control_reset_n(S_AXI_ARESETN),
        .audio_reset_n(audio_reset_sync[1]),
        .i2s_bclk(i2s_bclk),
        .i2s_lrclk(i2s_lrclk),
        .i2s_adc_data(i2s_adc_data),
        .i2s_dac_data(i2s_dac_data),
        .s_axi_awaddr(S_AXI_AWADDR),
        .s_axi_awvalid(S_AXI_AWVALID),
        .s_axi_awready(S_AXI_AWREADY),
        .s_axi_wdata(S_AXI_WDATA),
        .s_axi_wstrb(S_AXI_WSTRB),
        .s_axi_wvalid(S_AXI_WVALID),
        .s_axi_wready(S_AXI_WREADY),
        .s_axi_bresp(S_AXI_BRESP),
        .s_axi_bvalid(S_AXI_BVALID),
        .s_axi_bready(S_AXI_BREADY),
        .s_axi_araddr(S_AXI_ARADDR),
        .s_axi_arvalid(S_AXI_ARVALID),
        .s_axi_arready(S_AXI_ARREADY),
        .s_axi_rdata(S_AXI_RDATA),
        .s_axi_rresp(S_AXI_RRESP),
        .s_axi_rvalid(S_AXI_RVALID),
        .s_axi_rready(S_AXI_RREADY));
endmodule
