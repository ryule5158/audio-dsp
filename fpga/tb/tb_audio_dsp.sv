`timescale 1ns/1ps

module tb_audio_dsp;
    reg clk = 1'b0;
    reg reset_n = 1'b0;
    always #5 clk = ~clk;

    reg core_valid = 1'b0;
    reg signed [23:0] core_left_in = 0;
    reg signed [23:0] core_right_in = 0;
    reg signed [25:0] gain_q23 = 26'sd8388608;
    reg signed [23:0] dc_r_q23 = 24'sd0;
    reg [31:0] phase_inc = 32'd0;
    reg [1:0] osc_select = 2'd0;
    reg signed [23:0] synth_level_q23 = 24'sd0;
    wire core_out_valid;
    wire signed [23:0] core_left_out;
    wire signed [23:0] core_right_out;

    aud_fpga_core core (
        .clk(clk), .reset_n(reset_n), .sample_valid(core_valid),
        .left_in(core_left_in), .right_in(core_right_in),
        .gain_q23(gain_q23), .dc_r_q23(dc_r_q23),
        .phase_inc(phase_inc), .osc_select(osc_select),
        .synth_level_q23(synth_level_q23),
        .sample_out_valid(core_out_valid), .left_out(core_left_out),
        .right_out(core_right_out));

    reg delay_valid = 1'b0;
    reg signed [23:0] delay_left_in = 0;
    reg signed [23:0] delay_right_in = 0;
    reg [2:0] delay_samples = 3'd2;
    reg signed [23:0] delay_feedback = 24'sd0;
    reg signed [23:0] delay_mix = 24'sh7fffff;
    wire delay_ready;
    wire delay_out_valid;
    wire signed [23:0] delay_left_out;
    wire signed [23:0] delay_right_out;

    aud_delay_stereo_q23 #(.ADDR_WIDTH(3), .DEPTH(8)) delay (
        .clk(clk), .reset_n(reset_n), .sample_valid(delay_valid),
        .left_in(delay_left_in), .right_in(delay_right_in),
        .delay_samples(delay_samples), .feedback_q23(delay_feedback),
        .mix_q23(delay_mix), .ready(delay_ready),
        .sample_out_valid(delay_out_valid), .left_out(delay_left_out),
        .right_out(delay_right_out));

    // Idle high so the first transmitted left channel has a real high-to-low
    // frame transition; I2S has no separate frame-start signal.
    reg i2s_lrclk = 1'b1;
    reg i2s_data = 1'b0;
    wire i2s_frame_valid;
    wire signed [23:0] i2s_left;
    wire signed [23:0] i2s_right;
    reg i2s_frame_seen = 1'b0;
    reg signed [23:0] captured_i2s_left = 0;
    reg signed [23:0] captured_i2s_right = 0;
    integer failure_count = 0;
    aud_i2s_slave_rx receiver (
        .bclk(clk), .reset_n(reset_n), .lrclk(i2s_lrclk),
        .serial_data(i2s_data), .frame_valid(i2s_frame_valid),
        .left_sample(i2s_left), .right_sample(i2s_right));

    reg [5:0] axi_awaddr = 0;
    reg axi_awvalid = 0;
    wire axi_awready;
    reg [31:0] axi_wdata = 0;
    reg [3:0] axi_wstrb = 4'hf;
    reg axi_wvalid = 0;
    wire axi_wready;
    wire [1:0] axi_bresp;
    wire axi_bvalid;
    reg axi_bready = 1;
    reg [5:0] axi_araddr = 0;
    reg axi_arvalid = 0;
    wire axi_arready;
    wire [31:0] axi_rdata;
    wire [1:0] axi_rresp;
    wire axi_rvalid;
    reg axi_rready = 1;
    reg axi_ack_toggle = 0;
    wire [175:0] axi_param_bus;
    wire axi_update_toggle;

    aud_axi_lite_regs axi_registers (
        .clk(clk), .reset_n(reset_n), .s_axi_awaddr(axi_awaddr),
        .s_axi_awvalid(axi_awvalid), .s_axi_awready(axi_awready),
        .s_axi_wdata(axi_wdata), .s_axi_wstrb(axi_wstrb),
        .s_axi_wvalid(axi_wvalid), .s_axi_wready(axi_wready),
        .s_axi_bresp(axi_bresp), .s_axi_bvalid(axi_bvalid),
        .s_axi_bready(axi_bready), .s_axi_araddr(axi_araddr),
        .s_axi_arvalid(axi_arvalid), .s_axi_arready(axi_arready),
        .s_axi_rdata(axi_rdata), .s_axi_rresp(axi_rresp),
        .s_axi_rvalid(axi_rvalid), .s_axi_rready(axi_rready),
        .ack_toggle(axi_ack_toggle), .param_bus(axi_param_bus),
        .update_toggle(axi_update_toggle));

    always @(posedge clk) begin
        if (i2s_frame_valid) begin
            i2s_frame_seen <= 1'b1;
            captured_i2s_left <= i2s_left;
            captured_i2s_right <= i2s_right;
        end
    end

    task fail;
        input [8*80-1:0] message;
        begin
            $display("FPGA_RTL_TEST_FAIL: %0s", message);
            failure_count = failure_count + 1;
        end
    endtask

    task pulse_core;
        input signed [23:0] left_value;
        input signed [23:0] right_value;
        begin
            @(negedge clk);
            core_left_in = left_value;
            core_right_in = right_value;
            core_valid = 1'b1;
            @(negedge clk);
            core_valid = 1'b0;
            #1;
            if (!core_out_valid)
                fail("core output-valid missing");
        end
    endtask

    task push_delay;
        input signed [23:0] value;
        input signed [23:0] expected;
        begin
            while (!delay_ready) @(negedge clk);
            delay_left_in = value;
            delay_right_in = -value;
            delay_valid = 1'b1;
            @(negedge clk);
            delay_valid = 1'b0;
            while (!delay_out_valid) @(negedge clk);
            #1;
            if (delay_left_out !== expected || delay_right_out !== -expected)
                fail("delay output mismatch");
        end
    endtask

    task send_i2s_channel;
        input channel;
        input [23:0] value;
        integer bit_index;
        begin
            @(negedge clk);
            i2s_lrclk = channel;
            i2s_data = 1'b0;
            @(negedge clk); // one-bit I2S delay
            for (bit_index = 23; bit_index >= 0; bit_index = bit_index - 1) begin
                i2s_data = value[bit_index];
                @(negedge clk);
            end
            repeat (8) begin
                i2s_data = 1'b0;
                @(negedge clk);
            end
        end
    endtask

    task axi_write;
        input [5:0] address;
        input [31:0] value;
        input [1:0] expected_response;
        begin
            while (!axi_awready || !axi_wready) @(negedge clk);
            axi_awaddr = address;
            axi_wdata = value;
            axi_awvalid = 1'b1;
            axi_wvalid = 1'b1;
            @(negedge clk);
            axi_awvalid = 1'b0;
            axi_wvalid = 1'b0;
            while (!axi_bvalid) @(negedge clk);
            if (axi_bresp !== expected_response)
                fail("AXI write response mismatch");
            @(negedge clk);
        end
    endtask

    task axi_read;
        input [5:0] address;
        input [31:0] expected_value;
        begin
            while (!axi_arready) @(negedge clk);
            axi_araddr = address;
            axi_arvalid = 1'b1;
            @(negedge clk);
            axi_arvalid = 1'b0;
            while (!axi_rvalid) @(negedge clk);
            if (axi_rresp !== 2'b00 || axi_rdata !== expected_value)
                fail("AXI read data/response mismatch");
            @(negedge clk);
        end
    endtask

    initial begin
        repeat (4) @(negedge clk);
        reset_n = 1'b1;

        pulse_core(24'sd2097152, -24'sd2097152);
        if (core_left_out !== 24'sd2097152 ||
            core_right_out !== -24'sd2097152)
            fail("unity/DC first-sample mismatch");
        pulse_core(24'sd2097152, -24'sd2097152);
        if (core_left_out !== 24'sd0 || core_right_out !== 24'sd0)
            fail("DC blocker state mismatch");

        @(negedge clk);
        reset_n = 1'b0;
        repeat (2) @(negedge clk);
        reset_n = 1'b1;
        gain_q23 = 26'sd16777216; // 2.0
        pulse_core(24'sd6291456, -24'sd6291456);
        if (core_left_out !== 24'sh7fffff ||
            core_right_out !== -24'sd8388608)
            fail("gain saturation mismatch");

        @(negedge clk);
        reset_n = 1'b0;
        repeat (2) @(negedge clk);
        reset_n = 1'b1;
        gain_q23 = 26'sd0;
        osc_select = 2'd2;
        synth_level_q23 = 24'sh7fffff;
        pulse_core(24'sd0, 24'sd0);
        if (core_left_out > -24'sd8388600 || core_left_out !== core_right_out)
            fail("square oscillator mix mismatch");

        push_delay(24'sd100, 24'sd0);
        push_delay(24'sd200, 24'sd0);
        push_delay(24'sd300, 24'sd100);

        send_i2s_channel(1'b0, 24'h123456);
        send_i2s_channel(1'b1, 24'hfedcba);
        #1;
        if (!i2s_frame_seen || captured_i2s_left !== 24'sh123456 ||
            captured_i2s_right !== 24'shfedcba)
            fail("I2S receive alignment mismatch");

        axi_read(6'h24, 32'h41554431);
        axi_write(6'h04, 32'd16777216, 2'b00);
        if ($signed(axi_param_bus[25:0]) !== 26'sd16777216)
            fail("AXI gain register mismatch");
        axi_write(6'h00, 32'd1, 2'b00);
        axi_write(6'h0c, 32'h12345678, 2'b10);
        axi_ack_toggle = axi_update_toggle;
        repeat (2) @(negedge clk);
        axi_write(6'h0c, 32'h12345678, 2'b00);
        if (axi_param_bus[81:50] !== 32'h12345678)
            fail("AXI phase register mismatch");

        if (failure_count == 0)
            $display("FPGA_RTL_TESTS_OK");
        else
            $display("FPGA_RTL_TESTS_FAILED=%0d", failure_count);
        $finish;
    end
endmodule
