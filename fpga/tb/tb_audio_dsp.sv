`timescale 1ns/1ps

module tb_audio_dsp;
    reg clk = 1'b0;
    reg audio_clk = 1'b0;
    reg reset_n = 1'b0;
    always #5 clk = ~clk;
    always #7 audio_clk = ~audio_clk;

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
    integer sample_index;

    wire selftest_led;
    aud_bx71_selftest_top #(
        .CLOCK_HZ(1000),
        .SAMPLE_RATE_HZ(50),
        .DELAY_ADDR_WIDTH(3),
        .DELAY_DEPTH(8),
        .DELAY_SAMPLES(3),
        .HEARTBEAT_BIT(5),
        .ERROR_BLINK_BIT(2)) selftest (
        .pl_clk_50m(clk),
        .reset_n(reset_n),
        .fpga_led0(selftest_led));
    aud_i2s_slave_rx receiver (
        .bclk(clk), .reset_n(reset_n), .lrclk(i2s_lrclk),
        .serial_data(i2s_data), .frame_valid(i2s_frame_valid),
        .left_sample(i2s_left), .right_sample(i2s_right));

    reg tx_lrclk = 1'b0;
    reg signed [23:0] tx_left = 24'sh123456;
    reg signed [23:0] tx_right = 24'shfedcba;
    wire tx_data;
    aud_i2s_slave_tx transmitter (
        .bclk(clk), .reset_n(reset_n), .lrclk(tx_lrclk),
        .left_sample(tx_left), .right_sample(tx_right),
        .serial_data(tx_data));

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
    wire [175:0] axi_param_bus;
    wire axi_update_toggle;
    reg cdc_frame_boundary = 1'b0;
    wire [175:0] cdc_param_bus;
    wire cdc_ack_audio;
    reg cdc_ack_sync_1 = 1'b0;
    reg cdc_ack_sync_2 = 1'b0;

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
        .ack_toggle(cdc_ack_sync_2), .param_bus(axi_param_bus),
        .update_toggle(axi_update_toggle));

    aud_param_cdc #(.WIDTH(176)) cdc (
        .audio_clk(audio_clk), .audio_reset_n(reset_n),
        .frame_boundary(cdc_frame_boundary),
        .param_bus_async(axi_param_bus),
        .update_toggle_async(axi_update_toggle),
        .param_bus_audio(cdc_param_bus),
        .ack_toggle_audio(cdc_ack_audio));

    always @(posedge clk) begin
        if (!reset_n) begin
            cdc_ack_sync_1 <= 1'b0;
            cdc_ack_sync_2 <= 1'b0;
        end else begin
            cdc_ack_sync_1 <= cdc_ack_audio;
            cdc_ack_sync_2 <= cdc_ack_sync_1;
        end
    end

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

    task check_i2s_tx_channel;
        input channel;
        input [23:0] expected;
        reg [23:0] received;
        integer bit_index;
        begin
            received = 24'd0;
            @(negedge clk);
            tx_lrclk = channel;
            @(posedge clk); // transmitter samples the new LRCLK value
            for (bit_index = 23; bit_index >= 0; bit_index = bit_index - 1) begin
                @(negedge clk); // transmitter launches on the falling edge
                @(posedge clk); // codec captures on the rising edge
                received[bit_index] = tx_data;
            end
            if (received !== expected)
                fail("I2S transmit alignment mismatch");
            repeat (8) begin
                @(negedge clk);
                @(posedge clk);
                if (tx_data !== 1'b0)
                    fail("I2S transmit slot padding is not zero");
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

    task axi_write_split;
        input [5:0] address;
        input [31:0] value;
        input address_first;
        begin
            if (address_first) begin
                while (!axi_awready) @(negedge clk);
                axi_awaddr = address;
                axi_awvalid = 1'b1;
                @(negedge clk);
                axi_awvalid = 1'b0;
                repeat (2) @(negedge clk);
                while (!axi_wready) @(negedge clk);
                axi_wdata = value;
                axi_wvalid = 1'b1;
                @(negedge clk);
                axi_wvalid = 1'b0;
            end else begin
                while (!axi_wready) @(negedge clk);
                axi_wdata = value;
                axi_wvalid = 1'b1;
                @(negedge clk);
                axi_wvalid = 1'b0;
                repeat (2) @(negedge clk);
                while (!axi_awready) @(negedge clk);
                axi_awaddr = address;
                axi_awvalid = 1'b1;
                @(negedge clk);
                axi_awvalid = 1'b0;
            end
            while (!axi_bvalid) @(negedge clk);
            if (axi_bresp !== 2'b00)
                fail("split AXI write response mismatch");
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

        // Cross the small test RAM's write-pointer wrap twice.
        for (sample_index = 4; sample_index <= 18; sample_index = sample_index + 1)
            push_delay(sample_index * 100, (sample_index - 2) * 100);

        // Reset invalidates old BRAM history without requiring a RAM clear.
        @(negedge clk);
        reset_n = 1'b0;
        repeat (4) @(negedge clk);
        reset_n = 1'b1;
        delay_samples = 3'd0; // bounded to one sample, never uninitialized RAM
        push_delay(24'sd111, 24'sd0);
        push_delay(24'sd222, 24'sd111);

        @(negedge clk);
        reset_n = 1'b0;
        repeat (4) @(negedge clk);
        reset_n = 1'b1;
        delay_samples = 3'd1;
        delay_feedback = 24'sh400000; // exactly 0.5
        push_delay(24'sd1024, 24'sd0);
        push_delay(24'sd0, 24'sd1024);
        push_delay(24'sd0, 24'sd512);
        push_delay(24'sd0, 24'sd256);
        delay_mix = 24'sd0;
        push_delay(24'sd333, 24'sd333);
        push_delay(-24'sd555, -24'sd555);

        send_i2s_channel(1'b0, 24'h123456);
        send_i2s_channel(1'b1, 24'hfedcba);
        #1;
        if (!i2s_frame_seen || captured_i2s_left !== 24'sh123456 ||
            captured_i2s_right !== 24'shfedcba)
            fail("I2S receive alignment mismatch");
        check_i2s_tx_channel(1'b1, 24'hfedcba);
        check_i2s_tx_channel(1'b0, 24'h123456);

        axi_read(6'h24, 32'h41554431);
        axi_write_split(6'h04, 32'd16777216, 1'b1);
        if ($signed(axi_param_bus[25:0]) !== 26'sd16777216)
            fail("AXI gain register mismatch");
        axi_write(6'h00, 32'd1, 2'b00);
        axi_write(6'h0c, 32'h12345678, 2'b10);
        repeat (4) @(posedge audio_clk);
        @(negedge audio_clk);
        if (cdc_param_bus !== 176'd0 || cdc_ack_audio !== 1'b0)
            fail("parameter CDC changed before a frame boundary");
        cdc_frame_boundary = 1'b1;
        @(negedge audio_clk);
        cdc_frame_boundary = 1'b0;
        repeat (6) @(negedge clk);
        if (cdc_ack_sync_2 !== axi_update_toggle)
            fail("parameter CDC acknowledgement missing");
        if ($signed(cdc_param_bus[25:0]) !== 26'sd16777216)
            fail("parameter CDC payload mismatch");
        axi_write_split(6'h0c, 32'h12345678, 1'b0);
        if (axi_param_bus[81:50] !== 32'h12345678)
            fail("AXI phase register mismatch");

        axi_wstrb = 4'b0010;
        axi_write(6'h0c, 32'h0000ab00, 2'b00);
        axi_read(6'h0c, 32'h1234ab78);
        axi_wstrb = 4'b0000;
        axi_write(6'h0c, 32'hffffffff, 2'b00);
        axi_read(6'h0c, 32'h1234ab78);
        axi_wstrb = 4'hf;
        axi_write(6'h0d, 32'hffffffff, 2'b10); // unaligned
        axi_write(6'h24, 32'hffffffff, 2'b10); // read-only ID
        axi_read(6'h0c, 32'h1234ab78);
        axi_read(6'h24, 32'h41554431);

        repeat (180) @(negedge clk);
        if (selftest.error_latched !== 1'b0)
            fail("BX71 internal self-test watchdog latched");
        if (selftest.accepted_count < 8 || selftest.output_count < 7)
            fail("BX71 internal self-test did not process enough samples");
        if (selftest.activity_latched !== 1'b1)
            fail("BX71 internal self-test saw no DSP activity");

        if (failure_count == 0)
            $display("FPGA_RTL_TESTS_OK");
        else
            $display("FPGA_RTL_TESTS_FAILED=%0d", failure_count);
        $finish;
    end

    initial begin
        #95000;
        fail("testbench watchdog: a handshake did not complete");
        $finish;
    end
endmodule
