/*
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef _VREG_H_
#define _VREG_H_

/********************************************************/
/*    CRT register                                      */
/********************************************************/
#define    CRT_BASE_ADDRESS                    0x000400
#define    VGA_GRAPH_BASE_ADDRESS              0x600000
#define    VGA_CURSOR_BASE_ADDRESS             0x300000
#define    VGA_OSD_BASE_ADDRESS                0x300000
#define    RGB_565                             0x0
#define    RGB_888                             0x1
#define    YUV_444                             0x2
#define	   NO_SUPPORTED_CRT_FMT				   0x3
#define    CRT_LOW_THRESHOLD_VALUE             0x12
#define    CRT_HIGH_THRESHOLD_VALUE            0x1E

// AST3000's Registers 
#ifdef Watcom
#define     VIDEOBASE_OFFSET                            0x10000
#else
#define     VIDEOBASE_OFFSET                            0x0
#endif

#define     KEY_CONTROL_REG                             0x00 + VIDEOBASE_OFFSET
    #define     VIDEO_LOCKED                            0
    #define     VIDEO_UNLOCK                            1

// Registers for video1 and video2
#define     VIDEO1_ENGINE_SEQUENCE_CONTROL_REG          0x04 + VIDEOBASE_OFFSET
#define     VIDEO2_ENGINE_SEQUENCE_CONTROL_REG          0x104 + VIDEOBASE_OFFSET
    #define     MODE_DETECTION_TRIGGER                  0x00000001  //Bit[0] trigger HW auto mode detection
    #define     VIDEO_CAPTURE_TRIGGER                   0x00000002  //Bit[1] trigger HW to capture video
	#define		FORCE_HW_IDLE_MASK						(1<<2)		//Bit[2] Force engine into ready(idle) state
		#define		ENGINE_RESET						(0<<2)
		#define		ENGINE_IDLE							(1<<2)
    #define     VIDEO_CAPTURE_AUTO_MODE                 0x00000008  //Bit[3]
        #define     CAPTURE_FRAME_MODE                  0
        #define     CAPTURE_AUTO_MODE                   1
		#define		CAPTURE_AUTO_MODE_BIT				3
		#define		CAPTURE_AUTO_MODE_MASK				0x00000008
    #define     VIDEO_CODEC_TRIGGER                     0x00000010  //Bit[4] trigger HW to compress or decompress video
        #define     CODEC_TRIGGER_BIT                   4
        #define     CODEC_TRIGGER_MASK                  0x00000010
        #define     CLEAR_CODEC_TRIGGER                 0
        #define     EN_CODEC_TRIGGER                    1
    #define     VIDEO_CODEC_AUTO_MODE                   0x00000020  //Bit[5]
        #define     CODEC_FRAME_MODE                    0
        #define     CODEC_AUTO_MODE                     1
		#define		CODEC_AUTO_MODE_BIT					5
        #define     CODEC_AUTO_MODE_MASK                0x00000020
	#define		INSERT_FULL_FRAME_MASK					(1<<6)		//Bit[6] Insert full frame compression
		#define		INSERT_FULL_FRAME_EN				(1<<6)
		#define		INSERT_FULL_FRAME_OFF				(0<<6)
    #define     WATCH_DOG_ENABLE                        0x00000080  //Bit[7] Video Enable watch dog for mode change detection
        #define     WATCH_DOG_ENABLE_BIT                7
        #define     WATCH_DOG_OFF                       0
        #define     WATCH_DOG_EN                        1
        #define     WATCH_DOG_EN_MASK                   0x00000080
    #define     VIDEO_CRT_SELECTION                     0x00000100  //Bit[8]
        #define     CRT1                                0
        #define     CRT2                                1
    #define     ANTI_TEARING_ENABLE                     0x00000200  //Bit[9] Anti-tearing mode enable for decoding
        #define     ANTI_TEARING_EN                     1
    #define     STREAM_DATA_MODE                        0x00000400  //Bit[11:10] Buffer and Stream Data type
        #define     STREAM_DATA_MODE_BIT                10
        #define     STREAM_DATA_MODE_MASK               0x00000C00
        #define     YUV444_MODE                         0
        #define     YUV420_MODE                         1
        #define     YUV420_MODE_WITH_AST2000            2 //AST2000 compatible
    #define     VIDEO_CAPTURE_READY                     0x00010000  //Bit[16] Video capture ready status read back(read only)
		#define		CAPTURE_READY_MASK                  0x00010000  
        #define     HW_BUSY                             0
        #define     HW_IDLE                             1
    #define     VIDEO_CODEC_READY                       0x00040000  //Bit[18] Video codec ready status read back(read only)
		#define		CODEC_READY_MASK					0x00040000
        //#define     HW_BUSY                             0
        //#define     HW_IDLE                             1
    
#define    VIDEO1_CONTROL_REG                           0x08 + VIDEOBASE_OFFSET
#define    VIDEO2_CONTROL_REG                           0x108 + VIDEOBASE_OFFSET
    #define     VIDEO_HSYNC_POLARITY                    0x00000001  //Bit[0] Video source hsync polarity
        #define     VIDEO_HSYNC_POLARITY_BIT            0
        #define     NO_INVERSE_POL                      0
        #define     INVERSE_POL							1
    #define     VIDEO_VSYNC_POLARITY                    0x00000002  //Bit[1] Video source vsync polarity
        #define     VIDEO_VSYNC_POLARITY_BIT            1        
        //Setting defination the same as VIDEO_HSYNC_POLARITY
    #define     VIDEO_EXTERNAL_SOURCE                   0x00000004  //Bit[2] Video external source
        #define     EXTERNAL_SOURCE_BIT                 2
        #define     INTERNAL_VGA_SOURCE                 0
        #define     EXTERNAL_VGA_SOURCE                 1
        #define     EXTERNAL_SOURCE_MASK                0x00000004                     
    #define     VIDEO_ANALONG_EXTERNAL_SOURCE           0x00000008  //Bit[3] Video Analog external source
        #define     ANALONG_EXTERNAL_SOURCE_BIT         3
        #define     DVI_SOURCE                          0
        #define     ANALOG_RGB_SOURCE                   1
    #define     VIDEO_INTERNAL_TIMING_GEN               0x00000010  //Bit[4] Video Use internal timing generator
        #define     INTERNAL_TIMING_GEN_BIT             4
        #define     EXTERNAL_DE_SIGNAL                  0 //DVI only
        #define     VR0C_VR10_DE_SINGAL                 1 //use VR0C and VR10 for generate VDE signal
    /******     Video2 Only from DAC    ******/        
    #define     VIDEO1_CAPTURE_FROM                     0x00000020  //Bit[5] Video1 capture data direct from VGA frame buffer(internal VGA only)  
        #define     FROM_DAC_PORT                       0
        #define     FROM_FRAME_BUFFER                   1     
    #define     WRITE_DATA_FORMAT                       0x00000040  //Bit[7:6] Write data format
        #define     WRITE_DATA_FORMAT_BIT               6
        #define     WRTIE_DATA_FORMAT_MASK              0x000000C0
        #define     CCIR601_2_YUV_FORMAT                0
        #define     FULL_YUV_FORMAT                     1
        #define     RGB_FORMAT                          2
		#define     VGA_CURSOR_DISABLE                      0x00000100  //Bit[8] External video port slection
        #define     VGA_CURSOR_NORMAL                   0
        #define     VGA_CURSOR_OFF                      1    
    #define     VIDEO_CAPTURE_LINEAR_MODE               0x00000200  //Bit[9] VIDEO_CAPTURE_LINEAR_MODE
        #define     LINEAR_MODE                         0
        #define     TILE_MODE		                    1		
    #define     VIDEO_CLOCK_DELAY                       0x00000400  //Bit[11:10] Video clock delay control
        #define     VIDEO_CLOCK_DELAY_BIT               10
        #define     VIDEO_CLOCK_DELAY_MASK              0x00000C00
        #define     DELAY_0_NS                          0
        #define     DELAY_1_NS                          1
        #define     INV_AND_DELAY_0_NS                  2
        #define     INV_AND_DELAY_1_NS                  3
    #define     VIDEO_CCIR656_SOURCE_MODE               0x00001000  //Bit[12] Video CCIR656 source mode
        #define     RGB_SOURCE_MODE                     0
        #define     CCIR656_SOURCE_MODE                 1
    #define     SOURCE_PORT_CLOCK_MODE                  0x00002000  //Bit[13] Video Source port clock mode
        #define     SINGLE_EDGE_MODE                    0
        #define     DUAL_EDGE_MODE                      1
    #define     EXTERNAL_PORT_SELECTION                 0x00004000  //Bit[14] External video port slection
        #define     VIDEO_PORT_A                        0
        #define     VIDEO_PORT_B                        1
    #define     VIDEO_CAPTURE_FRAME_RATE                0x00010000  //Bit[23:16] Video capture frame rate control
        #define     VIDEO_CAPTURE_FRAME_RATE_BIT        16
        #define     VIDEO_CAPTURE_FRAME_RATE_MASK       0x00FF0000  //Maximum frame rate = XX * SourceFPS / 60

#define    VIDEO1_TIMEING_GEN_HOR_REG                   0x0C + VIDEOBASE_OFFSET
#define    VIDEO2_TIMEING_GEN_HOR_REG                   0x10C + VIDEOBASE_OFFSET
    #define     VIDEO_HDE_END                           0x00000000  //Bit[12:0] Video HDE End timing generator
    #define     VIDEO_HDE_END_BIT                       0  //Number of pixels from rising edge of Hsync for first active pixel
    #define     VIDEO_HDE_END_MASK                      0x00001FFF
    #define     VIDEO_HDE_START                         0x00010000  //Bit[28:16] Video HDE Start timing generator
    #define     VIDEO_HDE_START_BIT                     16 //Number of pixels from rising edge of Hsync for last active pixel
    #define     VIDEO_HDE_START_MASK                    0x1FFF0000
        #define     FRAME_RATE_OFF                      0    
    
#define    VIDEO1_TIMEING_GEN_V_REG                     0x10 + VIDEOBASE_OFFSET
#define    VIDEO2_TIMEING_GEN_V_REG                     0x110 + VIDEOBASE_OFFSET
    #define     VIDEO_VDE_END                           0x00000001  //Bit[11:0] Video VDE End timing generator
    #define     VIDEO_VDE_END_BIT                       0  //Number of pixels from rising edge of Vsync for first active pixel
    #define     VIDEO_VDE_END_MASK                      0x00000FFF
    #define     VIDEO_VDE_START                         0x00010000  //Bit[27:16] Video VDE Start timing generator
    #define     VIDEO_VDE_START_BIT                     16 //Number of pixels from rising edge of Vsync for last active pixel
    #define     VIDEO_VDE_START_MASK                    0x0FFF0000

#define    VIDEO1_SCALE_FACTOR_REG                      0x14 + VIDEOBASE_OFFSET
#define    VIDEO2_SCALE_FACTOR_REG                      0x114 + VIDEOBASE_OFFSET
    #define     HOR_SCALING_FACTOR                      0x00000001  //Bit[15:0] Video Horizontal scaling factor
    #define     HOR_SCALING_FACTOR_BIT                  0   //The formula=4096/(Horizontal scaling facotr)
    #define     HOR_SCALING_FACTOR_MASK                 0x0000FFFF
    #define     VER_SCALING_FACTOR                      0x00000000  //Bit[31:16] Video Vertical scaling factor
    #define     VER_SCALING_FACTOR_BIT                  16   //The formula=4096/(Vertical scaling facotr)
    #define     VER_SCALING_FACTOR_MASK                 0xFFFF0000

#define    VIDEO1_SCALE_FACTOR_PARAMETER0_REG           0x18 + VIDEOBASE_OFFSET  //Scaling Parameters F00, F01, F02, F03
#define    VIDEO2_SCALE_FACTOR_PARAMETER0_REG           0x118 + VIDEOBASE_OFFSET   

#define    VIDEO1_SCALE_FACTOR_PARAMETER1_REG           0x1C + VIDEOBASE_OFFSET  //Scaling Parameters F10, F11, F12, F13
#define    VIDEO2_SCALE_FACTOR_PARAMETER1_REG           0x11C + VIDEOBASE_OFFSET   

#define    VIDEO1_SCALE_FACTOR_PARAMETER2_REG           0x20 + VIDEOBASE_OFFSET  //Scaling Parameters F20, F21, F22, F23
#define    VIDEO2_SCALE_FACTOR_PARAMETER2_REG           0x120 + VIDEOBASE_OFFSET   

#define    VIDEO1_SCALE_FACTOR_PARAMETER3_REG           0x24 + VIDEOBASE_OFFSET  //Scaling Parameters F30, F31, F32, F33
#define    VIDEO2_SCALE_FACTOR_PARAMETER3_REG           0x124 + VIDEOBASE_OFFSET

#define    VIDEO1_BCD_CONTROL_REG                       0x2C + VIDEOBASE_OFFSET
#define    VIDEO2_BCD_CONTROL_REG                       0x12C + VIDEOBASE_OFFSET
    #define     BCD_ENABLE                              0x00000001  //Bit[0] Enable block change detection(BCD)
        #define     BCD_ENABLE_BIT                      0
        #define     BCD_ENABLE_MASK                     0x00000001
    #define     BCD_TOLERANCE                           0x00010000  //Bit[23:16]
        #define     BCD_TOLERANCE_BIT                   16 //flag as changed block when the video data difference greater
        #define     BCD_TOLERANCE_MASK                  0x00FF0000

#define     VIDEO1_CAPTURE_WINDOWS_REG                  0x30 + VIDEOBASE_OFFSET
#define     VIDEO2_CAPTURE_WINDOWS_REG                  0x130 + VIDEOBASE_OFFSET
    #define     CAPTURE_VER_LINE						0x00000001  //Bit[10:0] Video compression vertical line total
    #define     CAPTURE_VER_LINE_BIT					0
    #define     CAPTURE_VER_LINE_MASK					0x000007FF
    #define     CAPTURE_HOR_PIXEL						0x00010000  //Bit[26:16] Video compression vertical line total
    #define     CAPTURE_HOR_PIXEL_BIT					16
    #define     CAPTURE_HOR_PIXEL_MASK					0x07FF0000

#define     VIDEO1_COMPRESS_WINDOWS_REG                 0x34 + VIDEOBASE_OFFSET
#define     VIDEO2_COMPRESS_WINDOWS_REG                 0x134 + VIDEOBASE_OFFSET
    #define     COMPRESS_VER_LINE                       0x00000001  //Bit[10:0] Video compression vertical line total
    #define     COMPRESS_VER_LINE_BIT                   0
    #define     COMPRESS_VER_LINE_MASK                  0x000007FF
    #define     COMPRESS_HOR_PIXEL                      0x00010000  //Bit[26:16] Video compression vertical line total
    #define     COMPRESS_HOR_PIXEL_BIT                  16
    #define     COMPRESS_HOR_PIXEL_MASK                 0x07FF0000

#define     VIDEO1_COMPRESS_BUF_PROCESS_OFFSET_REG      0x38
#define     VIDEO2_COMPRESS_BUF_PROCESS_OFFSET_REG      0x138
    #define     COMPRESS_BUF_PROCESS_OFFSET_ALIGN       127 //128 byte alignment
    #define     COMPRESS_BUF_PROCESS_OFFSET_MASK        0xFFFFFF

#define     VIDEO1_DECOMPRESS_BUF_PROCESS_OFFSET_REG    0x38
#define     VIDEO2_DECOMPRESS_BUF_PROCESS_OFFSET_REG    0x138
    #define     DECOMPRESS_BUF_PROCESS_OFFSET_ALIGN     127 //128 byte alignment
    #define     DECOMPRESS_BUF_PROCESS_OFFSET_MASK      0xFFFFFF


//For Compression
#define     VIDEO1_COMPRESS_BUF_READ_OFFSET_REG         0x3C + VIDEOBASE_OFFSET     //For stream mode
#define     VIDEO2_COMPRESS_BUF_READ_OFFSET_REG         0x13C + VIDEOBASE_OFFSET
    #define     COMPRESS_BUF_READ_OFFSET_ALIGN          127 //128 byte alignment
    #define     COMPRESS_BUF_READ_OFFSET_MASK           0x00FFFFFF
//For Decompression
#define     VIDEO1_DECOMPRESS_BUF_WRITE_OFFSET_REG      0x3C + VIDEOBASE_OFFSET     //For stream mode 
#define     VIDEO2_DECOMPRESS_BUF_WRITE_OFFSET_REG      0x13C + VIDEOBASE_OFFSET
    #define     DECOMPRESS_BUF_WRITE_OFFSET_ALIGN       127 //128 byte alignment
    #define     DECOMPRESS_BUF_WRITE_OFFSET_MASK        0x00FFFFFF

#define     VIDEO1_CRC_BUF_ADDR_REG                     0x40 + VIDEOBASE_OFFSET
#define     VIDEO2_CRC_BUF_ADDR_REG                     0x140 + VIDEOBASE_OFFSET
    #define     CRC_BUF_ADDR_ALIGN                      7   //8 byte alignment
    #define     CRC_BUF_ADDR_MASK                       0x0FFFFFFF

#define     VIDEO1_BUF_1_ADDR_REG                       0x44 + VIDEOBASE_OFFSET     //For Source Buffer in frame mode
#define     VIDEO2_BUF_1_ADDR_REG                       0x144 + VIDEOBASE_OFFSET
    #define     BUF_1_ADDR_ALIGN                        255 //256 byte alignment
    #define     BUF_1_ADDR_MASK                         0x1FFFFFFF

#define     VIDEO1_BUF_LINE_OFFSET_REG                  0x48 + VIDEOBASE_OFFSET     //Must set both in Frame/Stream mode
#define     VIDEO2_BUF_LINE_OFFSET_REG                  0x148 + VIDEOBASE_OFFSET
    #define     BUF_LINE_OFFSET_ALIGN                   7 //8 byte alignment
    #define     BUF_LINE_OFFSET_MASK                    0x00003FFF

#define     VIDEO1_BUF_2_ADDR_REG                       0x4C + VIDEOBASE_OFFSET     //For BCD Buffer in frame mode
#define     VIDEO2_BUF_2_ADDR_REG                       0x14C + VIDEOBASE_OFFSET     
    #define     BUF_2_ADDR_ALIGN                        255 //256 byte alignment
    #define     BUF_2_ADDR_MASK                         0x1FFFFFFF

#define     VIDEO1_FLAG_BUF_ADDR_REG                    0x50 + VIDEOBASE_OFFSET     //For block change flag buffer
#define     VIDEO2_FLAG_BUF_ADDR_REG                    0x150 + VIDEOBASE_OFFSET     
    #define     FLAG_BUF_ADDR_ALIGN                     7 //8 byte alignment
    #define     FLAG_BUF_ADDR_MASK                      0x1FFFFFFF

#define     VIDEO1_COMPRESS_BUF_ADDR_REG                0x54 + VIDEOBASE_OFFSET     //For stream mode
#define     VIDEO2_COMPRESS_BUF_ADDR_REG                0x154 + VIDEOBASE_OFFSET     
    #define     COMPRESS_BUF_ADDR_ALIGN                 127 //128 byte alignment
    #define     COMPRESS_BUF_ADDR_MASK                  0x1FFFFFFF

#define     VIDEO1_STREAM_BUF_SIZE                      0x58 + VIDEOBASE_OFFSET     //For stream mode 
#define     VIDEO2_STREAM_BUF_SIZE                      0x158 + VIDEOBASE_OFFSET
    #define     STREAM_PACKET_SIZE                      0x00000001
        #define     STREAM_PACKET_SIZE_BIT              0
        #define     STREAM_PACKET_SIZE_MASK             0x00000007
        #define     PACKET_SIZE_1KB                     0
        #define     PACKET_SIZE_2KB                     1
        #define     PACKET_SIZE_4KB                     2
        #define     PACKET_SIZE_8KB                     3
        #define     PACKET_SIZE_16KB                    4
        #define     PACKET_SIZE_32KB                    5
        #define     PACKET_SIZE_64KB                    6
        #define     PACKET_SIZE_128KB                   7
    #define     RING_BUF_PACKET_NUM                     0x00000008
        #define     RING_BUF_PACKET_NUM_BIT             3
        #define     RING_BUF_PACKET_NUM_MASK            0x00000018
        #define     PACKETS_4                           0
        #define     PACKETS_8                           1
        #define     PACKETS_16                          2
        #define     PACKETS_32                          3
    #define     SKIP_HIGH_MB_THRES                      0x00010000  //Bit[22:16] Skip high quality macro block threshold
        #define     SKIP_HIGH_MB_THRES_BIT              16
        #define     SKIP_HIGH_MB_THRES_MASK             0x007F0000
    #define     SKIP_TEST_MODE                          0x00800000  //Bit[24:23] Skip test mode
        #define     SKIP_TEST_MODE_BIT                  23
        #define     SKIP_TEST_MODE_MASK                 0x01800000
        #define     YUV_TEST                            2   //recommend

#define     VIDEO1_BUF_CODEC_OFFSET_READ                0x5C + VIDEOBASE_OFFSET     //For stream mode,         
#define     VIDEO2_BUF_CODEC_OFFSET_READ                0x15C + VIDEOBASE_OFFSET    //Video stream buffer offset read back(HW)
    #define     BUF_CODEC_OFFSET_ALIGN                  255 //256 byte alignment
    #define     BUF_CODEC_OFFSET_MASK                   0x00FFFFFF

#define     VIDEO1_COMPRESS_CONTROL_REG                 0x60 + VIDEOBASE_OFFSET
#define     VIDEO2_COMPRESS_CONTROL_REG                 0x160 + VIDEOBASE_OFFSET
    #define     JPEG_ONLY_ENCODE                        0x00000001  //Bit[0] JPEG only encoding
        #define     JPEG_ONLY_BIT                       0
        #define     JPEG_MIX_MODE                       0   //Enable JPEG/VQ mix mode encoding
        #define     JPEG_ONLY_MODE                      1   //JPEG encoding mode only
    #define     VQ_4_COLOR_ENCODE                       0x00000002  //Bit[1] Enable 4 color VQ encoding
        #define     VQ_4_COLOR_BIT                      1
        #define     VQ_1_2_COLOR_MODE                   0   //1 and 2 color mode VQ encoding
        #define     VQ_4_COLOR_MODE                     1   //1, 2 and 4 color VQ encoding
    #define     QUALITY_CODEC_SETTING                   0x00000004  //Bit[2] High and best video quality codec mode setting
        #define     QUALITY_CODEC_SETTING_BIT           2
        #define     JPEG_CODEC_MODE                     0   //not supported in best video quality mode
        #define     QUANTI_CODEC_MODE                   1
    #define     JPEG_DUAL_QUALITY_EN                    0x00000008  //Bit[3] JPEG dual quality mode
        #define     EN_JPEG_DUAL_QUALITY                1   //enable(only for normal video quality mode)
    #define     BEST_QUALITY_EN                         0x00000010  //Bit[4] Best quality video mode enable
        #define     BEST_QUALITY_EN_BIT                 4
        #define     EN_BEST_QUALITY                     1   //enable(only for quantization codec mode)
    #define     RC4_CRYPT_EN                            0x00000020  //Bit[5] Enable RC4 encryption/decryption
		#define		RC4_CRYPT_EN_BIT					5
		#define     RC4_CRYPT_EN_MASK                   0x00000020
        #define     RC4_CRYPT_ON                        1   //enable
    #define     NORMAL_QUANTI_CHROMI_TABLE              0x00000040  //Bit[10:6] Normal video quality mode JPEG DCT chromi quantination table
        #define     NORMAL_QUANTI_CHROMI_TABLE_BIT      6
        #define     NORMAL_QUANTI_CHROMI_TABLE_MASK     0x000007C0
    #define     NORMAL_QUANTI_LUMI_TABLE                0x00000800  //Bit[15:11] Normal video quality mode JPEG DCT lumi quantination table
        #define     NORMAL_QUANTI_LUMI_TABLE_BIT        11
        #define     NORMAL_QUANTI_LUMI_TABLE_MASK       0x0000F800
    #define     HIGH_QUALITY_EN                         0x00010000  //Bit[16] High video quality mode enable
        #define     HIGH_QUALITY_EN_BIT                 16
        #define     EN_HIGH_QUALITY                     1   //Enable
    #define     UV_CIR656_FORMAT                        0x00080000  //Bit[19] UV fromat
        #define     UV_CIR656_FORMAT_BIT                19
        #define     USE_UV_CIR656                       1   //recommand
    #define     HUFFMAN_TABLE_SELECT                    0x00100000  //Bit[20] JPEG Huffman table combination
        #define     DUAL_TABLE                          0   //Dual Y, UV table
        #define     SINGLE_TABLE                        1   //Single Y table
        #define     SINGLE_UV_TABLE                     0x00200000  //1x: Single UV table
    #define     HIGH_QUANTI_CHROMI_TABLE                0x00400000  //Bit[26:22] High quality JPEG DCT chromi quantization table
        #define     HIGH_QUANTI_CHROMI_TABLE_BIT        22
        #define     HIGH_QUANTI_CHROMI_TABLE_MASK       0x07C00000                
    #define     HIGH_DEQUANTI_VALUE                     0x00400000  //Bit[26:22] High quality de-quantization value
        #define     HIGH_DEQUANTI_VALUE_BIT             22
        #define     HIGH_DEQUANTI_VALUE_MASK            0x07C00000        
    #define     HIGH_QUANTI_LUMI_TABLE                  0x08000000  //Bit[31:27] High quality JPEG DCT lumi quantization table
        #define     HIGH_QUANTI_LUMI_TABLE_BIT          27
        #define     HIGH_QUANTI_LUMI_TABLE_MASK         0xF8000000
    #define     BEST_DEQUANTI_VALUE                     0x08000000  //Bit[31:27] Best quality de-quantization value
        #define     BEST_DEQUANTI_VALUE_BIT             27
        #define     BEST_DEQUANTI_VALUE_MASK            0xF8000000
        

#define     VIDEO1_QUANTI_TABLE_LOW_REG                 0x64 + VIDEOBASE_OFFSET     //Match with 0x60 Bit[10:6], Bit[15:11]
#define     VIDEO2_QUANTI_TABLE_LOW_REG                 0x164 + VIDEOBASE_OFFSET
    #define     QUANTI_CHROMI_TABLE_LOW                 0x00000001  //Bit[4:0] Normal video low quality block chromi quantization table
        #define     QUANTI_CHROMI_TABLE_LOW_BIT         0
        #define     QUANTI_CHROMI_TABLE_LOW_MASK        0x0000001F
    #define     QUANTI_LUMI_TABLE_LOW                   0x00000020  //Bit[9:5] Normal video low quality block lumi quantization table
        #define     QUANTI_LUMI_TABLE_LOW_BIT           5
        #define     QUANTI_LUMI_TABLE_LOW_MASK          0x000003E0

#define     VIDEO1_QUANTI_VALUE_REG                     0x68 + VIDEOBASE_OFFSET     //Match with 0x60 Bit[26:22],Bit[31:27]
#define     VIDEO2_QUANTI_VALUE_REG                     0x168 + VIDEOBASE_OFFSET
    #define     HIGH_QUANTI_VALUE                       0x00000001  //Bit[14:0] High quality quantization value. Format is 1.14
        #define     HIGH_QUANTI_VALUE_BIT               0
        #define     HIGH_QUANTI_VALUE_MASK              0x00007FFF
    #define     BEST_QUANTI_VALUE                       0x00010000  //Bit[30:16] Best quality quantization value. Format is 1.14
        #define     BEST_QUANTI_VALUE_BIT               16
        #define     BEST_QUANTI_VALUE_MASK              0x7FFF0000
    
#define     VIDEO1_BSD_PARA_REG                         0x6C + VIDEOBASE_OFFSET    //Video BSD Parameters Register    
#define     VIDEO2_BSD_PARA_REG                         0x16C + VIDEOBASE_OFFSET
    #define     BSD_HIGH_THRES                          0x00000001  //Bit[7:0] Block sharpness detection high threshold
    #define     BSD_HIGH_THRES_BIT                      0
    #define     BSD_HIGH_THRES_MASK                     0x000000FF
    #define     BSD_LOW_THRES                           0x00000100  //Bit[15:8] Block shaprpness detection low threshold
    #define     BSD_LOW_THRES_BIT                       8
    #define     BSD_LOW_THRES_MASK                      0x0000FF00
    #define     BSD_HIGH_COUNTS                         0x00010000  //Bit[21:16] Block sharpness detection high counts threshold
    #define     BSD_HIGH_COUNTS_BIT                     16
    #define     BSD_HIGH_COUNTS_MASK                    0x003F0000
    #define     BSD_LOW_COUNTS                          0x00400000  //Bit[27:22] Block sharpness detection low counts threshold
    #define     BSD_LOW_COUNTS_BIT                      22
    #define     BSD_LOW_COUNTS_MASK                     0x0FC00000

#define VIDEO1_COMPRESS_FRAME_SIZE_REG                  0x70 + VIDEOBASE_OFFSET
#define VIDEO2_COMPRESS_FRAME_SIZE_REG                  0x170 + VIDEOBASE_OFFSET
    #define     COMPRESS_FRAME_SIZE_READ                0x00000001  //Bit[19:0] Video compression frame size read back(number of DW)
    #define     COMPRESS_FRAME_SIZE_READ_BIT            0
    #define     COMPRESS_FRAME_SIZE_READ_MASK           0x003FFFFF

#define     VIDEO1_COMPRESS_BLOCK_COUNT_REG             0x74 + VIDEOBASE_OFFSET
#define     VIDEO2_COMPRESS_BLOCK_COUNT_REG             0x174 + VIDEOBASE_OFFSET
    #define     PROCESS_BLOCK_COUNT_READ_BIT            0
    #define     PROCESS_BLOCK_COUNT_READ_MASK			0x00003FFF	//Bit[13:0] Video processed total block counter read back(number of blocks)
    #define     COMPRESS_BLOCK_COUNT_READ_BIT           16
    #define     COMPRESS_BLOCK_COUNT_READ_MASK          0xFFFF0000	//Bit[29:16] Video processed total block counter read back(number of blocks)

#define     VIDEO1_COMPRESS_FRAME_END_READ              0x78 + VIDEOBASE_OFFSET     //Video compression stream frame end pointer    
#define     VIDEO2_COMPRESS_FRAME_END_READ              0x178 + VIDEOBASE_OFFSET
    #define     COMPRESS_FRAME_END_READ_ALIGN           7
    #define     COMPRESS_FRAME_END_READ_MASK            0x00FFFFFF

#define     VIDEO1_COMPRESS_FRAME_COUNT_READ            0x7C + VIDEOBASE_OFFSET     
#define     VIDEO2_COMPRESS_FRAME_COUNT_READ            0x17C + VIDEOBASE_OFFSET 
    #define     COMPRESS_FRAME_COUNT_READ               0x00000001  //Bit[15:0] Video compression frame count read back(number of frame)
    #define     COMPRESS_FRAME_COUNT_READ_BIT           0
    #define     COMPRESS_FRAME_COUNT_READ_MASK          0xFFFFFFFF

#define     VIDEO1_USER_DEFINE_HEADER                   0x80 + VIDEOBASE_OFFSET
#define     VIDEO2_USER_DEFINE_HEADER                   0x180 + VIDEOBASE_OFFSET
    #define     USER_DEFINE_HEADER                      0x00000001  //Bit[15:0] Video user defined header parameter
    #define     USER_DEFINE_HEADER_BIT                  0
    #define     USER_DEFINE_HEADER_MASK                 0x0000FFFF
    
#define     VIDE1_MODE_DETECTION_EDGE_H_REG             0x90 + VIDEOBASE_OFFSET
#define     VIDE2_MODE_DETECTION_EDGE_H_REG             0x190 + VIDEOBASE_OFFSET
    #define     LEFT_EDGE_LOCATION                      0x00000001  //Bit[11:0] Video source left edge location from sync rising edge
    #define     LEFT_EDGE_LOCATION_BIT                  0
    #define     LEFT_EDGE_LOCATION_MASK                 0x00000FFF
	#define		NO_VER_SYNC								(1 << 12)		//Bit[12] No Vertical sync detected
	#define		NO_HOR_SYNC								(1 << 13)		//Bit[13] No horizontal sync detected
	#define		NO_ACTIVE_DISP							(1 << 14)		//Bit[14] No active display detected
	#define		NO_DISP_CLOCK							(1 << 15)		
    #define     RIGHT_EDGE_LOCATION                     0x00010000  //Bit[27:16] Video source right edge location from sync rising edge
    #define     RIGHT_EDGE_LOCATION_BIT                 16
    #define     RIGHT_EDGE_LOCATION_MASK                0x0FFF0000

#define     VIDE1_MODE_DETECTION_EDGE_V_REG             0x94 + VIDEOBASE_OFFSET
#define     VIDE2_MODE_DETECTION_EDGE_V_REG             0x194 + VIDEOBASE_OFFSET
    #define     TOP_EDGE_LOCATION                       0x00000001  //Bit[11:0] Video source top edge location from sync rising edge
    #define     TOP_EDGE_LOCATION_BIT                   0
    #define     TOP_EDGE_LOCATION_MASK                  0x00000FFF
    #define     BOTTOM_EDGE_LOCATION                    0x00010000  //Bit[27:16] Video source bottom edge location from sync rising edge
    #define     BOTTOM_EDGE_LOCATION_BIT                16
    #define     BOTTOM_EDGE_LOCATION_MASK               0x0FFF0000

#define VIDEO1_MODE_DETECTION_STATUS_READ_REG           0x98 + VIDEOBASE_OFFSET
#define VIDEO2_MODE_DETECTION_STATUS_READ_REG           0x198 + VIDEOBASE_OFFSET    
    #define     MODE_DETECTION_HOR_TIME_READ            0x00000001  //Bit[11:0] Mode detection Horizontal time read back (read only)
    #define     MODE_DETECTION_HOR_TIME_READ_BIT        0
    #define     MODE_DETECTION_HOR_TIME_READ_MASK       0x00000FFF
    #define     ANALONG_DIGITAL_READ                    0x00001000  //Bit[12] Auto detection for external analog or digital source read back
        #define     ANALONG_DIGITAL_READ_BIT            12
        #define     DVI_SIGNAL                          0
        #define     ADC_SIGNAL                          1
    #define     MODE_DETECTION_HOR_STABLE_READ          0x00002000  //Bit[13] Mode detection horizontal stable read back
        #define     HOR_STABLE                          1
    #define     MODE_DETECTION_VER_STABLE_READ          0x00004000  //Bit[14] Mode detection vertical stable read back
        #define     VER_STABLE                          1
    #define     OUT_LOCK_READ                           0x00008000  //Bit[15] Mode detection out of lock read back
        #define     SIGNAL_OUT_LOCK                     1
    #define     MODE_DETECTION_VER_LINE_READ            0x00010000  //Bit[27:16] Mode detection Vertical lines read back
    #define     MODE_DETECTION_VER_LINE_READ_BIT        16
    #define     MODE_DETECTION_VER_LINE_READ_MASK       0x0FFF0000
    #define     VSYNC_POLARITY_READ                     0x10000000  //Bit[28] Vsync polarity read back
    #define     HSYNC_POLARITY_READ                     0x20000000  //Bit[29] Hsync polarity read back
    #define     MODE_DETECTION_VSYNC_READY              0x40000000  //Bit[30] Mode detection Vsync ready
    #define     MODE_DETECTION_HSYNC_READY              0x80000000  //Bit[31] Mode detection Hsync ready

/******     VIDEO MEMAGER SETTING               ******/
#define     VIDEOM_ENGINE_SEQUENCE_CONTROL_REG          0x204 + VIDEOBASE_OFFSET
    #define     VIDEOM_CAPTURE_TRIGGER                  0x00000002  //Bit[1] trigger HW to capture video
    #define     VIDEOM_AUTO_MODE						0x00000008  //Bit[3]
        #define     DISABLE_AUTO_MODE                   0
        #define     AUTO_COMPRESS						1
    #define     VIDEOM_CODEC_TRIGGER                    0x00000010  //Bit[4] trigger HW to compress or decompress video
    #define     VIDEOM_SOURCE_SELECTION                 0x00000100  //Bit[8]
        #define     VIDEO1                              0
        #define     VIDEO2                              1
    //#define     STREAM_DATA_MODE                        0x00000400  //Bit[11:10] Buffer and Stream Data type
    //    #define     STREAM_DATA_MODE_BIT                10
    //    #define     STREAM_DATA_MODE_MASK               0x00000C00
    //    #define     YUV444_MODE                         0
    //    #define     YUV420_MODE                         1
    //    #define     YUV420_MODE_WITH_AST2000            2 //AST2000 compatible
    #define     VIDEOM_CAPTURE_READY                    0x00010000  //Bit[16] Video capture ready status read back(read only)
        //#define     HW_BUSY                             0
        //#define     HW_IDLE                             1
    #define     VIDEOM_CODEC_READY                      0x00040000  //Bit[18] Video codec ready status read back(read only)
        //#define     HW_BUSY                             0
        //#define     HW_IDLE                             1

#define    VIDEOM_SCALE_FACTOR_REG                      0x214 + VIDEOBASE_OFFSET
//    #define     HOR_SCALING_FACTOR                      0x00000001  //Bit[15:0] Video Horizontal scaling factor
//    #define     HOR_SCALING_FACTOR_BIT                  0   //The formula=4096/(Horizontal scaling facotr)
//    #define     HOR_SCALING_FACTOR_MASK                 0x0000FFFF
//    #define     VER_SCALING_FACTOR                      0x00000000  //Bit[31:16] Video Vertical scaling factor
//    #define     VER_SCALING_FACTOR_BIT                  16   //The formula=4096/(Vertical scaling facotr)
//    #define     VER_SCALING_FACTOR_MASK                 0xFFFF0000

#define    VIDEOM_SCALE_FACTOR_PARAMETER0_REG           0x218 + VIDEOBASE_OFFSET  //Scaling Parameters F00, F01, F02, F03

#define    VIDEOM_SCALE_FACTOR_PARAMETER1_REG           0x21C + VIDEOBASE_OFFSET  //Scaling Parameters F10, F11, F12, F13

#define    VIDEOM_SCALE_FACTOR_PARAMETER2_REG           0x220 + VIDEOBASE_OFFSET  //Scaling Parameters F20, F21, F22, F23

#define    VIDEOM_SCALE_FACTOR_PARAMETER3_REG           0x224 + VIDEOBASE_OFFSET  //Scaling Parameters F30, F31, F32, F33

#define    VIDEOM_BCD_CONTROL_REG                       0x22C + VIDEOBASE_OFFSET
    //#define     BCD_ENABLE                              0x00000001  //Bit[0] Enable block change detection(BCD)
    //#define     BCD_TOLERANCE                           0x00010000  //Bit[23:16]
    //    #define     BCD_TOLERANCE_BIT                   16 //flag as changed block when the video data difference greater
    //    #define     BCD_TOLERANCE_MASK                  0x00FF0000

#define     VIDEOM_CAPTURE_WINDOWS_REG                  0x230 + VIDEOBASE_OFFSET
    //#define     RC4_TABLE_ADDR_ALIGN                    7   //8 byte alignment
    //#define     RC4_TABLE_ADDR_MASK                     0x0FFFFFFF

#define     VIDEOM_COMPRESS_WINDOWS_REG                 0x234 + VIDEOBASE_OFFSET
    //#define     COMPRESS_VER_LINE                       0x00000001  //Bit[12:0] Video compression vertical line total
    //#define     COMPRESS_VER_LINE_BIT                   0
    //#define     COMPRESS_VER_LINE_MASK                  0x00001FFF
    //#define     COMPRESS_HOR_PIXEL                      0x00010000  //Bit[12:0] Video compression vertical line total
    //#define     COMPRESS_HOR_PIXEL_BIT                  16
    //#define     COMPRESS_HOR_PIXEL_MASK                 0x1FFF0000

#define     VIDEOM_COMPRESS_BUF_PROCESS_OFFSET_REG      0x238
    //#define     COMPRESS_BUF_PROCESS_OFFSET_ALIGN       127 //128 byte alignment
    //#define     COMPRESS_BUF_PROCESS_OFFSET_MASK        0x3FFFFF


//For Compression
#define     VIDEOM_COMPRESS_BUF_READ_OFFSET_REG         0x23C + VIDEOBASE_OFFSET     //For stream mode
    //#define     COMPRESS_BUF_READ_OFFSET_ALIGN          127 //128 byte alignment
    //#define     COMPRESS_BUF_READ_OFFSET_MASK           0x003FFFFF
//For Decompression
#define     VIDEOM_DECOMPRESS_BUF_WRITE_OFFSET_REG      0x23C + VIDEOBASE_OFFSET     //For stream mode 
    //#define     DECOMPRESS_BUF_WRITE_OFFSET_ALIGN       127 //128 byte alignment
    //#define     DECOMPRESS_BUF_WRITE_OFFSET_MASK        0x003FFFFF
    
#define     VIDEOM_BUF_1_ADDR_REG                       0x244 + VIDEOBASE_OFFSET     //For Source Buffer in frame mode
    //#define     BUF_1_ADDR_ALIGN                        255 //256 byte alignment
    //#define     BUF_1_ADDR_MASK                         0x0FFFFFFF

#define     VIDEOM_BUF_LINE_OFFSET_REG                  0x248 + VIDEOBASE_OFFSET     //Must set both in Frame/Stream mode
    //#define     BUF_LINE_OFFSET_ALIGN                   7 //8 byte alignment
    //#define     BUF_LINE_OFFSET_MASK                    0x00003FFF

#define     VIDEOM_BUF_2_ADDR_REG                       0x24C + VIDEOBASE_OFFSET     //For BCD Buffer in frame mode
    //#define     BUF_2_ADDR_ALIGN                        255 //256 byte alignment
    //#define     BUF_2_ADDR_MASK                         0x0FFFFFFF

#define     VIDEOM_FLAG_BUF_ADDR_REG                    0x250 + VIDEOBASE_OFFSET     //For block change flag buffer
    //#define     FLAG_BUF_ADDR_ALIGN                     7 //8 byte alignment
    //#define     FLAG_BUF_ADDR_MASK                      0x0FFFFFFF

#define     VIDEOM_COMPRESS_BUF_ADDR_REG                0x254 + VIDEOBASE_OFFSET     //For stream mode
    //#define     FLAG_BUF_ADDR_ALIGN                     7 //8 byte alignment
    //#define     FLAG_BUF_ADDR_MASK                      0x0FFFFFFF
    
#define     VIDEOM_BUF_CODEC_OFFSET_READ                0x25C + VIDEOBASE_OFFSET     //For stream mode,         
    //#define     BUF_CODEC_OFFSET_ALIGN                  255 //256 byte alignment
    //#define     BUF_CODEC_OFFSET_MASK                   0x003FFFFF
    
#define     VIDEOM_COMPRESS_CONTROL_REG                 0x260 + VIDEOBASE_OFFSET
    //#define     JPEG_ONLY_ENCODE                        0x00000001  //Bit[0] JPEG only encoding
    //    #define     JPEG_MIX_MODE                       0   //Enable JPEG/VQ mix mode encoding
    //    #define     JPEG_ONLY_MODE                      1   //JPEG encoding mode only
    //#define     VQ_4_COLOR_ENCODE                       0x00000002  //Bit[1] Enable 4 color VQ encoding
    //    #define     VQ_1_2_COLOR_MODE                   0   //1 and 2 color mode VQ encoding
    //    #define     VQ_4_COLOR_MODE                     1   //1, 2 and 4 color VQ encoding
    //#define     QUALITY_CODEC_SETTING                   0x00000004  //Bit[2] High and best video quality codec mode setting
    //    #define     JPEG_CODEC_MODE                     0   //not supported in best video quality mode
    //    #define     QUANTI_CODEC_MODE                   1
    //#define     JPEG_DUAL_QUALITY_EN                    0x00000008  //Bit[3] JPEG dual quality mode
    //    #define     EN_JPEG_DUAL_QUALITY                1   //enable(only for normal video quality mode)
    //#define     BEST_QUALITY_EN                         0x00000010  //Bit[4] Best quality video mode enable
    //    #define     EN_BEST_QUALITY                     1   //enable(only for quantization codec mode)
    //#define     RC4_CRYPT_EN                            0x00000020  //Bit[5] Enable RC4 encryption/decryption
    //    #define     EN_RC4_CRYPT                        1   //enable
    //#define     NORMAL_QUANTI_CHROMI_TABLE              0x00000040  //Bit[10:6] Normal video quality mode JPEG DCT chromi quantination table
    //    #define     NORMAL_QUANTI_CHROMI_TABLE_BIT      6
    //    #define     NORMAL_QUANTI_CHROMI_TABLE_MASK     0x000007C0
    //#define     NORMAL_QUANTI_LUMI_TABLE                0x00000800  //Bit[15:11] Normal video quality mode JPEG DCT lumi quantination table
    //    #define     NORMAL_QUANTI_LUMI_TABLE_BIT        11
    //    #define     NORMAL_QUANTI_LUMI_TABLE_MASK       0x0000F800
    //#define     HIGH_QUALITY_EN                         0x00010000  //Bit[16] High video quality mode enable
    //    #define     EN_HIGH_QUALITY                     1   //Enable
    //#define     UV_CIR656_FORMAT                        0x00080000  //Bit[19] UV fromat
    //    #define     USE_UV_CIR656                       1   //recommand
    //#define     HUFFMAN_TABLE_SELECT                    0x00100000  //Bit[20] JPEG Huffman table combination
    //    #define     DUAL_TABLE                          0   //Dual Y, UV table
    //    #define     SINGLE_TABLE                        1   //Single Y table
    //    #define     SINGLE_UV_TABLE                     0x00200000  //1x: Single UV table
    //#define     HIGH_QUANTI_CHROMI_TABLE                0x00400000  //Bit[26:22] High quality JPEG DCT chromi quantization table
    //    #define     HIGH_QUANTI_CHROMI_TABLE_BIT        22
    //    #define     HIGH_QUANTI_CHROMI_TABLE_MASK       0x07C00000                
    //#define     HIGH_DEQUANTI_VALUE                     0x00400000  //Bit[26:22] High quality de-quantization value
    //    #define     HIGH_DEQUANTI_VALUE_BIT             22
    //    #define     HIGH_DEQUANTI_VALUE_MASK            0x07C00000        
    //#define     HIGH_QUANTI_LUMI_TABLE                  0x08000000  //Bit[31:27] High quality JPEG DCT lumi quantization table
    //    #define     HIGH_QUANTI_LUMI_TABLE_BIT          27
    //    #define     HIGH_QUANTI_LUMI_TABLE_MASK         0xF8000000
    //#define     BEST_DEQUANTI_VALUE                     0x08000000  //Bit[31:27] Best quality de-quantization value
    //    #define     BEST_QUANTI_VALUE_BIT               27
    //    #define     BEST_QUANTI_VALUE_MASK              0xF8000000
    
#define     VIDEOM_QUANTI_TABLE_LOW_REG                 0x264 + VIDEOBASE_OFFSET     //Match with 0x60 Bit[10:6], Bit[15:11]
//    #define     QUANTI_CHROMI_TABLE_LOW                 0x00000001  //Bit[4:0] Normal video low quality block chromi quantization table
//        #define     QUANTI_CHROMI_TABLE_LOW_BIT         0
//        #define     QUANTI_CHROMI_TABLE_LOW_MASK        0x0000001F
//    #define     QUANTI_LUMI_TABLE_LOW                   0x00000020  //Bit[9:5] Normal video low quality block lumi quantization table
//        #define     QUANTI_CHROMI_TABLE_LOW_BIT         5
//        #define     QUANTI_CHROMI_TABLE_LOW_MASK        0x000003E0

#define     VIDEOM_QUANTI_VALUE_REG                     0x268 + VIDEOBASE_OFFSET     //Match with 0x60 Bit[26:22],Bit[31:27]
//    #define     HIGH_QUANTI_VALUE                       0x00000001  //Bit[14:0] High quality quantization value. Format is 1.14
//        #define     HIGH_QUANTI_VALUE_BIT               0
//        #define     HIGH_QUANTI_VALUE_MASK              0x00007FFF
//    #define     BEST_QUANTI_VALUE                       0x00010000  //Bit[30:16] Best quality quantization value. Format is 1.14
//        #define     BEST_QUANTI_VALUE_BIT               16
//        #define     BEST_QUANTI_VALUE_MASK              0x7FFF0000
    
#define     VIDEOM_BSD_PARA_REG                         0x26C + VIDEOBASE_OFFSET    //Video BSD Parameters Register    
//    #define     BSD_HIGH_THRES                          0x00000001  //Bit[7:0] Block sharpness detection high threshold
//    #define     BSD_HIGH_THRES_BIT                      0
//    #define     BSD_HIGH_THRES_MASK                     0x000000FF
//    #define     BSD_LOW_THRES                           0x00000100  //Bit[15:8] Block shaprpness detection low threshold
//    #define     BSD_LOW_THRES_BIT                       8
//    #define     BSD_LOW_THRES_MASK                      0x0000FF00
//    #define     BSD_HIGH_COUNTS                         0x00010000  //Bit[21:16] Block sharpness detection high counts threshold
//    #define     BSD_HIGH_COUNTS_BIT                     16
//    #define     BSD_HIGH_COUNTS_MASK                    0x003F0000
//    #define     BSD_LOW_COUNTS                          0x01000000  //Bit[27:24] Block sharpness detection low counts threshold
//    #define     BSD_LOW_COUNTS_BIT                      24
//    #define     BSD_LOW_COUNTS_MASK                     0x3F000000

#define VIDEOM_COMPRESS_FRAME_SIZE_REG					  0x270 + VIDEOBASE_OFFSET
//    #define     COMPRESS_FRAME_SIZE_READ                0x00000001  //Bit[19:0] Video compression frame size read back(number of DW)
//    #define     COMPRESS_FRAME_SIZE_READ_BIT            0
//    #define     COMPRESS_FRAME_SIZE_READ_MASK           0x000FFFFF

#define     VIDEOM_COMPRESS_BLOCK_COUNT_REG				  0x274 + VIDEOBASE_OFFSET
//    #define     COMPRESS_BLOCK_COUNT_READ               0x00000001  //Bit[15:0] Video compress block counter read back(number of blocks)
//    #define     COMPRESS_BLOCK_COUNT_READ_BIT           0
//    #define     COMPRESS_BLOCK_COUNT_READ_MASK          0x0000FFFF

#define     VIDEOM_COMPRESS_FRAME_END_READ              0x278 + VIDEOBASE_OFFSET     //Video compression stream frame end pointer    
    //#define     COMPRESS_FRAME_END_READ_ALIGN           7
    //#define     COMPRESS_FRAME_END_READ_MASK            0x003FFFFF
    
#define     VIDEOM_USER_DEFINE_HEADER_REG			      0x280 + VIDEOBASE_OFFSET
//    #define     USER_DEFINE_HEADER                      0x00000001  //Bit[15:0] Video user defined header parameter
//    #define     USER_DEFINE_HEADER_BIT                  0
//    #define     USER_DEFINE_HEADER_MASK                 0x0000FFFF

/******  VR300-VR3FC: General Control registers  *****/
#define     VIDEO_CONTROL_REG                           0x300 + VIDEOBASE_OFFSET
    #define     CODEC_DECOMPRESS_MODE                   0x00000001  //Bit[0] Codec in de-compression mode
        #define     CODEC_DECOMPRESS_MODE_BIT           0
        #define     CODEC_DECOMPRESS_MODE_MASK          0x00000001 
        #define     COMPRESS_MODE                       0
        #define     DECOMPRESS_MODE                     1
    #define     VIDEO_SAFE_MODE							0x00000002  //Bit[1] VIDEO SAFE MODE
		#define     VIDEO_SAFE_MODE_BIT					1
		#define     VIDEO_SAFE_MODE_OFF		            0
        #define     VIDEO_SAFE_MODE_ON		            1
    #define     DELAY_VSYNC								0x00000004  //Bit[2] Delay Internal VSYNC
        #define     DELAY_VSYNC_BIT                     2
        #define     DELAY_VSYNC_MASK                    (1<<2)
		#define		DELAY_VSYNC_OFF						(0<<2)
		#define		DELAY_VSYNC_EN						(1<<2)
    #define     VER_DOWNSCALING_LINE_BUFFER_EN          0x00000010  //Bit[5:4] Video vertical downscaling line buffer enable
		#define		VER_LINE_BUFFER_MASK				(3<<4)
        #define     LINE_BUFFER_OFF                     (0<<4)
        #define     LINE_BUFFER_VIDEO1                  1
        #define     LINE_BUFFER_VIDEO2                  2
        #define     LINE_BUFFER_VIDEOM                  3
    #define     RC4_KEY_BUFFER_SELECTION				(1UL<<6)  //Bit[7:6] RC4 Key Buffer Selection
		#define		RC4_KEY_BUFFER_SELECTION_BIT		6
		#define		RC4_KEY_BUFFER_SELECTION_MASK		(3UL<<6)
        #define     RC4_KEY_BUFFER_VIDEO1               0
        #define     RC4_KEY_BUFFER_VIDEO2               1
        #define     RC4_KEY_BUFFER_VIDEOM               2
    #define     RC4_INIT_RESET							(1UL<<8)  //Bit[8] RC4 initial reset
		#define     RC4_INIT_RESET_BIT					8
		#define     RC4_INIT_RESET_MASK					(1UL<<8)
		#define     RC4_NORMAL_MODE						0
		#define     RC4_RESET_COUNTER					1
    #define     RC4_TEST_MODE							(1UL<<9)  //Bit[9] RC4 test mode
		#define     RC4_TEST_MODE_BIT					9
		#define     RC4_TEST_OFF						0
		#define     RC4_TEST_ON							1
    #define     RC4_SAVE_MODE							(1UL<<14)  //Bit[14] RC4 save mode
		#define     RC4_SAVE_MODE_BIT					14
		#define     RC4_SAVE_MODE_MASK					(1UL<<14)
		#define     RC4_SAVE_MODE_OFF					0
		#define     RC4_SAVE_MODE_ON					1
    #define     RC4_NO_RESET_FRAME						(1UL<<15)  //Bit[15] RC4 no reset when frame completed
		#define     RC4_NO_RESET_FRAME_BIT				15
		#define     RC4_NO_RESET_FRAME_MASK				(1UL<<15)
		#define     RC4_NO_RESET_FRAME_OFF				0		//Always reset
		#define     RC4_NO_RESET_FRAME_ON				1

#define VIDEO_INT_CONTROL_EN_REG                        0x304 + VIDEOBASE_OFFSET
    #define     VIDEO1_WATCH_DOG_INT_EN                 0x00000001  //Bit[0] Enable Video1 mode detection watch dog out of lock interrupt
    #define     VIDEO1_INPUT_COMPLETE_INT_EN            0x00000002  //Bit[1] Enable Video1 video input complete interrupt  (frame complete only for frame mode)
    #define     VIDEO1_PACKET_READY_INT_EN              0x00000004  //Bit[2] Enable Video1 packet ready interrupt
    #define     VIDEO1_COMPRESS_COMPLETE_INT_EN         0x00000008  //Bit[3] Enable Video1 compression complete interrupt
    #define     VIDEO1_MODE_DETECTION_READY_INT_EN      0x00000010  //Bit[4] Enable video1 mode detection ready interrupt
    #define     VIDEO1_FRAME_COMPLETE_INT_EN            0x00000020  //Bit[5] Enable Video1 frame complete interrupt     (only for stream mode)
    #define     VIDEO1_STREAM_ERR_INT_EN                0x00000040  //Bit[6] Enable Video1 decode stream error interrupt
    #define     VIDEO2_WATCH_DOG_INT_EN                 0x00000100  //Bit[8] Enable Video2 mode detection watch dog out of lock interrupt
    #define     VIDEO2_INPUT_COMPLETE_INT_EN            0x00000200  //Bit[9] Enable Video2 video input complete interrupt  (frame complete only for frame mode)
    #define     VIDEO2_PACKET_READY_INT_EN              0x00000400  //Bit[10] Enable Video2 packet ready interrupt
    #define     VIDEO2_COMPRESS_COMPLETE_INT_EN         0x00000800  //Bit[11] Enable Video2 compression complete interrupt
    #define     VIDEO2_MODE_DETECTION_READY_INT_EN      0x00001000  //Bit[12] Enable video2 mode detection ready interrupt
    #define     VIDEO2_FRAME_COMPLETE_INT_EN            0x00002000  //Bit[13] Enable Video2 frame complete interrupt    (only for stream mode)
    #define     VIDEO2_STREAM_ERR_INT_EN                0x00004000  //Bit[14] Enable Video2 decode stream error interrupt
    #define     VIDEOM_INPUT_COMPLETE_INT_EN            0x00010000  //Bit[16] Enable VideoM video input complete interrupt
    #define     VIDEOM_COMPRESS_COMPLETE_INT_EN         0x00020000  //Bit[17] Enable VideoM compression complete interrupt
	#define     VIDEOM_PACKET_READY_INT_EN				0x00040000  //Bit[18] Enable VideoM packet ready interrupt
	#define     VIDEOM_FRAME_COMPLETE_INT_EN            0x00080000  //Bit[19] Enable VideoM frame complete interrupt    (only for stream mode)
    
#define VIDEO_INT_CONTROL_READ_REG                      0x308 + VIDEOBASE_OFFSET //Clear when write 1
    #define     VIDEO1_WATCH_DOG_READ                   0x00000001  //Bit[0] Video1 mode detection watch dog out of lock interrupt status read back
        #define     VIDEO1_WATCH_DOG_BIT				0
        #define     VIDEO1_WATCH_DOG_MASK				0x00000001
    #define     VIDEO1_INPUT_COMPLETE_READ              0x00000002  //Bit[1] Video1 video input complete interrupt status read back (frame complete only for frame mode)
        #define     VIDEO1_INPUT_COMPLETE_BIT			1
        #define     VIDEO1_INPUT_COMPLETE_MASK			0x00000002
    #define     VIDEO1_PACKET_READY_READ                0x00000004  //Bit[2] Video1 packet ready interrupt status read back
        #define     VIDEO1_PACKET_READY_BIT		        2
        #define     VIDEO1_PACKET_READY_MASK		    0x00000004
    #define     VIDEO1_COMPRESS_COMPLETE_READ           0x00000008  //Bit[3] Video1 compression complete interrupt status read back
        #define     VIDEO1_COMPRESS_COMPLETE_BIT		3
        #define     VIDEO1_COMPRESS_COMPLETE_MASK		0x00000008
    #define     VIDEO1_MODE_DETECTION_READY_READ        0x00000010  //Bit[4] Video1 mode detection ready interrupt status read back
        #define     VIDEO1_MODE_DETECTION_READY_BIT     4
    #define     VIDEO1_FRAME_COMPLETE_READ              0x00000020  //Bit[5] Video1 frame complete interrupt status read back
        #define     VIDEO1_FRAME_COMPLETE_BIT			5
        #define     VIDEO1_FRAME_COMPLETE_MASK			0x00000020
    #define     VIDEO1_STREAM_ERR_READ                  0x00000040  //Bit[6] Video1 decode stream error interrupt status read back
        #define     VIDEO1_STREAM_ERR_BIT				6
        #define     VIDEO1_STREAM_ERR_MASK				0x00000040
    #define     VIDEO2_WATCH_DOG_READ                   0x00000100  //Bit[8] Video2 mode detection watch dog out of lock interrupt status read back
        #define     VIDEO2_WATCH_DOG_BIT				8
        #define     VIDEO2_WATCH_DOG_MASK				0x00000100
    #define     VIDEO2_INPUT_COMPLETE_READ              0x00000200  //Bit[9] Video2 video input complete interrupt status read back (frame complete only for frame mode)
        #define     VIDEO2_INPUT_COMPLETE_BIT			9
        #define     VIDEO2_INPUT_COMPLETE_MASK			0x00000200
    #define     VIDEO2_PACKET_READY_READ                0x00000400  //Bit[10] Video2 packet ready interrupt status read back
        #define     VIDEO2_PACKET_READY_BIT		        10
        #define     VIDEO2_PACKET_READY_MASK		    0x00000400
    #define     VIDEO2_COMPRESS_COMPLETE_READ           0x00000800  //Bit[11] Video2 compression complete interrupt status read back 
        #define     VIDEO2_COMPRESS_COMPLETE_BIT		11
        #define     VIDEO2_COMPRESS_COMPLETE_MASK		0x00000800      
		#define     VIDEO2_MODE_DETECTION_READY_READ    0x00001000  //Bit[12] Video2 mode detection ready interrupt status read back
        #define     VIDEO2_MODE_DETECTION_READY_BIT     12              
    #define     VIDEO2_FRAME_COMPLETE_READ              0x00002000  //Bit[13] Video2 frame complete interrupt status read back
        #define     VIDEO2_FRAME_COMPLETE_BIT		    13
        #define     VIDEO2_FRAME_COMPLETE_MASK			0x00002000
    #define     VIDEO2_STREAM_ERR_READ                  0x00004000  //Bit[14] Video2 decode stream error interrupt status read back
        #define     VIDEO2_STREAM_ERR_BIT				14
        #define     VIDEO2_STREAM_ERR_MASK				0x00004000
    //need check spec
    #define     VIDEOM_INPUT_COMPLETE_READ              0x00010000  //Bit[16] VideoM video input complete interrupt status read back
        #define     VIDEOM_INPUT_COMPLETE_BIT			16
        #define     VIDEOM_INPUT_COMPLETE_MASK			0x00010000
    #define     VIDEOM_COMPRESS_COMPLETE_READ           0x00020000  //Bit[17] VideoM compression complete interrupt status read back
        #define     VIDEOM_COMPRESS_COMPLETE_BIT		17
        #define     VIDEOM_COMPRESS_COMPLETE_MASK		0x00020000
    #define     VIDEOM_PACKET_READY_READ                0x00040000  //Bit[18] Clear Packet ready interrupt when write 1
        #define     VIDEOM_PACKET_READY_BIT             18
        #define     VIDEOM_PACKET_READY_MASK            0x00040000    
    #define     VIDEOM_FRAME_COMPLETE_READ              0x00080000  //Bit[19] Clear Frame complete interrupt when write 1
        #define     VIDEOM_FRAME_COMPLETE_BIT           19
        #define     VIDEOM_FRAME_COMPLETE_MASK          0x00080000
   
#define VIDEO_INT_CONTROL_CLEAR_REG                     0x308 + VIDEOBASE_OFFSET //Clear when write 1
    //Clear when write 1
    #define     VIDEO1_WATCH_DOG_CLEAR                  0x00000001  //Bit[0] Clear mode detection watch dog out of lock interrupt when write 1
    #define     VIDEO1_INPUT_COMPLETE_CLEAR             0x00000002  //Bit[1] Clear video input complete interrupt when write 1 (frame complete only for frame mode)
    #define     VIDEO1_PACKET_READY_CLEAR               0x00000004  //Bit[2] Clear Packet ready interrupt when write 1
        #define     VIDEO1_PACKET_READY_CLEAR_BIT       2
        #define     VIDEO1_PACKET_READY_CLEAR_MASK      0x00000004
    #define     VIDEO1_COMPRESS_COMPLETE_CLEAR          0x00000008  //Bit[3] Clear video compression interrupt when write 1
    #define     VIDEO1_MODE_DETECTION_READY_CLEAR       0x00000010  //Bit[4] Clear Video1 Mode detection ready interrupt when write 1
    #define     VIDEO1_FRAME_COMPLETE_CLEAR             0x00000020  //Bit[5] Clear Frame complete interrupt when write 1
        #define     VIDEO1_FRAME_COMPLETE_CLEAR_BIT     5
        #define     VIDEO1_FRAME_COMPLETE_CLEAR_MASK    0x00000020
    #define     VIDEO1_STREAM_ERR_CLEAR                 0x00000040  //Bit[6] Clear decode stream error interrupt when write 1
    #define     VIDEO2_WATCH_DOG_CLEAR                  0x00000100  //Bit[8] Clear Mode detection interrupt when write 1
    #define     VIDEO2_INPUT_COMPLETE_CLEAR             0x00000200  //Bit[9] Clear video input complete interrupt when write 1
    #define     VIDEO2_PACKET_READY_CLEAR               0x00000400  //Bit[10] Clear packet ready interrupt when write 1
    #define     VIDEO2_COMPRESS_COMPLETE_CLEAR          0x00000800  //Bit[11] Clear video compression complete interrupt when write 1
    #define     VIDEO2_MODE_DETECTION_READY_CLEAR       0x00001000  //Bit[12] Clear Video2 Mode detection ready interrupt when write 1
    #define     VIDEO2_FRAME_COMPLETE_CLEAR             0x00002000  //Bit[13] Clear Frame complete interrupt when write 1 (frame complete only for frame mode)
    #define     VIDEO2_STREAM_ERR_CLEAR                 0x00004000  //Bit[14] Clear Decode stream error interrupt when write 1
    //need check spec
    #define     VIDEOM_INPUT_COMPLETE_CLEAR             0x00010000  //Bit[16] Clear video input complete interrupt when write 1
    #define     VIDEOM_COMPRESS_COMPLETE_CLEAR          0x00020000  //Bit[17] Clear compression complete interrupt when write 1
        #define     VIDEOM_COMPRESS_COMPLETE_CLEAR_BIT  17
        #define     VIDEOM_COMPRESS_COMPLETE_CLEAR_MASK 0x00020000    
	#define     VIDEOM_PACKET_READY_CLEAR				0x00040000  //Bit[18] Clear compression complete interrupt when write 1
        #define     VIDEOM_PACKET_READY_CLEAR_BIT       18
        #define     VIDEOM_PACKET_READY_CLEAR_MASK      0x00040000    
    #define     VIDEOM_FRAME_COMPLETE_CLEAR             0x00100000  //Bit[20] Clear Frame complete interrupt when write 1
        #define     VIDEOM_FRAME_COMPLETE_CLEAR_BIT     20
        #define     VIDEOM_FRAME_COMPLETE_CLEAR_MASK    0x00100000
    #define     VIDEOM_STREAM_ERR_CLEAR                 0x00200000  //Bit[21] Clear decode stream error interrupt when write 1
    
#define VIDEO_MODE_DETECTION_PARAM_REG                  0x30C + VIDEOBASE_OFFSET
    #define     EDGE_PIXEL_THRES_BIT                    8           //Bit[15:8] Mode detection edge pixel threshold
    #define     EDGE_PIXEL_THRES_MASK                   0x0000FF00  
    #define     VER_STABLE_MAX_BIT                      16          //Bit[19:16] Mode detection vertical stable maximum
    #define     VER_STABLE_MAX_BIT_MASK                 0x000F0000  
    #define     HOR_STABLE_MAX_BIT                      20          //Bit[23:20] Mode detection horizontal stable maximum
    #define     HOR_STABLE_MAX_BIT_MASK                 0x00F00000
    #define     VER_STABLE_THRES_BIT                    24          //Bit[27:24] Mode detection vertical stable threshold
    #define     VER_STABLE_THRES_BIT_MASK               0x0F000000  
    #define     HOR_STABLE_THRES_BIT                    28          //Bit[31:28] Mode detection horizontal stable threshold
    #define     HOR_STABLE_THRES_BIT_MASK               0xF0000000
    
#define VIDEO_CRC_PRIMARY_REG                           0x320 + VIDEOBASE_OFFSET
    #define     CRC_CHECK_EN                            0x00000001  //Bit[0] Video port 1/2 Enable video capture write CRC check
        #define     CRC_CHECK_EN_BIT                    0
    #define     CRC_CHECK_HIGH                          0x00000002  //Bit[1] Video port 1/2 CRC check high bit only
        #define     CRC_CHECK_HIGH_BIT                  1
    #define     SKIP_COUNT_MAX                          0x00000004  //Bit[7:2] Video port 1/2 Max capture write skip count
        #define     SKIP_COUNT_MAX_BIT                  2
        #define     SKIP_COUNT_MAX_MASK                 0x000000FC
    #define     CRC_PRIMARY_POLY_LOW                    0x00000100  //Bit[15:8] Primary CRC low 8-bit polynomial
        #define     CRC_RIMARY_POLY_LOW_BIT             8
        #define     CRC_RIMARY_POLY_LOW_MASK            0x0000FF00
    #define     CRC_PRIMARY_POLY_HIGH                   0x00010000  //Bit[31:16] Primary CRC high 8-bit polynomial
        #define     CRC_RIMARY_POLY_HIGH_BIT            16
        #define     CRC_RIMARY_POLY_HIGH_MASK           0xFFFF0000
        

#define VIDEO_CRC_SECOND_REG                            0x324 + VIDEOBASE_OFFSET
    #define     CRC_SECOND_POLY_LOW                     0x00000100  //Bit[15:8] Secondary CRC low 8-bit polynomial
        #define     CRC_SECOND_POLY_LOW_BIT             8
        #define     CRC_SECOND_POLY_LOW_MASK            0x0000FF00
    #define     CRC_SECOND_POLY_HIGH                    0x00010000  //Bit[31:16] Secondary CRC high 8-bit polynomial
        #define     CRC_SECOND_POLY_HIGH_BIT            16
        #define     CRC_SECOND_POLY_HIGH_MASK           0xFFFF0000

#define VIDEO1_RC4_KEYS_REG                              0x400 + VIDEOBASE_OFFSET //Total Video1 RC4 Keys     
#define VIDEO2_RC4_KEYS_REG                              0x500 + VIDEOBASE_OFFSET //Total Video2 RC4 Keys     
#define VIDEOM_RC4_KEYS_REG                              0x600 + VIDEOBASE_OFFSET //Total VideoM RC4 Keys     

#endif /* end of _VREG_H_ */

