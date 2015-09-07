################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Bluetopia/btvs/BTVS.obj: ../Bluetopia/btvs/BTVS.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=small --opt_for_speed=1 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --advice:power="2,5" -g --gcc --define=__MSP430F5438A__ --define=__DISABLE_SMCLK__ --define=BTPS_MEMORY_BUFFER_SIZE=3250 --diag_warning=225 --display_error_number --diag_suppress=172 --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=nofloat --preproc_with_compile --preproc_dependency="Bluetopia/btvs/BTVS.pp" --obj_directory="Bluetopia/btvs" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


