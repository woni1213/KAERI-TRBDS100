`timescale 1ns / 1ps
/*
14 Bit 2.5 MSps ADC (400ns - 80 Clock)
System Clock : 200 MHz, 5ns
Conversion Time = 225ns (225ns / 5ns = 45)
Conv - SPI Delay = 20ns
SPI End Delay = 15ns
SPI 14Bit Time > 140ns (1 Cycle > 28ns)
ADC Freq = Min 400ns (2.5 MHz)

400ns로 테스트 해봐야함
*/

module ADC_LTC2313_14 #
(
    parameter integer DATA_WIDTH = 14,
    parameter integer AWIDTH = 16,
    parameter integer MEM_SIZE = 1300,

    parameter integer ADC_CONV_TIME = 45
)
(
    input i_fRST,
    input i_clk,

    // ZYNQ Ports
    input i_beam_trg,                                   // Beam Trigger 입력. RAM에 ADC Data 저장 시작
    output o_adc_conv,                                  // ADC IC CNV Pin
    output o_adc_data_save_flag,                        // PS Interlock. Ram 저장 완료
    output o_adc_state,                                 // Acc용 Flag

    // SPI
    input [2:0] i_spi_state,                            // SPI 상태 확인. SPI 전송 완료 신호로 사용
    output o_spi_start,                                 // SPI 전송 시작
    output [DATA_WIDTH - 1 : 0] o_spi_data,             // MOSI Data는 사용하지 않음. 0으로 설정

    // ADC Setup
    input [9:0] i_adc_freq,                             // ADC 측정 주기. 최소 80 이상. 80 이하면 동작하지 않음.
                                                        // 80 (0.4us) ~ 1024 (5.12us). X 5ns
    input [$clog2(MEM_SIZE) : 0] i_adc_data_ram_size,   // ADC Data RAM 크기. MEM_SIZE 상수에 의해서 10000 이하 설정.

    // RAM
    output reg [AWIDTH - 1 : 0] o_ram_addr,             // RAM Memory Addr.
    output o_ram_ce,                                    // RAM CE. 1로 고정                    
    output o_ram_we                                     // RAM WE. 1로 고정
);

    // state machine
    parameter idle = 0;
    parameter adc_conv = 1;
    parameter adc_acq = 2;
    parameter save = 3;

    reg [2:0] state;
    reg [2:0] n_state;

    //-- time counter --//
    reg [9:0] adc_freq_cnt;                             // ADC 측정 주기 카운터
    reg [2:0] trigger_cnt;

    //-- flag --//
    wire adc_conv_flag;                                 // ADC 시작용 내부 Flag
    reg adc_trg_flag;                                   // Beam Trigger 구분 Flag. 1이면 State가 save로 0이면 idle로 감
    reg adc_trg_np_flag;                                // ADC Trigger Neg, Pos Flag
                                                        // Trigger Level이 변화하기 전에 데이터 저장이 끝나서 다시 데이터를 덮어 쓰는것을 방지

    wire adc_trg_ons;

    // State Machine 상태
    always @(posedge i_clk or negedge i_fRST) 
    begin
        if (~i_fRST)
            state <= idle;

        else 
            state <= n_state;
    end

    // State Machine Control
    always @(*)
    begin
        case (state)
            idle :
            begin
                if (adc_conv_flag)
                    n_state <= adc_conv;

                else
                    n_state <= idle;
            end

            adc_conv :
            begin
                if (o_spi_start)
                    n_state <= adc_acq;

                else
                    n_state <= adc_conv;
            end

            adc_acq :
            begin
                if (i_spi_state == 4)                   // spi 전송 완료
                begin
                    if (adc_trg_flag)
                        n_state <= save;

                    else
                        n_state <= idle;
                end

                else
                    n_state <= adc_acq;
            end

            save :
                n_state <= idle;

            default :
                    n_state <= idle;
        endcase
    end

    // adc 전체 동작 카운터
    always @(posedge i_clk or negedge i_fRST) 
    begin
        if (~i_fRST)
            adc_freq_cnt <= 0;

        else if (adc_freq_cnt == i_adc_freq)
            adc_freq_cnt <= 0;

        else
            adc_freq_cnt <= adc_freq_cnt + 1;
    end

    // Trigger Flag
    always @(posedge i_clk or negedge i_fRST) 
    begin
        if (~i_fRST)
            adc_trg_flag <= 0;

        else if (trigger_cnt == 1)      // 만약 adc_trg_flag가 0으로 초기화가 안된 경우 강제로 0으로 만듬
            adc_trg_flag <= 0; 
        
        else if (adc_trg_ons)
            adc_trg_flag <= 1;

        else if ((o_ram_addr >= i_adc_data_ram_size - 1) && (state == idle))
            adc_trg_flag <= 0;

        else
            adc_trg_flag <= adc_trg_flag;
    end


    always @(posedge i_clk or negedge i_fRST) 
    begin
        if (~i_fRST)
            o_ram_addr <= 0;

        else if ((state == save) && (o_ram_addr < i_adc_data_ram_size))
            o_ram_addr <= o_ram_addr + 1;

        else if (adc_trg_ons)
            o_ram_addr <= 0;

        else
            o_ram_addr <= o_ram_addr;
    end


    always @(posedge i_clk or negedge i_fRST) 
    begin
        if (~i_fRST)
            trigger_cnt <= 0;

        else if ((!i_beam_trg) && (trigger_cnt < 5))
            trigger_cnt <= trigger_cnt + 1;

        else if (i_beam_trg)
            trigger_cnt <= 0;

        else
            trigger_cnt <= trigger_cnt;
    end

    
    assign o_adc_conv = (adc_freq_cnt < ADC_CONV_TIME) ? 1 : 0;                         // ADC Conversion Hold Time 225ns 
    assign o_spi_start = (adc_freq_cnt == (ADC_CONV_TIME + 1)) ? 1 : 0;                 // ADC Acquisition Start flag
    assign o_ram_we = 1;                                            
    assign o_ram_ce = 1;       
    assign o_spi_data = 0;                                      
    assign o_adc_data_save_flag = adc_trg_flag;                                        // RAM 저장 완료 신호. PS 인터럽트
    assign adc_conv_flag = ((adc_freq_cnt == 0) && (i_adc_freq >= 80)) ? 1 : 0;         // ADC Conversion Start flag
    assign o_adc_state = (state == save) ? 1 : 0;                                       // Save의 경우에만 데이터 ACC
    assign adc_trg_ons = (trigger_cnt == 2) ? 1 : 0;        // 1은 초기화, 2는 Trigger 시작
endmodule