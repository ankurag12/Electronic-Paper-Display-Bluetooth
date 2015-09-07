################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
epd_sys/crc16.obj: ../epd_sys/crc16.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --data_model=small -O2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware/msp430" --advice:power=all --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="epd_sys/crc16.pp" --obj_directory="epd_sys" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

epd_sys/dac-5820.obj: ../epd_sys/dac-5820.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --data_model=small -O2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware/msp430" --advice:power=all --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="epd_sys/dac-5820.pp" --obj_directory="epd_sys" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

epd_sys/i2c-eeprom.obj: ../epd_sys/i2c-eeprom.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --data_model=small -O2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware/msp430" --advice:power=all --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="epd_sys/i2c-eeprom.pp" --obj_directory="epd_sys" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

epd_sys/lzss.obj: ../epd_sys/lzss.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --data_model=small -O2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware/msp430" --advice:power=all --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="epd_sys/lzss.pp" --obj_directory="epd_sys" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

epd_sys/pmic-max17135.obj: ../epd_sys/pmic-max17135.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --data_model=small -O2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware/msp430" --advice:power=all --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="epd_sys/pmic-max17135.pp" --obj_directory="epd_sys" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

epd_sys/pmic-tps65185.obj: ../epd_sys/pmic-tps65185.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --data_model=small -O2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware/msp430" --advice:power=all --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="epd_sys/pmic-tps65185.pp" --obj_directory="epd_sys" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

epd_sys/pnm-utils.obj: ../epd_sys/pnm-utils.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --data_model=small -O2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware/msp430" --advice:power=all --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="epd_sys/pnm-utils.pp" --obj_directory="epd_sys" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

epd_sys/probe.obj: ../epd_sys/probe.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --data_model=small -O2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware/msp430" --advice:power=all --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="epd_sys/probe.pp" --obj_directory="epd_sys" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

epd_sys/utils.obj: ../epd_sys/utils.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --data_model=small -O2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware/msp430" --advice:power=all --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="epd_sys/utils.pp" --obj_directory="epd_sys" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

epd_sys/vcom.obj: ../epd_sys/vcom.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --data_model=small -O2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware/msp430" --advice:power=all --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="epd_sys/vcom.pp" --obj_directory="epd_sys" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


