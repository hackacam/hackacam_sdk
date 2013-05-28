/**************************************************************************
Copyright (c) 2007 Stretch, Inc. All rights reserved.  Stretch products are
protected under numerous U.S. and foreign patents, maskwork rights,
copyrights and other intellectual property laws.  

This source code and the related tools, software code and documentation, and
your use thereof, are subject to and governed by the terms and conditions of
the applicable Stretch IDE or SDK and RDK License Agreement (either as agreed
by you or found at www.stretchinc.com).  By using these items, you indicate
your acceptance of such terms and conditions between you and Stretch, Inc.
In the event that you do not agree with such terms and conditions, you may
not use any of these items and must immediately destroy any copies you have
made.
***************************************************************************/

#ifndef STRETCH_DIAG_ERRORS_H
#define STRETCH_DIAG_ERRORS_H


/**************************************************************************
    Boot-loader DDR diagnostic test errors:

        "DDR_DIAG_WRITEREAD_FAIL" - DDR write/read test failed

        "DDR_DIAG_ADDRLINES_FAIL" - DDR address lines test failed

        "DDR_DIAG_BITFLIP_FAIL" - DDR bit-flip test failed

        "DDR_DIAG_DMA_FAIL" - DDR DMA test failed

        "DDR_DIAG_READ_DMA_FAIL" - DDR read/DMA test failed
***************************************************************************/
#define DDR_DIAG_WRITEREAD_FAIL         0xb007e001
#define DDR_DIAG_ADDRLINES_FAIL         0xb007e002
#define DDR_DIAG_BITFLIP_FAIL           0xb007e003
#define DDR_DIAG_DMA_FAIL               0xb007e004
#define DDR_DIAG_READ_DMA_FAIL          0xb007e005


/**************************************************************************
    PLL diagnostic test errors:

        "PLL_TEST_MHZ_ERR" - Processor speed test failed

        "PLL_TEST_SYS_PLL_ERR" - PLL_SYS test failed

        "PLL_TEST_IO_PLL_ERR" - PLL_IO test failed

        "PLL_TEST_AIM_PLL_ERR" - PLL_AIM test failed

        "PLL_TEST_DP0_PLL_ERR" - PLL_DP0 test failed

        "PLL_TEST_DP2_PLL_ERR" - PLL_DP2 test failed

        "PLL_TEST_DDR_DLL_ERR" - DLL_DDR test failed

        "PLL_TEST_ARM_PLL_ERR" - PLL_ARM test failed

        "PLL_TEST_MAC_PLL_ERR" - PLL_MAC test failed
***************************************************************************/
#define PLL_TEST_MHZ_ERR                0x1000e001
#define PLL_TEST_SYS_PLL_ERR            0x1000e002
#define PLL_TEST_IO_PLL_ERR             0x1000e003
#define PLL_TEST_AIM_PLL_ERR            0x1000e004
#define PLL_TEST_DP0_PLL_ERR            0x1000e005
#define PLL_TEST_DP2_PLL_ERR            0x1000e006
#define PLL_TEST_DDR_DLL_ERR            0x1000e007
#define PLL_TEST_ARM_PLL_ERR            0x1000e008
#define PLL_TEST_MAC_PLL_ERR            0x1000e009


/**************************************************************************
    SPI flash diagnostic test errors:

        "SPI_TEST_READ_ERR" - Flash read error

        "SPI_TEST_ERASE_ERR" - Flash erase error

        "SPI_TEST_PROG_ERR" - Flash program error

        "SPI_TEST_UNLOCK_ERR" - Flash unlock error

        "SPI_TEST_COMPARE_ERR" - Flash data miscompare error

        "SPI_TEST_MAINT_ERR" - Flash maintenance command error

        "SPI_TEST_MISC_ERR" - Miscellaneous flash error
***************************************************************************/
#define SPI_TEST_READ_ERR               0x1001e001
#define SPI_TEST_ERASE_ERR              0x1001e002
#define SPI_TEST_PROG_ERR               0x1001e003
#define SPI_TEST_UNLOCK_ERR             0x1001e004
#define SPI_TEST_COMPARE_ERR            0x1001e005
#define SPI_TEST_MAINT_ERR              0x1001e006
#define SPI_TEST_MISC_ERR               0x1001e007


/**************************************************************************
    TWI EEPROM diagnostic test errors:

        "TWI_EEPROM_TEST_READ_ERR" - TWI EEPROM read error

        "TWI_EEPROM_TEST_WRITE_ERR" - TWI EEPROM write error

        "TWI_EEPROM_TEST_INIT_ERR" - TWI EEPROM initialization error

        "TWI_EEPROM_TEST_COMPARE_ERR" - TWI EEPROM data miscompare error

        "TWI_EEPROM_TEST_WP_COMPARE_ERR" - TWI EEPROM write-protect error

        "TWI_EEPROM_TEST_CRYPTO_ERR" - TWI crypto-EEPROM error
***************************************************************************/
#define TWI_EEPROM_TEST_READ_ERR        0x1002e001
#define TWI_EEPROM_TEST_WRITE_ERR       0x1002e002
#define TWI_EEPROM_TEST_INIT_ERR        0x1002e003
#define TWI_EEPROM_TEST_COMPARE_ERR     0x1002e004
#define TWI_EEPROM_TEST_WP_COMPARE_ERR  0x1002e005
#define TWI_EEPROM_TEST_CRYPTO_ERR      0x1002e006


/**************************************************************************
    Epson diagnostic test errors:

        "EPSON_REG_TEST_INIT_ERR" - Epson test initialization error

        "EPSON_REG_TEST_WALKING_ERR" - Epson register bit-walk error
***************************************************************************/
#define EPSON_REG_TEST_INIT_ERR         0x1003e001
#define EPSON_REG_TEST_WALKING_ERR      0x1003e002


/**************************************************************************
    Decoder diagnostic test errors:

        "DECODER_AUDIO_TEST_INIT_ERR" - Decoder audio test init error

        "DECODER_AUDIO_TEST_NO_AUDIO_ERR" - Decoder audio not received error

        "TW2815_REG_TEST_ERR" - Decoder register test error

        "TW2864_REG_TEST_ERR" - Decoder register test error

        "TW2866_REG_TEST_ERR" - Decoder register test error

        "DECODER_VIDEO_TEST_INIT_ERR" - Decoder video test init error

        "DECODER_VIDEO_TEST_NO_VIDEO_ERR" - Decoder video not received error

        "DECODER_VIDEO_TEST_TIMEOUT" - Decoder video test timeout error

        "DECODER_VIDDET_TEST_INIT_ERR" - Decoder A/V detection: init error

        "DECODER_VIDDET_TEST_UNKNOWN_CHIP" - Decoder A/V detection: unknown 
        chip error

        "DECODER_VIDDET_TEST_NO_INPUT_ERR" - Decoder A/V detection: input 
        signal not detected error

        "DECODER_VIDDET_TEST_CONFLICT_ERR" - Decoder A/V detection: video
        standard conflict error

        "DECODER_VIDDET_TEST_NO_SYNC_ERR" - Decoder A/V detection: no video
        sync error

        "DECODER_AUDDET_TEST_NO_SYNC_ERR" - Decoder A/V detection: no audio
        sync error

        "DECODER_UNIQUE_VIDEO_TEST_ERR" - Video inputs are not unique

        "NVP1104_REG_TEST_ERR" - Nextchip register test error

        "NVP1114_REG_TEST_ERR" - Nextchip register test error

        "DECODER_AUDIO_TEST_TIMEOUT" - Decoder audio test timeout error

        "DECODER_UNIQUE_AUDIO_TEST_ERR" - Audio inputs are not unique

        "GV7601_REG_TEST_ERR" - Gennum register test error

        "DEMUX_TEST_ERR" - Demux test error

        "DIAG_TIMEOUT_ERR" - Timeout error

        "CX25828_REG_TEST_ERR" - Conexant register test error
***************************************************************************/
#define DECODER_AUDIO_TEST_INIT_ERR         0x1004e001
#define DECODER_AUDIO_TEST_NO_AUDIO_ERR     0x1004e002
#define TW2815_REG_TEST_ERR                 0x1004e003
#define TW2864_REG_TEST_ERR                 0x1004e013
#define TW2866_REG_TEST_ERR                 0x1004e014
#define TW2868_REG_TEST_ERR                 0x1004e016
#define DECODER_VIDEO_TEST_INIT_ERR         0x1004e004
#define DECODER_VIDEO_TEST_NO_VIDEO_ERR     0x1004e005
#define DECODER_VIDEO_TEST_TIMEOUT          0x1004e015
#define DECODER_VIDDET_TEST_INIT_ERR        0x1004e006
#define DECODER_VIDDET_TEST_UNKNOWN_CHIP    0x1004e007
#define DECODER_VIDDET_TEST_NO_INPUT_ERR    0x1004e008
#define DECODER_VIDDET_TEST_CONFLICT_ERR    0x1004e009
#define DECODER_VIDDET_TEST_NO_SYNC_ERR     0x1004e00a
#define DECODER_AUDDET_TEST_NO_SYNC_ERR     0x1004e00b
#define DECODER_UNIQUE_VIDEO_TEST_ERR       0x1004e00c
#define NVP1104_REG_TEST_ERR                0x1004e01d
#define NVP1114_REG_TEST_ERR                0x1004e00d
#define DECODER_AUDIO_TEST_TIMEOUT          0x1004e00e
#define DECODER_UNIQUE_AUDIO_TEST_ERR       0x1004e00f
#define GV7601_REG_TEST_ERR                 0x1004e010
#define DEMUX_TEST_ERR                      0x1004e011
#define DIAG_TIMEOUT_ERR                    0x1004e012
#define CX25828_REG_TEST_ERR                0x1004e013


/**************************************************************************
    PCIe diagnostic test errors:

        "PCIE_EYEMASK_TEST_NO_CBB" - PCIe did not detect the CBB test board.

        "PCIE_EYEMASK_TEST_ERR" - PCIe eyemask test failure.

        "PCIE_EYEMASK_TEST_TIMEOUT" - PCIe eyemask test timeout.

        "PCIE_LOOPBACK_TEST_ERR" - PCIe loopback test failure.
***************************************************************************/
#define PCIE_EYEMASK_TEST_NO_CBB        0x1005e001
#define PCIE_EYEMASK_TEST_ERR           0x1005e002
#define PCIE_EYEMASK_TEST_TIMEOUT       0x1005e003
#define PCIE_LOOPBACK_TEST_ERR          0x1005e004


/**************************************************************************
    AIM diagnostic test errors:

        "AIM_TEST_ERRORS" - AIM errors were detected during the test.
***************************************************************************/
#define AIM_TEST_ERRORS                 0x1006e001


/**************************************************************************
    SMO output test errors:

        "ASWITCH_ENABLE_ERR" - Analog switch output enable error

        "SMO_ENABLE_ERR" - SMO output enable error

        "VOUT_ENABLE_ERR" - VOUT output enable error

        "AOUT_ENABLE_ERR" - AOUT output enable error
***************************************************************************/
#define ASWITCH_ENABLE_ERR              0x1007e001
#define SMO_ENABLE_ERR                  0x1007e002
#define VOUT_ENABLE_ERR                 0x1007e003
#define AOUT_ENABLE_ERR                 0x1007e004


/**************************************************************************
    I/O board test errors (not run in field diagnostics):

        "IOBOARD_LOOPBACK_ERR" - I/O board loopback test error

        "IOBOARD_CAMTERM_ERR" - Camera termination test error

        "IOBOARD_ALARMIN_ERR" - Alarm input test error

        "IOBOARD_ALARMOUT_ERR" - Alarm output test error

        "IOBOARD_ALARMOUTRELAY_ERR" - Alarm output relay test error

        "IOBOARD_LED_ERR" - LED test error
***************************************************************************/
#define IOBOARD_LOOPBACK_ERR            0x1008e001
#define IOBOARD_CAMTERM_ERR             0x1008e002
#define IOBOARD_ALARMIN_ERR             0x1008e003
#define IOBOARD_ALARMOUT_ERR            0x1008e004
#define IOBOARD_ALARMOUTRELAY_ERR       0x1008e005
#define IOBOARD_LED_ERR                 0x1008e006


/**************************************************************************
    Global reset errors (not run in field diagnostics):

        "GLOBAL_RESET_TEST_ERR" - Global reset test error
***************************************************************************/
#define GLOBAL_RESET_TEST_ERR           0x1009e001


/**************************************************************************
    TWI Scan test errors:

        "TWI_SCAN_TEST_INIT_ERR" - TWI Scan initialization error

        "TWI_SCAN_TEST_ERR" - TWI Scan test error
***************************************************************************/
#define TWI_SCAN_TEST_INIT_ERR          0x100ae001
#define TWI_SCAN_TEST_ERR               0x100ae002

#endif /* STRETCH_DIAG_ERRORS_H */

