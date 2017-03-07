#include <sys/alt_irq.h>
//#include <modular_sgdma_dispatcher.h>
#include "altera_msgdma.h"
#include <altera_msgdma_csr_regs.h>
#include <altera_msgdma_descriptor_regs.h>
#include<sys/alt_errno.h>
#include <io.h>
#include <stdio.h>
#include "sys/alt_cache.h"

const char *read_dma_name = READ_DMA_0_CSR_NAME;
const char *write_dma_name = WRITE_DMA_0_CSR_NAME;



//Interrupt related function prototypes
static void write_dma_isr(void* context);

int main() {

	//open dma streams
	alt_msgdma_dev *read_device_ptr = alt_msgdma_open(read_dma_name);
	alt_msgdma_dev *write_device_ptr = alt_msgdma_open(write_dma_name);

	//register callback
	alt_msgdma_register_callback(read_device_ptr, write_dma_isr,
			ALTERA_MSGDMA_CSR_GLOBAL_INTERRUPT_MASK, NULL);
	alt_msgdma_register_callback(write_device_ptr, write_dma_isr,
			ALTERA_MSGDMA_CSR_GLOBAL_INTERRUPT_MASK, NULL);

	alt_u32 block_length = 32*sizeof(alt_u32);
	alt_u32 data[32];
	data[0] = 15;
	alt_u32 *data_block = data;
	alt_u32 destination[32];
	destination[0] = 0;

	alt_dcache_flush_all();

	alt_msgdma_standard_descriptor mm_to_st_dma_struct;
	alt_msgdma_standard_descriptor st_to_mm_dma_struct;

	if (0 != alt_msgdma_construct_standard_mm_to_st_descriptor(
					read_device_ptr, &mm_to_st_dma_struct, data_block,
					block_length,
					ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK)) {
		printf("failed construct descriptor mm to st\n");
	}
	if (0 != alt_msgdma_construct_standard_st_to_mm_descriptor(
					write_device_ptr, &st_to_mm_dma_struct, destination,
					block_length,
					ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK)) {
		printf("failed construct descriptor st to mm\n");
	}

	if (0 != alt_msgdma_standard_descriptor_sync_transfer(read_device_ptr,
					&mm_to_st_dma_struct)) {
		printf("failed starting transfer mm to st\n");
	}
	if (0 != alt_msgdma_standard_descriptor_sync_transfer(write_device_ptr,
					&st_to_mm_dma_struct)) {
		printf("failed starting transfer st to mm\n");
	}

	alt_dcache_flush_all();

	int i;
	for( i = 0; i < 100000; i++){
		printf("%d %d  ",(int)destination[0], (int)data[0]);
	}

	return 0;
}
static void write_dma_isr(void* context) {
	printf("reached isr \n");
}
