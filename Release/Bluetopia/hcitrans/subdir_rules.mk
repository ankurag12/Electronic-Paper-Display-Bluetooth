################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Bluetopia/hcitrans/HCITRANS.obj: ../Bluetopia/hcitrans/HCITRANS.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --data_model=small -O2 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/include" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btvs" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Bluetopia/hcitrans" --include_path="C:/Users/Robo/Repos/EPD-BT-Workspace/pl-epd-ti-bt-v4/Hardware/msp430" --advice:power=all --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="Bluetopia/hcitrans/HCITRANS.pp" --obj_directory="Bluetopia/hcitrans" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


