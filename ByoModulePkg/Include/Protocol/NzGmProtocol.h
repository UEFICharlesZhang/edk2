
#ifndef __NZGM_PROTOCOL_H__
#define __NZGM_PROTOCOL_H__

// {6E0B4DF9-E86E-4208-AD14-EF9CE9C5D3F7}
#define NZGM_PROTOCOL_GUID = \
  { 0x6e0b4df9, 0xe86e, 0x4208, { 0xad, 0x14, 0xef, 0x9c, 0xe9, 0xc5, 0xd3, 0xf7 } }


#ifndef SCH_CONTEXT_DEFINED
  
typedef struct {
  UINT32 total[2];
  UINT32 state[8];
  UINT8  buffer[64];
} sch_context;

#define SCH_CONTEXT_DEFINED

#endif

typedef
void
(EFIAPI *TCM_SCH_STARTS) (
  sch_context *ctx
  );

typedef
void
(EFIAPI *TCM_SCH_UPDATE) (
  sch_context *ctx,
  UINT8       *Input,
  UINT32      Length
  );

typedef
void
(EFIAPI *TCM_SCH_FINISH) (
  sch_context *ctx,
  UINT8       digest[32]
  );
  
typedef
int
(EFIAPI *TCM_HMACF) (
  UINT8   *text, 
  UINT32  text_len, 
  UINT8   *key, 
  UINT32  key_len, 
  UINT8   digest[32]
  );  
  
typedef
int
(EFIAPI *TCM_ECC_INIT) (
  VOID
  );  

typedef   
int 
(EFIAPI *TCM_ECC_ENCRYPT) (
    UINT8  *plaintext, 
    UINT32 uPlaintextLen, 
    UINT8  *pubkey, 
    UINT32 uPubkeyLen, 
    UINT8  *ciphertext, 
    UINT32 *puCiphertextLen
    );

  
typedef struct {
  TCM_SCH_STARTS      tcm_sch_starts;
  TCM_SCH_UPDATE      tcm_sch_update;
  TCM_SCH_FINISH      tcm_sch_finish;
  TCM_HMACF           tcm_hmac;
  TCM_ECC_INIT        tcm_ecc_init;
  TCM_ECC_ENCRYPT     tcm_ecc_encrypt;
} EFI_NZGM_PROTOCOL;


extern EFI_GUID  gEfiNzGmProtocolGuid;

#endif

