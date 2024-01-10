`timescale 1ns / 1ps

/*

*/

module DATA_ACC
(
    input i_fRST,
    input i_clk,

    input i_adc_state,
    input i_acc_flag,
    input i_acc_reset,
    input i_beam_cycle_flag,

    input [13:0] i_adc_raw_data,
    input [44:0] i_acc_threshold,

    output reg [22:0] o_beam_cycle_acc_data,
    output reg [44:0] o_beam_pulse_acc_data,
    output reg [21:0] o_beam_pulse_cnt,
    output o_beam_interlock
);
    parameter init = 0;
    parameter idle = 1;
    parameter cycle_init = 2;
    parameter cycle_acc = 3;
    parameter pulse_acc = 4;

    reg [2:0] state;
    reg [2:0] n_state;

    // State Machine Init
    always @(posedge i_clk or negedge i_fRST) 
    begin
        if (~i_fRST)
            state <= init;

        else 
            state <= n_state;
    end

    // State Machine
    always @(*)
    begin
        case (state)
            init :
            begin
                if (i_acc_flag)
                    n_state <= idle;

                else
                    n_state <= init;
            end

            idle :
            begin
                if (i_acc_reset)
                    n_state <= init;

                else if (i_beam_cycle_flag && i_acc_flag)
                    n_state <= cycle_init;

                else
                    n_state <= idle;
            end

            cycle_init :
                n_state <= cycle_acc;

            cycle_acc :
            begin
                if (i_acc_reset)
                    n_state <= init;

                else if (~i_beam_cycle_flag)
                    n_state <= pulse_acc;

                else
                    n_state <= cycle_acc;
            end

            pulse_acc :
                n_state <= idle;


            default :
                    n_state <= idle;
        endcase
    end

    // Beam Cycle Acc
    always @(posedge i_clk or negedge i_fRST)
    begin
        if (~i_fRST)
            o_beam_cycle_acc_data <= 0;

        else if (state == init)
            o_beam_cycle_acc_data <= 0;

        else if (state == cycle_init)
            o_beam_cycle_acc_data <= 0;

        else if (state == cycle_acc && i_adc_state)
            o_beam_cycle_acc_data <= o_beam_cycle_acc_data + i_adc_raw_data;
        
        else 
            o_beam_cycle_acc_data <= o_beam_cycle_acc_data;
    end

    // Beam Pulse Acc
    always @(posedge i_clk or negedge i_fRST)
    begin
        if (~i_fRST)
            o_beam_pulse_acc_data <= 0;

        else if (state == init)
            o_beam_pulse_acc_data <= 0;

        else if (state == pulse_acc)
            o_beam_pulse_acc_data <= o_beam_pulse_acc_data + o_beam_cycle_acc_data;

        else
            o_beam_pulse_acc_data <= o_beam_pulse_acc_data;
    end

    // Beam Count Acc
    always @(posedge i_clk or negedge i_fRST)
    begin
        if (~i_fRST)
            o_beam_pulse_cnt <= 0;

        else if (state == init)
            o_beam_pulse_cnt <= 0;

        else if (state == cycle_init)
            o_beam_pulse_cnt <= o_beam_pulse_cnt + 1;
        
        else 
            o_beam_pulse_cnt <= o_beam_pulse_cnt;
    end

    assign o_beam_interlock = (i_acc_threshold <= o_beam_pulse_acc_data) ? 1 : 0;
    
endmodule