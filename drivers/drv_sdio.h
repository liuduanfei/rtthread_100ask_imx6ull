
#ifndef __DRV_SDIO_H__
#define __DRV_SDIO_H__


/* uSDHCx_CMD_XFR_TYP */
#define CMDINX_SHIFT                  24
#define CMDINX_MASK                   (0x3F << CMDINX_SHIFT)

#define DPSEL                         (0x1 << 21)

#define CICEN_SHIFT                   20
#define CICEN                         (0x1 << CICEN_SHIFT)

#define CCCEN_SHIFT                   19
#define CCCEN                         (0x1 << CCCEN_SHIFT)

#define RSPTYP_SHIFT                  16
#define RSPTYP_MASK                   (0x03 << RSPTYP_SHIFT)
#define RSPTYP_0                      (0x00 << RSPTYP_SHIFT)
#define RSPTYP_136                    (0x01 << RSPTYP_SHIFT)
#define RSPTYP_48                     (0x02 << RSPTYP_SHIFT)
#define RSPTYP_48_CHECK_BUSY          (0x03 << RSPTYP_SHIFT)



/* uSDHCx_SYS_CTRL */
#define INITA_SHIFT                   27
#define INITA                         (0x01 << INITA_SHIFT)

#define RSTA_SHIFT                    24
#define RSTA                          (0x01 << RSTA_SHIFT)

#define DTOCV_SHIFT                   16
#define DTOCV_MASK                    (0xF << DTOCV_SHIFT)

#define SDCLKFS_SHIFT                 8
#define SDCLKFS_MASK                  (0xFF << DVS_SHIFT)
#define DVS_SHIFT                     4
#define DVS_MASK                      (0x07 << DVS_SHIFT)

/* uSDHCx_PROT_CTRL */
#define BURST_LEN_EN_SHIFT            27
#define BURST_LEN_EN_MASK             (0x7 << BURST_LEN_EN_SHIFT)

#define BURST_LEN_INCR                (0x1 << BURST_LEN_EN_SHIFT)
#define BURST_LEN_INCR4               (0x2 << BURST_LEN_EN_SHIFT)
#define BURST_LEN_INCR4_WRAP          (0x4 << BURST_LEN_EN_SHIFT)

#define DMASEL_SHIFT                  8
#define DMASEL_MASK                   (0x3 << DMASEL_SHIFT)

#define DMASEL_NO                     (0x0 << DMASEL_SHIFT)
#define DMASEL_ADMA1                  (0x1 << DMASEL_SHIFT)
#define DMASEL_ADMA2                  (0x2 << DMASEL_SHIFT)


#define EMODE_SHIFT                   4
#define EMODE_MASK                    (0x03 << EMODE_SHIFT)
#define EMODE_BIG_ENDIAN              (0x00 << EMODE_SHIFT)
#define EMODE_HALF_WORD_BIG_ENDIAN    (0x01 << EMODE_SHIFT)
#define EMODE_LITTLE_ENDIAN           (0x02 << EMODE_SHIFT)

#define DTW_SHIFT                     1
#define DTW_MASK                      (0x03 << DTW_SHIFT)
#define DTW_8                         (0x02 << DTW_SHIFT)
#define DTW_4                         (0x01 << DTW_SHIFT)
#define DTW_1                         (0x00 << DTW_SHIFT)

/* uSDHCx_INT_SIGNAL_EN */
#define DMAEIEN                       (0x1 << 28)
#define TNEIEN                        (0x1 << 26)
#define AC12EIEN                      (0x1 << 24)
#define DEBEIEN                       (0x1 << 22)
#define DCEIEN                        (0x1 << 21)
#define DTOEIEN                       (0x1 << 20)
#define CIEIEN                        (0x1 << 19)
#define CEBEIEN                       (0x1 << 18)
#define CCEIEN                        (0x1 << 17)
#define CTOEIEN                       (0x1 << 16)
#define TPIEN                         (0x1 << 14)
#define RTEIEN                        (0x1 << 12)
#define CINTIEN                       (0x1 << 8)
#define CRMIEN                        (0x1 << 7)
#define CINSIEN                       (0x1 << 6)
#define BRRIEN                        (0x1 << 5)
#define BWRIEN                        (0x1 << 4)
#define DINTIEN                       (0x1 << 3)
#define BGEIEN                        (0x1 << 2)
#define TCIEN                         (0x1 << 1)
#define CCIEN                         (0x1 << 0)

/* uSDHCx_INT_STATUS_EN */
#define DMAESEN                       (0x1 << 28)
#define TNESEN                        (0x1 << 26)
#define AC12ESEN                      (0x1 << 24)
#define DEBESEN                       (0x1 << 22)
#define DCESEN                        (0x1 << 21)
#define DTOESEN                       (0x1 << 20)
#define CIESEN                        (0x1 << 19)
#define CEBESEN                       (0x1 << 18)
#define CCESEN                        (0x1 << 17)
#define CTOESEN                       (0x1 << 16)
#define TPSEN                         (0x1 << 14)
#define RTESEN                        (0x1 << 12)
#define CINTSEN                       (0x1 << 8)
#define CRMSEN                        (0x1 << 7)
#define CINSSEN                       (0x1 << 6)
#define BRRSEN                        (0x1 << 5)
#define BWRSEN                        (0x1 << 4)
#define DINTSEN                       (0x1 << 3)
#define BGESEN                        (0x1 << 2)
#define TCSEN                         (0x1 << 1)
#define CCSEN                         (0x1 << 0)

/* uSDHCx_INT_STATUS */
#define DMAE                          (0x1 << 28)
#define TNE                           (0x1 << 26)
#define AC12E                         (0x1 << 24)
#define DEBE                          (0x1 << 22)
#define DCE                           (0x1 << 20)
#define DTOE                          (0x1 << 28)
#define CIE                           (0x1 << 19)
#define CEBE                          (0x1 << 18)
#define CCE                           (0x1 << 17)
#define CTOE                          (0x1 << 16)
#define TP                            (0x1 << 14)
#define RTE                           (0x1 << 12)
#define CINT                          (0x1 << 8)
#define CRM                           (0x1 << 7)
#define CINS                          (0x1 << 6)
#define BRR                           (0x1 << 5)
#define BWR                           (0x1 << 4)
#define DINT                          (0x1 << 3)
#define DINT                          (0x1 << 3)
#define BGE                           (0x1 << 2)
#define TC                            (0x1 << 1)
#define CC                            (0x1 << 0)


#define SDHC_CMD_ERRORS  (CIE | CEBE | CCE | CTOE)
#define SDHC_DATA_ERRORS (DMAE | DEBE | DCE | DTOE | DTOE)


/* uSDHCx_PRES_STATE */
#define SDOFF                         (0x1 << 7)
#define SDSTB                         (0x1 << 3)
#define CDIHB                         (0x1 << 1)
#define CIHB                          (0x1 << 0)

#define EXT_DMA_EN                    (0x1 << 0)

/* 0 - write */
#define DTDSEL                        (0x1 << 4)
#define DMAEN                         (0x1 << 0)

/* uSDHCx_WTMK_LVL */
#define WR_BRST_LEN_SHIFT             24
#define WR_BRST_LEN_MASK              (0x1F << WR_BRST_LEN_SHIFT)

#define WR_WML_SHIFT                  16
#define WR_WML_MASK                   (0xFF << WR_WML_SHIFT)

#define RD_BRST_LEN_SHIFT             8
#define RD_BRST_LEN_MASK              (0x1F << RD_BRST_LEN_SHIFT)

#define RD_WML_SHIFT                  0
#define RD_WML_MASK                   (0xFF << RD_WML_SHIFT)

/* uSDHCx_BLK_ATT */

#define BLKCNT_SHIFT                  16
#define BLKCNT_MASK                   (0xFFFF << BLKCNT_SHIFT)

#define BLKSIZE_SHIFT                 0
#define BLKSIZE_MASK                  (0x1FFF << BLKCNT_SHIFT)


#endif /* __DRV_SDIO_H__ */
