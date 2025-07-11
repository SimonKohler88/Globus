/*----------------------------------------------*/
/* TJpgDec System Configurations R0.03          */
/*----------------------------------------------*/


// JPEG Decoder
//
// CONFIG_JD_USE_ROM is not set
#define CONFIG_JD_SZBUF 512
#define CONFIG_JD_FORMAT 0
// CONFIG_JD_FORMAT_RGB888=y
// CONFIG_JD_FORMAT_RGB565 is not set
// CONFIG_JD_USE_SCALE=y
// CONFIG_JD_TBLCLIP=y
#define CONFIG_JD_FASTDECODE 1
#define CONFIG_JD_TBLCLIP 1
#define CONFIG_JD_DEFAULT_HUFFMAN 0

// CONFIG_JD_FASTDECODE_BASIC is not set
// CONFIG_JD_FASTDECODE_32BIT=y
// CONFIG_JD_FASTDECODE_TABLE is not set
// CONFIG_JD_DEFAULT_HUFFMAN=y
// end of JPEG Decoder


#define JD_SZBUF        CONFIG_JD_SZBUF
/* Specifies size of stream input buffer */

#define JD_FORMAT       CONFIG_JD_FORMAT
/* Specifies output pixel format.
/  0: RGB888 (24-bit/pix)
/  1: RGB565 (16-bit/pix)
/  2: Grayscale (8-bit/pix)
*/

#if defined(CONFIG_JD_USE_SCALE)
#define JD_USE_SCALE    CONFIG_JD_USE_SCALE
#else
#define JD_USE_SCALE    0
#endif
/* Switches output descaling feature.
/  0: Disable
/  1: Enable
*/

#if defined(CONFIG_JD_TBLCLIP)
#define JD_TBLCLIP      CONFIG_JD_TBLCLIP
#else
#define JD_TBLCLIP      0
#endif
/* Use table conversion for saturation arithmetic. A bit faster, but increases 1 KB of code size.
/  0: Disable
/  1: Enable
*/

#define JD_FASTDECODE   CONFIG_JD_FASTDECODE
/* Optimization level
/  0: Basic optimization. Suitable for 8/16-bit MCUs.
/  1: + 32-bit barrel shifter. Suitable for 32-bit MCUs.
/  2: + Table conversion for huffman decoding (wants 6 << HUFF_BIT bytes of RAM)
*/

#if defined(CONFIG_JD_DEFAULT_HUFFMAN)
#define JD_DEFAULT_HUFFMAN CONFIG_JD_DEFAULT_HUFFMAN
#else
#define JD_DEFAULT_HUFFMAN 0
#endif
