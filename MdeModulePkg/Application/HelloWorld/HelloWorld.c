/** @file
  This sample application bases on HelloWorld PCD setting
  to print "UEFI Hello World!" to the UEFI Console.

  Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>

//
// String token ID of help message text.
// Shell supports to find help message in the resource section of an application image if
// .MAN file is not found. This global variable is added to make build tool recognizes
// that the help string is consumed by user and then build tool will add the string into
// the resource section. Thus the application can use '-?' option to show help message in
// Shell.
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_STRING_ID mStringHelpTokenId = STRING_TOKEN (STR_HELLO_WORLD_HELP_INFORMATION);
/* MHU address space */
#define MHU_BASE   (0x2A000000)                                                                                                              
#define MHU_SCP_UBOOT_BASE  (MHU_BASE + 0x20)
#define MHU_UBOOT_SCP_BASE  (MHU_BASE + 0x120)
#define MHU_CONFIG_BASE   (MHU_BASE + 0x500)
/*MHU os chanle address*/
#define MHU_SCP_OS_BASE     (MHU_BASE + 0x0)
#define MHU_OS_SCP_BASE	    (MHU_BASE + 0x100)
#define SCP_TO_AP_OS_MEM_BASE     (0x2A006000 + 0x1000)
#define AP_TO_SCP_OS_MEM_BASE    (0x2A006000 + 0x1400)
	
/* shared memroy base */
#define SCP_TO_AP_SHARED_MEM_BASE  (0x2A006000 + 0x800)
#define AP_TO_SCP_SHARED_MEM_BASE  (0x2A006000 + 0xC00)
typedef struct {
	/* Command ID */
	UINT32 id        : 7;
	/* Set ID. Identifies whether this is a standard or extended command. */
	UINT32 set       : 1;
	/* Sender ID to match a reply. The value is sender specific. */
	UINT32 sender    : 8;
	/* Size of the payload in bytes (0 - 511) */
	UINT32 size      : 9;
	UINT32 reserved  : 7;
	/*
	 * Status indicating the success of a command.
	 * See the enum below.
	 */
	UINT32 status;
} scpi_cmd_mhu_t;

typedef struct {
	UINT16 sensor_id;
} pl_get_sensor_value_req_t;

typedef struct {
	UINT32 value_low;
	UINT32 value_high;
} pl_get_sensor_value_rsp_t;

typedef enum {
	SCPI_HEAD_TYPE_REQ = 0, /* SCPI requeset header */
	SCPI_HEAD_TYPE_RSP      /* SCPI response header */
} scpi_header_type_t;

/* MHU channel type */
typedef enum {
	MHU_SCP_UBOOT_CHN = 0,
	MHU_UBOOT_SCP_CHN,
	MHU_SCP_OS_CHN,
	MHU_OS_SCP_CHN
} mhu_chn_type;
	
/* u-boot MHU channel registers */
#define MHU_SCP_CONFIG	MHU_CONFIG_BASE + 0x4
#define MHU_AP_CONFIG	MHU_CONFIG_BASE + 0xc
#define MHU_STAT	0x0
#define MHU_SET     0x8
#define MHU_CLEAR   0x10

/*os MHU channel registers */
#define SCP_OS_CONFIG MHU_CONFIG_BASE
#define AP_OS_CONFIG  MHU_CONFIG_BASE + 0x8

/*
 * Slot 31 is reserved because the MHU hardware uses this register bit to
 * indicate a non-secure access attempt. The total number of available slots is
 * therefore 31 [30:0].
 */
#define MHU_MAX_SLOT_ID		30

typedef struct
{
	unsigned int id  :7;
	unsigned int set : 1; 
	unsigned int sender: 8;
	unsigned int size :  9;
	unsigned int reserved : 7;
	unsigned int status;
	unsigned char dev_id;
  	unsigned char power_state;
	unsigned short rev;
}scpi_cmd_t;

typedef enum {
	SCPI_SET_NORMAL = 0, /* Normal SCPI commands */
	SCPI_SET_EXTENDED    /* Extended SCPI commands */
} scpi_set_t;

enum scpi_cmd_id {
	CMD_ID_SCP_READY = 1,
	CMD_ID_GET_SCP_CAPABILITY,
	CMD_ID_SET_CSS_POWER_STATE,
	CMD_ID_GET_CSS_POWER_STATE,
	CMD_ID_SET_SYSTEM_POWER_STATE,
	CMD_ID_SET_CPU_TIMER,
	CMD_ID_CANCEL_CPU_TIMER,
	CMD_ID_GET_DVFS_CAPABILITY,
	CMD_ID_GET_DVFS_INFO,
	CMD_ID_SET_DVFS,
	CMD_ID_GET_DVFS,
	CMD_ID_GET_DVFS_STATISTICS,
	CMD_ID_GET_CLOCKS_CAPABILITY,
	CMD_ID_GET_CLOCK_INFO,
	CMD_ID_SET_CLOCK_VALUE,
	CMD_ID_GET_CLOCK_VALUE,
	CMD_ID_GET_POWER_SUPPLY_CAPABILITY,
	CMD_ID_GET_POWER_SUPPLY_INFO,
	CMD_ID_SET_POWER_SUPPLY,
	CMD_ID_GET_POWER_SUPPLY,
	CMD_ID_GET_SENSOR_CAPABILITY,
	CMD_ID_GET_SENSOR_INFO,
	CMD_ID_GET_SENSOR_VALUE,
	CMD_ID_CONFIG_PERIODIC_SENSOR_READINGS,
	CMD_ID_CONFIG_SENSOR_BOUNDS,
	CMD_ID_ASYNC_SENSOR_VALUE,
	CMD_ID_SET_DEVICE_POWER_STATE,
	CMD_ID_GET_DEVICE_POWER_STATE,
	CMD_ID_UNKNOW,
};
	
enum {
	SCP_OK = 0, 	/* Success */
	SCP_E_PARAM,	/* Invalid parameter(s) */
	SCP_E_ALIGN,	/* Invalid alignment */
	SCP_E_SIZE, 	/* Invalid size */
	SCP_E_HANDLER,	/* Invalid handler or callback */
	SCP_E_ACCESS,	/* Invalid access or permission denied */
	SCP_E_RANGE,	/* Value out of range */
	SCP_E_TIMEOUT,	/* Time out has ocurred */
	SCP_E_NOMEM,	/* Invalid memory area or pointer */
	SCP_E_PWRSTATE, /* Invalid power state */
	SCP_E_SUPPORT,	/* Feature not supported or disabled */
	SCPI_E_DEVICE,	/* Device error */
	SCPI_E_BUSY,	/* Device is busy */
	SCPI_E_OS,		/* RTOS error occurred */
	SCPI_E_DATA,	/* unexpected or invalid data received */
	SCPI_E_STATE,	/* invalid or unattainable state reuested */
}; 


volatile int mhu_done = 0;
volatile int mhu_os_done = 0;
volatile int sensor_cfg = 0;
//scto,hds,gmu,
unsigned char dev_id[] = {
	0x40,
	0x20,
	0x10,
};
static void print_header(scpi_cmd_mhu_t *header, int type)
{
	if(0 == type)
		Print (L"SCPI requeset AP -> SCP : \n");
	else
		Print (L"SCPI response SCP -> AP : \n");

	Print (L"header : \n");
	Print (L"    id     : 0x%x\n", header->id);
	Print (L"    set    : 0x%x\n", header->set);
	Print (L"    sender : 0x%x\n", header->sender);
	Print (L"    size   : 0x%x\n", header->size);
	Print (L"    status : 0x%x\n", header->status);
}
static scpi_cmd_mhu_t * init_msg(int cmd_type, enum scpi_cmd_id id, int sender_id, int pl_size)
{
	scpi_cmd_mhu_t *header;
  EFI_STATUS Status;
	
	// header = (scpi_cmd_mhu_t *)malloc(sizeof(scpi_cmd_mhu_t) + 512);
	Status = gBS->AllocatePool(AllocateAddress, sizeof(scpi_cmd_t) + 512, (void **)&header);
	if(EFI_ERROR(Status)) {
	  DEBUG((EFI_D_ERROR, "%a() L%d : %r\n",__FUNCTION__, __LINE__, Status));
	}
	if(NULL == header)
	  return NULL;
	// memset(header, 0, sizeof(scpi_cmd_mhu_t) + 512);
	SetMem32(header, sizeof(scpi_cmd_t) + 512,0);
	id = id & 0x7f;
	header->id = id;
	header->sender = sender_id;
	header->set = cmd_type;
	header->size = pl_size;

	return header;
}

void mhu_chn_init(UINT32 chn_type)
{
	UINT64 base = MHU_CONFIG_BASE;

	if(MHU_SCP_UBOOT_CHN == chn_type) {
		base += 0x4;
	} else if (MHU_UBOOT_SCP_CHN == chn_type) {
		base += 0xc;
	} else if (MHU_SCP_OS_CHN == chn_type) {
		base += 0x0;
	} else if (MHU_OS_SCP_CHN == chn_type) {
		base += 0x8;
	} else {
		Print (L"init : mhu channel %d not supported.\n", chn_type);
		return;
	}

	// writel(0x1, base);
	//writel(0x1, base);
	MmioWrite32(base, 0x1);

	return;
}

int mhu_chn_message_start(UINT32 chn_type, UINT32 slot_id)
{
	UINT64 base = 0;
	UINT32 val = 0;
	int i;
	
	if(MHU_SCP_UBOOT_CHN == chn_type) {
		base = MHU_SCP_UBOOT_BASE;
	} else if (MHU_UBOOT_SCP_CHN == chn_type) {
		base = MHU_UBOOT_SCP_BASE;
	} else if (MHU_SCP_OS_CHN == chn_type) {
		base = MHU_SCP_OS_BASE;
	} else if (MHU_OS_SCP_CHN == chn_type) {
		base = MHU_OS_SCP_BASE;
	} else {
		Print (L"start : mhu channel %d not supported.\n", chn_type);
		//DEBUG((EFI_D_INFO, "start : mhu channel %d not supported.\n", chn_type));
		return -1;
	}

	/* Make sure any previous command has finished */
	for(i = 0; i < 10; i++) {
		// val = readl(base + MHU_STAT) & (1 << slot_id);
		val = MmioRead32(base + MHU_STAT) & (1 << slot_id);
		if(val & (1 << slot_id))
			// mdelay(1);
			gBS->Stall(1000);
		else 
			return 0;
	}

	Print (L"scpi wait for slot %d timeout.\n", slot_id);
	//DEBUG((EFI_D_INFO, "scpi wait for slot %d timeout.\n", slot_id));
	return -1;
}

int mhu_chn_message_send(UINT32 chn_type, UINT32 slot_id)
{
	UINT64 base = 0;

	if(MHU_SCP_UBOOT_CHN == chn_type) {
		base = MHU_SCP_UBOOT_BASE;
	} else if (MHU_UBOOT_SCP_CHN == chn_type) {
		base = MHU_UBOOT_SCP_BASE;
	} else if (MHU_SCP_OS_CHN == chn_type) {
		base = MHU_SCP_OS_BASE;
	} else if (MHU_OS_SCP_CHN == chn_type) {
		base = MHU_OS_SCP_BASE;
	} else {
		Print (L"send : mhu channel %d not supported.\n", chn_type);
		return -1;
	}

	/* Send command to SCP */
	// writel(1 << slot_id, base + MHU_SET);
	MmioWrite32(base + MHU_SET, 1 << slot_id);
	
	return 0;
}
int mhu_chn_message_wait(UINT32 chn_type, UINT32 slot_id)
{
	int i = 0;
	UINT64 base = 0;
	/* Wait for response from SCP */
	UINT32 response;

	if(MHU_SCP_UBOOT_CHN == chn_type) {
		base = MHU_SCP_UBOOT_BASE;
	} else if (MHU_UBOOT_SCP_CHN == chn_type) {
		base = MHU_UBOOT_SCP_BASE;
	} else {
		Print (L"wait : mhu channel %d not supported.\n", chn_type);
		return -1;
	}

	for(i = 0; 1 < 100; i++) {
		response = MmioRead32(base + MHU_STAT) & (1 << slot_id);
		// response = readl(base + MHU_STAT) & (1 << slot_id);
		if(1 != (response & (1 << slot_id)))
			// mdelay(1);
			gBS->Stall(1000);
		else
			return 0;
	}

	Print (L"wait for scp response timeout.\n");
	return -1;
}

int mhu_chn_message_end(UINT32 chn_type, UINT32 slot_id)                                                                                                         
{                                                                                                                                        
    UINT64 base = 0;                                                                                                                                                                           
    if(MHU_SCP_UBOOT_CHN == chn_type) {
		base = MHU_SCP_UBOOT_BASE;
		mhu_done = 0;
	} else if (MHU_UBOOT_SCP_CHN == chn_type) {
		base = MHU_UBOOT_SCP_BASE;
	} else if (MHU_SCP_OS_CHN == chn_type) {
		mhu_os_done = 0;
	} else if (MHU_OS_SCP_CHN == chn_type) {

	} else {
		Print (L"end : mhu channel %d not supported.\n", chn_type);
		return -1;
	}                                                                                                                                              
                                                                                                                                                                                   
    /*                                                                                                                                                                              
     * Clear any response we got by writing one in the relevant slot bit to                                                                                                         
     * the CLEAR register                                                                                                                                                           
     */                                                                                                                                                                             
    // writel(1 << slot_id, base + MHU_CLEAR);        
	  MmioWrite32(base + MHU_CLEAR, 1 << slot_id);
                                                                                                                                                                                   
    return 0;                                                                                                                                                                       
}    

static void* mhu(int method, UINT64 header_addr, UINT64 pl_addr, UINT32 cmd_id)
{
	int  res = 0;
	UINT32 pl_size = 0;
	scpi_cmd_mhu_t* header;
	UINT64 share_mem_base = 0;

	if(1 == method ) {
		
		res = mhu_chn_message_start(MHU_UBOOT_SCP_CHN, 0x0);
		if(SCP_OK != res)
			return NULL;

		CopyMem((void *)AP_TO_SCP_SHARED_MEM_BASE, (void *)header_addr, sizeof(scpi_cmd_mhu_t));
		pl_size = ((scpi_cmd_mhu_t *)header_addr)->size;
		if((0 != pl_size) && (0x0 != pl_addr))
			CopyMem((void *)(AP_TO_SCP_SHARED_MEM_BASE + sizeof(scpi_cmd_mhu_t)), 
				(void*)pl_addr, pl_size);

		
		res = mhu_chn_message_send(MHU_UBOOT_SCP_CHN, 0x0);
		if(0 != res)
			return NULL;
		
		res = mhu_chn_message_wait(MHU_SCP_UBOOT_CHN, 0x0);
		if(0 != res)
			return NULL;
		
		
		header = (scpi_cmd_mhu_t *)SCP_TO_AP_SHARED_MEM_BASE;
		if((cmd_id != header->id)) {
			Print (L"error! expect id = 0x%x, but received id = 0x%x \n",
				cmd_id, header->id);
				return NULL;
		}
		print_header(header, SCPI_HEAD_TYPE_RSP);

		mhu_chn_message_end(MHU_SCP_UBOOT_CHN, 0x0);

		share_mem_base = SCP_TO_AP_SHARED_MEM_BASE;

		//return (void *)(SCP_TO_AP_SHARED_MEM_BASE + sizeof(scpi_cmd_t));

	} else if(2 == method){

		res = mhu_chn_message_start(MHU_OS_SCP_CHN, 0x0);
		if(SCP_OK != res)
			return NULL;

		CopyMem((void *)AP_TO_SCP_OS_MEM_BASE, (void *)header_addr, sizeof(scpi_cmd_mhu_t));
		pl_size = ((scpi_cmd_mhu_t *)header_addr)->size;
		if((0 != pl_size) && (0x0 != pl_addr))
			CopyMem((void *)(AP_TO_SCP_OS_MEM_BASE + sizeof(scpi_cmd_mhu_t)), 
				(void*)pl_addr, pl_size);
		
		res = mhu_chn_message_send(MHU_OS_SCP_CHN, 0x0);
		if(0 != res)
			return NULL;
		
		res = mhu_chn_message_wait(MHU_SCP_OS_CHN, 0x0);
		if(0 != res)
			return NULL;
		
		
		header = (scpi_cmd_mhu_t *)SCP_TO_AP_OS_MEM_BASE;
		if((cmd_id != header->id)) {
			Print (L"error! expect id = 0x%x, but received id = 0x%x \n",
				cmd_id, header->id);
				return NULL;
		}
		print_header(header, SCPI_HEAD_TYPE_RSP);

		mhu_chn_message_end(MHU_SCP_OS_CHN, 0x0);

		share_mem_base = SCP_TO_AP_OS_MEM_BASE;

		//return (void *)(SCP_TO_AP_OS_MEM_BASE + sizeof(scpi_cmd_t));
	}
		return (void *)(share_mem_base + sizeof(scpi_cmd_mhu_t));
	
}

static UINT64 get_empty_payload(UINT64 header)
{
	return header + sizeof(scpi_cmd_mhu_t);
}  

static int func_get_sensor_value(int method, int sensor_id)
{
	//struct arm_smccc_res smc_res;
	scpi_cmd_mhu_t *header = NULL;
	//int sender_id = get_sender_id();
	int sender_id = 0;
	int res = 0;
	pl_get_sensor_value_rsp_t *pl_rsp = NULL;

	//if(1 != (argc - args_indx))
	//	return CMD_RET_USAGE;

	header = init_msg(SCPI_SET_NORMAL, CMD_ID_GET_SENSOR_VALUE, sender_id, sizeof(pl_get_sensor_value_req_t));
	if(NULL == header)
		return -1;

	pl_get_sensor_value_req_t *payload = (pl_get_sensor_value_req_t *)get_empty_payload((UINT64)header);
	//payload->sensor_id = simple_strtoul(argv[args_indx], NULL, 16);
	payload->sensor_id = (UINT16)sensor_id;
	
	print_header(header, SCPI_HEAD_TYPE_REQ);
	Print (L"payload : \n");
	Print (L"    sensor id : 0x%x\n", payload->sensor_id);

	if(0 == method) {
		//arm_smccc_smc(SMC_SCPI_ID, (unsigned long)header, 
		//	(unsigned long)payload, 0, 0, 0, 0, 0, &smc_res);
		//pl_rsp = (pl_get_sensor_value_rsp_t *)
		//	extract_rsp_payload(&smc_res, CMD_ID_GET_SENSOR_VALUE, sender_id);
	} else {
		pl_rsp = (pl_get_sensor_value_rsp_t *)
			mhu(method, (UINT64)header, (UINT64)payload, CMD_ID_GET_SENSOR_VALUE);
	}

	if(NULL == pl_rsp) {
		Print (L"get rsp payload failed.\n");
		res = 1;
		goto failed;
	}
	Print (L"payload : \n");
	Print (L"    value low  : 0x%x\n", pl_rsp->value_low);
	Print (L"    value high : 0x%x\n", pl_rsp->value_high);

failed:
	if(NULL != header)
		gBS->FreePool(header);
		// free(header);

	return res;
}


/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINT32 Index;

  Index = 0;

  //
  // Three PCD type (FeatureFlag, UINT32 and String) are used as the sample.
  //
  if (FeaturePcdGet (PcdHelloWorldPrintEnable)) {
    for (Index = 0; Index < PcdGet32 (PcdHelloWorldPrintTimes); Index ++) {
      //
      // Use UefiLib Print API to print string to UEFI console
      //
      Print ((CHAR16*)PcdGetPtr (PcdHelloWorldPrintString));
    }
  }
		mhu_chn_init(MHU_UBOOT_SCP_CHN);                                                                                                    
		mhu_chn_init(MHU_SCP_UBOOT_CHN);
		//func_get_scp_capability(1);		
		func_get_sensor_value(1, 0);
  return EFI_SUCCESS;
}
