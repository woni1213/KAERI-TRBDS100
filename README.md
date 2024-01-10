# TRBDS100
 양성자 100 MeV 표적실 ADC 장비
 전류 측정
 전류 조사량 누적 기능 (최대 3일)
 최대 500us 측정

## 0. Address Map
DIO   :     0x4000_0000  
ADC A :     0x4000_1000  
ADC B :     0x4000_2000  
ADC C :     0x4000_3000  
ADC D :     0x4000_4000
ADC E :     0x4000_5000  
ADC F :     0x4000_6000  
ADC G :     0x4000_7000  
ADC H :     0x4000_8000    

## 1. System
### a. 개요
- 8 Ch ADC, 8 Ch DIO Ports
- 8 Ch Interlock, 1 Ch Trigger Iuput Port
- System Clock 5ns (**FCLK_CLK0 : 200MHz**)
- 3 Range : 220uA / 2.2mA / 22mA 

### b. PL User IP 구성
- ADC_0
- ADC_1
- ADC_2
- ADC_3
- ADC_4
- ADC_5
- ADC_6
- ADC_7
- DIO_0

## 2. ADC IP
### a. 개요
- LTC2313-14, Internal Vref : 4.096V
- OpAmp - Negative Voltage Clamping - ADC
- SPI (50MHz)
- 최대 0.4us Sampling (2.5MHz, 1MHz / 1us로 동작 원함)
- 14 Bit
- RAM Size 최대 1300 (수정 가능, 2.5MHz일 경우 500us 당 1250개)
- Accmulate 기능
- Threshold에 의한 Interlock 기능

### b. 구성
- TOP_ADC.v
  - ADC_LTC2313.v (LTC2313-14 Control)
  - ADC_S00_AXI.v (AXI)
  - ADC_SPI.v (SPI)
  - DPBRAM.v (DPRAM)
  - DATA_ACC.v (Data Accumulate)

### c. 동작
- 전원이 켜지는 순간 설정된 Sampling 시간으로 ADC 동작. ADC Off 시 Sampling Time을 80 미만으로 설정.
- Sampling Time 80 이상 설정해야함. 80미만일 경우 동작 안함.
- Conversion Time : 225ns 고정 (Busy 신호가 없음)
- SPI 20MHz, CPOL : 0, CPHA : 0

### d. AXI
- **[READ Only]**
  - slv_reg 0 [15:0]    : 현재 ADC 값
  - slv_reg 1 [2:0]  +4 : SPI Status (Used Debug)
  - slv_reg 2 [15:0] +8 : RAM Data


- **[Write]**
  - slv_reg 3 [9:0]  +C     : ADC Sampling Time (240 이상) 최소 sampling 시간. * 5ns
  - slv_reg 4 [13:0] +10    : RAM Save Size : (Capture time = Size * Sampling Time)
  - slv_reg 5 [15:0] +14    : RAM Address

### e. 변환식
- 0V : ADC Data = 32768
- &#43; Voltage : (ADC Data * Gain) - Offset
- &#45; Voltage : ((65536 - ADC Data) * Gain) + Offset
- Channel : Gain / Offset
- 1 Ch : 0.000384 / 0.004332
- 2 Ch : 0.000385 / 0.005559
- 3 Ch : 0.000384 / 0.003755
- 4 Ch : 0.000386 / 0.004544

### f. Accmulate

### g. Interlock

### h. 주의 사항
- Sampling Time 240 이상


## 3. DIO
### a. 개요
- Digital Output
- 4 Ch DO
- 1 Ch Interrupt Output
- 1 Ch Trigger I/O
- Interrupt : 5V
- etc : 3.3V

### b. 구성
- Top_DIO.v
  - dio_s00_AXI.v (AXI)

### c. 동작
- 0 : High
- 1 : Low
- 모두 PS와 1:1 연결. 모든 핀 제어는 PS에서 해야함.

### d. AXI
- **[Write - reg 0]**
  - slv_reg 0 [0]    : Trigger Out
  - slv_reg 0 [1]    : Interrupt Out
- **[Write - reg 1]**
  - slv_reg 1 [0]  +4 : Ch 1 DO
  - slv_reg 1 [1]  +4 : Ch 2 DO
  - slv_reg 1 [2]  +4 : Ch 3 DO
  - slv_reg 1 [3]  +4 : Ch 4 DO


### e-mail : <woni1213@e-hmt.kr>