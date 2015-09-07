################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Hardware/MSP430_EXP5438/HAL.obj: ../Hardware/MSP430_EXP5438/HAL.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=small --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.4/include" --include_path="C:/Users/Robo/Google Drive/sTicker/Coding Stuff/CCS Projects/EPD with BT SPP/SPPDemo_Standalone/Hardware" --include_path="C:/Users/Robo/Google Drive/sTicker/Coding Stuff/CCS Projects/EPD with BT SPP/SPPDemo_Standalone/Hardware/MSP430_EXP5438" --include_path="C:/Users/Robo/Google Drive/sTicker/Coding Stuff/CCS Projects/EPD with BT SPP/SPPDemo_Standalone/Bluetopia/include" --include_path="C:/Users/Robo/Google Drive/sTicker/Coding Stuff/CCS Projects/EPD with BT SPP/SPPDemo_Standalone/Bluetopia/btpsvend" --include_path="C:/Users/Robo/Google Drive/sTicker/Coding Stuff/CCS Projects/EPD with BT SPP/SPPDemo_Standalone/Bluetopia/btvs" --include_path="C:/Users/Robo/Google Drive/sTicker/Coding Stuff/CCS Projects/EPD with BT SPP/SPPDemo_Standalone/Bluetopia/btpskrnl" --include_path="C:/Users/Robo/Google Drive/sTicker/Coding Stuff/CCS Projects/EPD with BT SPP/SPPDemo_Standalone/Bluetopia/hcitrans" -g --define=__MSP430F5438A__ --define=__DISABLE_SMCLK__ --define=BTPS_MEMORY_BUFFER_SIZE=3250 --diag_warning=225 --display_error_number --diag_suppress=172 --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="Hardware/MSP430_EXP5438/HAL.pp" --obj_directory="Hardware/MSP430_EXP5438" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


