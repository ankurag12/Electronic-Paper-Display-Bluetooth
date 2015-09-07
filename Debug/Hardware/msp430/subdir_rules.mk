################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Hardware/msp430/HAL.obj: ../Hardware/msp430/HAL.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=small --opt_for_speed=1 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --advice:power="2,5" -g --gcc --define=__MSP430F5438A__ --define=__DISABLE_SMCLK__ --define=BTPS_MEMORY_BUFFER_SIZE=3250 --diag_warning=225 --display_error_number --diag_suppress=172 --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=nofloat --preproc_with_compile --preproc_dependency="Hardware/msp430/HAL.pp" --obj_directory="Hardware/msp430" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Hardware/msp430/hal_pmm.obj: ../Hardware/msp430/hal_pmm.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=small --opt_for_speed=1 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --advice:power="2,5" -g --gcc --define=__MSP430F5438A__ --define=__DISABLE_SMCLK__ --define=BTPS_MEMORY_BUFFER_SIZE=3250 --diag_warning=225 --display_error_number --diag_suppress=172 --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=nofloat --preproc_with_compile --preproc_dependency="Hardware/msp430/hal_pmm.pp" --obj_directory="Hardware/msp430" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Hardware/msp430/msp430-epd.obj: ../Hardware/msp430/msp430-epd.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=small --opt_for_speed=1 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --advice:power="2,5" -g --gcc --define=__MSP430F5438A__ --define=__DISABLE_SMCLK__ --define=BTPS_MEMORY_BUFFER_SIZE=3250 --diag_warning=225 --display_error_number --diag_suppress=172 --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=nofloat --preproc_with_compile --preproc_dependency="Hardware/msp430/msp430-epd.pp" --obj_directory="Hardware/msp430" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Hardware/msp430/msp430-flashmem.obj: ../Hardware/msp430/msp430-flashmem.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=small --opt_for_speed=1 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --advice:power="2,5" -g --gcc --define=__MSP430F5438A__ --define=__DISABLE_SMCLK__ --define=BTPS_MEMORY_BUFFER_SIZE=3250 --diag_warning=225 --display_error_number --diag_suppress=172 --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=nofloat --preproc_with_compile --preproc_dependency="Hardware/msp430/msp430-flashmem.pp" --obj_directory="Hardware/msp430" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Hardware/msp430/msp430-gpio.obj: ../Hardware/msp430/msp430-gpio.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=small --opt_for_speed=1 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --advice:power="2,5" -g --gcc --define=__MSP430F5438A__ --define=__DISABLE_SMCLK__ --define=BTPS_MEMORY_BUFFER_SIZE=3250 --diag_warning=225 --display_error_number --diag_suppress=172 --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=nofloat --preproc_with_compile --preproc_dependency="Hardware/msp430/msp430-gpio.pp" --obj_directory="Hardware/msp430" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Hardware/msp430/msp430-i2c.obj: ../Hardware/msp430/msp430-i2c.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=small --opt_for_speed=1 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --advice:power="2,5" -g --gcc --define=__MSP430F5438A__ --define=__DISABLE_SMCLK__ --define=BTPS_MEMORY_BUFFER_SIZE=3250 --diag_warning=225 --display_error_number --diag_suppress=172 --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=nofloat --preproc_with_compile --preproc_dependency="Hardware/msp430/msp430-i2c.pp" --obj_directory="Hardware/msp430" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Hardware/msp430/msp430-interrupts.obj: ../Hardware/msp430/msp430-interrupts.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=small --opt_for_speed=1 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --advice:power="2,5" -g --gcc --define=__MSP430F5438A__ --define=__DISABLE_SMCLK__ --define=BTPS_MEMORY_BUFFER_SIZE=3250 --diag_warning=225 --display_error_number --diag_suppress=172 --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=nofloat --preproc_with_compile --preproc_dependency="Hardware/msp430/msp430-interrupts.pp" --obj_directory="Hardware/msp430" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Hardware/msp430/msp430-sdcard.obj: ../Hardware/msp430/msp430-sdcard.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=small --opt_for_speed=1 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --advice:power="2,5" -g --gcc --define=__MSP430F5438A__ --define=__DISABLE_SMCLK__ --define=BTPS_MEMORY_BUFFER_SIZE=3250 --diag_warning=225 --display_error_number --diag_suppress=172 --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=nofloat --preproc_with_compile --preproc_dependency="Hardware/msp430/msp430-sdcard.pp" --obj_directory="Hardware/msp430" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Hardware/msp430/msp430-spi.obj: ../Hardware/msp430/msp430-spi.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=small --opt_for_speed=1 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --advice:power="2,5" -g --gcc --define=__MSP430F5438A__ --define=__DISABLE_SMCLK__ --define=BTPS_MEMORY_BUFFER_SIZE=3250 --diag_warning=225 --display_error_number --diag_suppress=172 --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=nofloat --preproc_with_compile --preproc_dependency="Hardware/msp430/msp430-spi.pp" --obj_directory="Hardware/msp430" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Hardware/msp430/msp430-timers.obj: ../Hardware/msp430/msp430-timers.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=small --opt_for_speed=1 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --advice:power="2,5" -g --gcc --define=__MSP430F5438A__ --define=__DISABLE_SMCLK__ --define=BTPS_MEMORY_BUFFER_SIZE=3250 --diag_warning=225 --display_error_number --diag_suppress=172 --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=nofloat --preproc_with_compile --preproc_dependency="Hardware/msp430/msp430-timers.pp" --obj_directory="Hardware/msp430" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Hardware/msp430/msp430-uart.obj: ../Hardware/msp430/msp430-uart.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=small --opt_for_speed=1 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --advice:power="2,5" -g --gcc --define=__MSP430F5438A__ --define=__DISABLE_SMCLK__ --define=BTPS_MEMORY_BUFFER_SIZE=3250 --diag_warning=225 --display_error_number --diag_suppress=172 --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=nofloat --preproc_with_compile --preproc_dependency="Hardware/msp430/msp430-uart.pp" --obj_directory="Hardware/msp430" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


